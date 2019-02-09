#pragma once

#include <util/time.hpp>

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <list>
#include <cassert>

class LazyTimer
{
private:

	std::int64_t timeframe_;
	std::int64_t offset_;
	std::int64_t next_ts_;

public:

	LazyTimer(std::int64_t timeframe = 0, std::int64_t offset = 0)
		: timeframe_(timeframe)
		, offset_(offset)
		, next_ts_(0)
	{
	}

	bool TimerEvent(std::int64_t ts, bool include_first = false)
	{
		bool ret = false;
		if (ts >= next_ts_)
		{
			if (next_ts_ != 0 || include_first)
				ret = true;

			next_ts_ = (1 + ts / timeframe_) * timeframe_ + offset_;
		}
		return ret;
	}
};

enum class SchedulerEventType
{
	Once,
	Daily,
	Rebuild
};

struct SchedulerEvent
{
	int64_t ts;
	int64_t time_int;
	int64_t timeframe;
	SchedulerEventType type;

	const char *once_format = "%Y.%m.%d %H:%M:%S";
	const char *daily_format = "%H:%M:%S";

	SchedulerEvent(SchedulerEventType _type, const std::string& _dt)
		: ts(0)
		, time_int(0)
		, timeframe(0)
		, type(_type)
	{
		std::tm dt{};
		std::istringstream ss(_dt);

		switch (_type)
		{
			case SchedulerEventType::Once:
			{
				//ss >> std::get_time(&dt, once_format);
				break;
			}
			case SchedulerEventType::Daily:
			{
				timeframe = 86400000;
				dt.tm_year = 70;
				dt.tm_mday = 1;
				//ss >> std::get_time(&dt, daily_format);
				break;
			}
			default: return;
		}
		
		time_int = dt2ts(dt);
	}
};

class LazyScheduler
{
protected:

	std::vector<SchedulerEvent*> events_;
	std::list<SchedulerEvent*> event_queue_;

	int64_t next_ts_;
	const int64_t schedule_timeframe_ = 86400000;

protected:

	void Reset()
	{
		next_ts_ = 0;
		events_.clear();
		AddEvent(new SchedulerEvent(SchedulerEventType::Rebuild, ""));
	}

	void ScheduleEvent(SchedulerEvent* evt, int64_t ts)
	{
		evt->ts = ts;
		event_queue_.push_back(evt);
	}

	virtual void BuildSchedule(std::int64_t ts)
	{
		event_queue_.clear();

		int64_t begin_ts = (ts / schedule_timeframe_) * schedule_timeframe_;
		int64_t end_ts = begin_ts + schedule_timeframe_;

		for (auto evt : events_)
		{
			switch (evt->type)
			{
				case SchedulerEventType::Once:
				{
					if (evt->time_int >= begin_ts && evt->time_int < end_ts)
						ScheduleEvent(evt, evt->time_int);
					break;
				}
				case SchedulerEventType::Rebuild:
				{
					ScheduleEvent(evt, end_ts);
					break;
				}
				default:
				{
					for (auto dts = (ts / evt->timeframe) * evt->timeframe; dts < end_ts; dts += evt->timeframe)
					{
						if (dts >= begin_ts)
							ScheduleEvent(evt, dts + evt->time_int);
					}
					break;
				}				
			}
		}

		event_queue_.sort([](SchedulerEvent* a, SchedulerEvent* b) {
			return a->ts < b->ts;
		});

		assert(!event_queue_.empty());
		next_ts_ = event_queue_.front()->ts;
	}

	SchedulerEvent* PopEvent()
	{
		SchedulerEvent* evt = event_queue_.front();
		event_queue_.pop_front();

		assert(!event_queue_.empty());
		next_ts_ = event_queue_.front()->ts;

		return evt;
	}

public:
	
	LazyScheduler()
	{
		Reset();
	}

	virtual ~LazyScheduler() {}

	void AddEvent(SchedulerEvent* evt)
	{
		events_.push_back(evt);
	}
	
	template<class T>
	bool GetEvent(std::int64_t ts, T** evt)
	{
		if (ts < next_ts_)
			return false;

		if (event_queue_.empty()
			|| event_queue_.front()->type == SchedulerEventType::Rebuild)
		{
			BuildSchedule(ts);
			if (ts < next_ts_)
				return false;
		}

		*evt = static_cast<T*>(PopEvent());
		return true;
	}
};

#pragma once

#include <chrono>
#include <string>
#include <ctime>
#include <cinttypes>
#include <thread>

#include <util/string.hpp>

inline std::int64_t time_now()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count() + 10800000;  // +3 hours
}

inline void sleep_mcs(std::int64_t interval)
{
	std::this_thread::sleep_for(std::chrono::microseconds(interval));
}

inline std::tm* ts2dt(std::int64_t ts)
{
	ts /= 1000;
	return std::gmtime(&ts);
}

inline std::int64_t dt2ts(std::tm& dt)
{
	return std::mktime(&dt) * 1000;
}

inline std::string ts_format(std::int64_t ts)
{
	if (ts <= 0)
		return "invalid time";

	time_t ts_s = ts / 1000;
	auto tm = std::gmtime(&ts_s);

	char time[64] = {};
	strftime(time, 64, "%y.%m.%d %H:%M:%S", tm);

	char msc[5] = {};
	sprintf(msc, ".%03ld", ts % 1000);

	return std::string(time) + std::string(msc);
}

inline std::string ts_format_log(std::int64_t ts)
{
	auto t = ts2dt(ts);
	return string_format(
		"%02d:%02d:%02d.%03d",
		t->tm_hour, t->tm_min, t->tm_sec, ts % 1000
	);
}

inline std::string dt_format(const std::tm& dt)
{
	char time[64] = {};
	strftime(time, 64, "%y.%m.%d", &dt);

	return std::string(time);
}

class Magic
{
private:

	std::int32_t prefix_;
	std::int32_t counter_;

public:

	Magic()
		: prefix_((time_now() % 1000) << 21)
		, counter_(time_now() % 1000)
	{
	}

	std::int32_t Generate()
	{
		return prefix_ + counter_++;
	}
};

#pragma once

#include <util/numeric.hpp>

#include <deque>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>

class RollingBase
{
protected:

	std::int64_t close_ts_;
	std::int64_t window_;
	std::int64_t timeframe_;
	std::double_t last_;
	std::deque<std::double_t> bars_;

public:

	RollingBase(std::int64_t window, std::int64_t timeframe)
		: close_ts_(0)
		, window_(window)
		, timeframe_(timeframe)
		, last_(0)
	{
	}

	virtual bool Good() const
	{
		return bars_.size() == window_ - 1;
	}

	virtual void OnPush(std::double_t prev_back, std::double_t new_back) {}
	virtual void OnPop(std::double_t prev_front, std::double_t new_front) {}
	virtual std::double_t calculate() { return 0; }

	void Update(std::int64_t ts, std::double_t value)
	{
		if (ts > close_ts_)
		{
			if (close_ts_ != 0)
			{
				std::double_t prev_value = bars_.empty() ? 0 : bars_.back();
				bars_.push_back(last_);
				OnPush(prev_value, bars_.back());

				if (bars_.size() > window_ - 1)
				{
					std::double_t prev_value = bars_.front();
					bars_.pop_front();
					OnPop(prev_value, bars_.front());
				}
			}

			close_ts_ = (1 + ts / timeframe_) * timeframe_;
		}
		
		last_ = value;
	}
};

class RollingMean : public RollingBase
{
protected:

	std::double_t sum_;

public:

	RollingMean(std::int64_t window = 0, std::int64_t timeframe = 0)
		: RollingBase(window, timeframe)
		, sum_(0)
	{
	}

	void OnPush(std::double_t prev_back, std::double_t new_back) override
	{
		sum_ += new_back;
	}

	void OnPop(std::double_t prev_front, std::double_t new_front) override
	{
		sum_ -= prev_front;
	}

	std::double_t calculate() override
	{
		return (sum_ + last_) / (bars_.size() + 1);
	}
};

class RollingStd : public RollingBase
{
private:

	const int bias_ = 1;
	std::double_t sum_sq_;
	std::double_t sum_;
	std::double_t std_;
	std::double_t mean_;

public:

	RollingStd(std::int64_t window = 0, std::int64_t timeframe = 0)
		: RollingBase(window, timeframe)
		, sum_sq_(0)
		, sum_(0)
		, std_(0)
		, mean_(0)
	{
	}

	void OnPush(std::double_t prev_back, std::double_t new_back) override
	{
		sum_ += new_back;
		sum_sq_ += new_back * new_back;
	}

	void OnPop(std::double_t prev_front, std::double_t new_front) override
	{
		sum_ -= prev_front;
		sum_sq_ -= prev_front * prev_front;
	}

	std::double_t calculate() override
	{
		std::double_t n = bars_.size() + 1;
		std::double_t sum = sum_ + last_;	
		mean_ = sum / n;

		std::double_t sum_sq = sum_sq_ + last_ * last_;
		std::double_t sse = sum_sq + n * mean_ * mean_ - 2 * mean_ * sum;
		std_ = std::sqrt(sse / (n - bias_));

		return std_;
	}

	std::double_t mean()
	{
		return mean_;
	}
};

class RollingMinMax : public RollingBase
{
private:

	std::function<bool(std::double_t, std::double_t)> func_;
	std::double_t value_;

public:

	RollingMinMax(std::int64_t window, std::int64_t timeframe, const std::function<bool(std::double_t, std::double_t)>& func)
		: RollingBase(window, timeframe)
		, func_(func)
		, value_(0)
	{
	}

	void OnPush(std::double_t prev_back, std::double_t new_back) override
	{
		switch (bars_.size())
		{
			case 0: __fallthrough;
			case 1: value_ = new_back; break;
			case 2: value_ = prev_back; break;
			default:
			{
				if (func_(prev_back, value_))
					value_ = prev_back;
			}
		}
	}

	void OnPop(std::double_t prev_front, std::double_t new_front) override
	{
		if (prev_front == value_)
			value_ = *std::min_element(bars_.begin(), bars_.end(), func_);
	}

	std::double_t calculate() override
	{
		return func_(last_, value_) ? last_ : value_;
	}

	std::double_t previous()
	{
		return value_;
	}
};

class RollingMin : public RollingMinMax
{
public:

	RollingMin(std::int64_t window = 0, std::int64_t timeframe = 0)
		: RollingMinMax(window, timeframe, [](std::double_t a, std::double_t b) { return a < b; })
	{
	}
};

class RollingMax : public RollingMinMax
{
public:

	RollingMax(std::int64_t window = 0, std::int64_t timeframe = 0)
		: RollingMinMax(window, timeframe, [](std::double_t a, std::double_t b) { return a > b; })
	{
	}
};

class RSI : public RollingBase
{
private:

	std::double_t pos_;
	std::double_t neg_;
	std::int64_t init_;

public:

	RSI(std::int64_t window = 0, std::int64_t timeframe = 0)
		: RollingBase(window, timeframe)
		, pos_(0)
		, neg_(0)
		, init_(0)
	{
	}

	void OnPush(std::double_t prev_back, std::double_t new_back) override
	{
		if (prev_back != 0) {
			std::double_t change = new_back - prev_back;
			std::double_t p = npmax(0, change);
			std::double_t n = npmax(0, -change);

			if (init_ <= window_)
			{
				pos_ += p;
				neg_ += n;

				if (init_++ == window_)
				{
					pos_ /= (std::double_t)window_;
					neg_ /= (std::double_t)window_;
				}
			}
			else
			{
				pos_ = (pos_ * (window_ - 1) + p) / (std::double_t)window_;
				neg_ = (neg_ * (window_ - 1) + n) / (std::double_t)window_;
			}
		}
		else init_++;
	}

	std::double_t calculate() override
	{
		std::double_t change = last_ - bars_.back();
		std::double_t pos = (pos_ * (window_ - 1) + npmax(0, change)) / (std::double_t)window_;
		std::double_t neg = (neg_ * (window_ - 1) + npmax(0, -change)) / (std::double_t)window_;

		if (neg != 0)
		{
			return 100 - 100 / (std::double_t)(1 + pos / neg);
		}
		else
		{
			return pos != 0 ? 100 : 50;
		}
	}
};

class RollingShift : public RollingBase
{
public:

	using RollingBase::RollingBase;

	std::double_t calculate() override
	{
		return bars_.front();
	}

	std::double_t diff()
	{
		return bars_.empty() ? 0 : bars_.back() - bars_.front();
	}
};

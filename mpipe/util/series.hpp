#pragma once

#include <vector>
#include <cinttypes>

namespace mpipe
{
	class Series
	{
	private:

		double prev_value_;
		std::int64_t close_ts_;
		std::int64_t timeframe_;

	public:

		std::vector<std::int64_t> index;
		std::vector<double> values;

	public:

		Series(std::int64_t timeframe)
			: prev_value_(0)
			, close_ts_(0)
			, timeframe_(timeframe)
		{
		}

		void Update(std::int64_t ts, double value)
		{
			if (ts > close_ts_)
			{
				if (close_ts_ != 0)
				{
					index.push_back(close_ts_);
					values.push_back(prev_value_);

					if (index.size() == index.capacity())
					{
						index.reserve(index.capacity() * 2);
						values.reserve(values.capacity() * 2);
					}
				}

				close_ts_ = (1 + ts / timeframe_) * timeframe_;
			}

			prev_value_ = value;
		}
	};
}

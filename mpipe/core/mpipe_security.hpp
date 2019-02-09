#pragma once

#include <core/mpipe_enum.hpp>
#include <core/mpipe_book.hpp>
#include <core/mpipe_position.hpp>
#include <util/decimal.hpp>
#include <util/string.hpp>
#include <util/time.hpp>
#include <util/numeric.hpp>

#include <algorithm>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <vector>
#include <list>
#include <map>

namespace mpipe
{	
	struct Security
	{
		std::string full_code;
		std::string class_code;

		std::int64_t px_decimals;
		std::int64_t px_step;
		std::int64_t qty_decimals;
		std::int64_t qty_step;

		std::int64_t leg_factor;
		std::double_t leg_weight;

		mpipe::Book book;
		mpipe::Position pos;
		
		std::int64_t last_ts;
		std::int64_t last_px;
		std::int64_t last_qty;		

		std::double_t margin_buy;
		std::double_t margin_sell;
				
		std::string __repr__(const std::string& prefix = "")
		{
			std::stringstream ss;
			ss << prefix << "security: " << full_code << " @ " << class_code << std::endl;
			ss << prefix << "px: " << "{ decimals: " << px_decimals << ", step: " << px_step << " }" << std::endl;
			ss << prefix << "qty: " << "{ decimals: " << qty_decimals << ", step: " << qty_step << " }" << std::endl;
			ss << prefix << "leg_factor: " << leg_factor << std::endl;
			ss << prefix << "leg_weight: " << leg_weight << std::endl;
			ss << std::endl;
			ss << book.__repr__(prefix);
			ss << pos.__repr__(prefix);
			ss << std::endl;
			ss << prefix << "last_ts: " << last_ts << " (" << ts_format(last_ts) << ")" << std::endl;
			ss << prefix << "last_px: " << last_px << std::endl;
			ss << prefix << "last_qty: " << last_qty << std::endl;
			ss << std::endl;
			ss << prefix << "margin_buy: " << margin_buy << std::endl;
			ss << prefix << "margin_sell: " << margin_sell << std::endl;
			ss << std::endl;
			
			return ss.str();
		}

		std::string __str__()
		{
			return string_format("%s: %lld { pending: %lld:%lld, chunks: %lld:%lld }",
				full_code.c_str(),
				pos.position,
				pos.buy_limit,
				pos.sell_limit,
				pos.buy_chunks.size(),
				pos.sell_chunks.size()
			);
		}
	};

	inline std::double_t GetMargin(Side side, Security* security)
	{
		switch (side)
		{
			case mpipe::Buy:
			{
				if (security->margin_buy > 0)
					return security->margin_buy;

				if (security->book.Valid())
					return security->book.ask->first / std::pow(10, security->px_decimals);

				return 0;
			}
			case mpipe::Sell: 
			{
				if (security->margin_sell > 0)
					return security->margin_sell;

				if (security->book.Valid())
					return security->book.bid->first / std::pow(10, security->px_decimals);

				return 0;
			}
			default: return 0;
		}
	}

	inline bool BookGood(const std::vector<Security*>& securities)
	{
		return std::all_of(securities.begin(), securities.end(),
			[](Security* sec) { return sec->book.ts > 0; });
	}

	inline bool LastGood(const std::vector<Security*>& securities)
	{
		return std::all_of(securities.begin(), securities.end(),
			[](Security* sec) { return sec->last_ts > 0; });
	}
}

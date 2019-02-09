#pragma once

#include <core/mpipe_enum.hpp>
#include <util/numeric.hpp>

#include <list>
#include <string>
#include <cassert>
#include <sstream>

namespace mpipe
{
	struct PositionChunk
	{
		std::int64_t ts;
		std::int64_t qty;

		PositionChunk(std::int64_t _ts, std::int64_t _qty)
			: ts(_ts)
			, qty(_qty)
		{
		}

		bool Collapse(PositionChunk& chunk)
		{
			std::int64_t min_qty = npmin(qty, chunk.qty);
			qty -= min_qty;
			chunk.qty -= min_qty;
			return 0 == qty;
		}

		bool Merge(const PositionChunk& chunk)
		{
			if (chunk.ts > ts + 60000)
				return false;

			qty += chunk.qty;
			return true;
		}
	};	

	struct Position
	{
		std::int64_t position;
		std::int64_t buy_limit;
		std::int64_t sell_limit;
		std::list<PositionChunk> buy_chunks;
		std::list<PositionChunk> sell_chunks;

		Position()
			: position(0)
			, buy_limit(0)
			, sell_limit(0)
		{
		}

		void Clear()
		{
			position = 0;
			buy_limit = 0;
			sell_limit = 0;
			buy_chunks.clear();
			sell_chunks.clear();
		}

		std::int64_t HowMuch(Side side, std::int64_t max_position = 0)
		{
			switch (side)
			{
				case Side::All: return 0;
				case Side::Buy: return npmax(0, max_position - position - buy_limit);
				case Side::Sell: return npmax(0, max_position + position - sell_limit);
				default: return 0;
			}
		}

		void ChunksApply(std::int64_t ts, std::int64_t qty, Side side)
		{
			PositionChunk chunk(ts, qty);
			auto& open_chunks = side == Side::Buy ? buy_chunks : sell_chunks;
			auto& close_chunks = side == Side::Sell ? buy_chunks : sell_chunks;

			while (!close_chunks.empty()
				&& chunk.qty > 0)
			{
				if (close_chunks.front().Collapse(chunk))
					close_chunks.pop_front();
			}

			if (chunk.qty > 0
				&& (open_chunks.empty() || !open_chunks.back().Merge(chunk)))
			{
				open_chunks.push_back(chunk);
			}

			assert(close_chunks.empty() || open_chunks.empty());
		}

		void PendingApply(std::int64_t delta, Side side)
		{
			switch (side)
			{
				case Side::Buy:
				{
					buy_limit += delta;
					assert(buy_limit >= 0);
					break;
				}
				case Side::Sell:
				{
					sell_limit += delta;
					assert(sell_limit >= 0);
					break;
				}
				default:
				{
					break;
				}
			}
		}

		std::string __repr__(const std::string& prefix = "")
		{
			std::stringstream ss;
			ss << prefix << "position: " << position << std::endl;
			ss << prefix << "limit: { buy: " << buy_limit << ", sell: " << sell_limit << " }" << std::endl;
			ss << prefix << "chunks: { buy: " << buy_chunks.size() << ", sell: " << sell_chunks.size() << " }" << std::endl;

			return ss.str();
		}
	};
}

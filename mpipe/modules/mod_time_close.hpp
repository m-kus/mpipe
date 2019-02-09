#pragma once

#include <core/mpipe.hpp>
#include <util/numeric.hpp>

namespace mpipe
{
	class ModTimeClose : public Module
	{
	private:
			
		std::int64_t timeout_;
		std::int64_t next_ts_;

	protected:

		std::int64_t PosChunksCollect(std::list<PositionChunk>& chunks, std::int64_t prev_ts)
		{
			std::int64_t qty = 0;

			for (auto& chunk : chunks)
			{
				if (chunk.ts > prev_ts)
				{
					next_ts_ = next_ts_ == 0 ? chunk.ts + timeout_ : npmin(next_ts_, chunk.ts + timeout_);
					break;
				}

				qty += chunk.qty;
			}

			return qty;
		}

	public:

		ModTimeClose(std::int64_t timeout)
			: timeout_(timeout)
			, next_ts_(0)
		{
		}

		void Mutate(State* state) override
		{			
			if (next_ts_ != 0)
			{
				if (state->signal.ts < next_ts_)
					return;

				next_ts_ = 0;
			}

			std::vector<Order> force_close;

			for (auto& sec : state->securities)
			{
				if (sec->leg_factor == 0
					|| sec->pos.position == 0)
				{
					continue;
				}

				Side pos_side = (Side)npsign(sec->pos.position);
				std::int64_t prev_ts = state->signal.ts - timeout_;
				std::int64_t qty = 0;

				switch (pos_side)
				{
					case Side::Buy: qty = PosChunksCollect(sec->pos.buy_chunks, prev_ts); break;
					case Side::Sell: qty = PosChunksCollect(sec->pos.sell_chunks, prev_ts); break;
					default: break;
				}

				if (qty > 0)
				{
					if (force_close.empty())
						force_close.reserve(state->securities.size());

					force_close.emplace_back(qty, (Side)(-pos_side), sec);
				}
			}

			if (!force_close.empty())
			{
				log_format("time close");
				state->orders = force_close;
				state->signal.action = Action::ForceClose;
			}
		}
	};
}

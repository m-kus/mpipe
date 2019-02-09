#pragma once

#include <core/mpipe.hpp>

namespace mpipe
{
	class ModMarketPx : public Module
	{
	private:

		std::int64_t min_qty_;
		std::int64_t slippage_;
		
	public:

		ModMarketPx(std::int64_t min_qty = 1, std::int64_t slippage = 0)
			: min_qty_(min_qty)
			, slippage_(slippage)
		{
		}

		void Mutate(State* state) override
		{
			for (auto& order : state->orders)
			{
				if (order.qty == 0)
					continue;

				if (!order.security->book.Valid())
				{
					state->signal.action = Action::Bypass;
					state->orders.clear();
					return;
				}
				
				std::int64_t liq_qty = 0;
				std::int64_t min_qty = min_qty_ * npabs(order.security->leg_factor);

				switch (order.side)
				{
					case Side::Buy:
					{
						order.px = order.security->book.ask->first + slippage_;
						liq_qty = order.security->book.ask->second;
						break;
					}
					case Side::Sell:
					{
						order.px = order.security->book.bid->first - slippage_;
						liq_qty = order.security->book.bid->second;
						break;
					}
					default: break;
				}
				
				order.qty = npmin(order.qty, npmax(liq_qty, min_qty));
			}
		}
	};
}

#pragma once

#include <core/mpipe.hpp>

namespace mpipe
{
	class ModLastPx : public Module
	{
	private:

		std::int64_t spread_pt_;

	public:

		ModLastPx(std::int64_t spread_pt = 0)
			: spread_pt_(spread_pt)
		{
		}

		void Mutate(State* state) override
		{
			for (auto& order : state->orders)
			{
				if (order.qty == 0)
					continue;

				if (order.security->last_ts == 0)
				{
					state->signal.action = Action::Bypass;
					state->orders.clear();
					return;
				}

				order.px = order.security->last_px + order.side * spread_pt_;
			}
		}
	};
}

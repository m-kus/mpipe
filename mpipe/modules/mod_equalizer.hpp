#pragma once

#include <core/mpipe.hpp>

#include <algorithm>

namespace mpipe
{
	class ModEqualizer : public Module
	{
	public:

		void Mutate(State* state) override
		{
			int64_t target_qty = 0;
			for (auto& order : state->orders)
			{
				int64_t common_qty = order.qty / npabs(order.security->leg_factor);
				if (common_qty < target_qty || target_qty == 0)
					target_qty = common_qty;
			}

			if (target_qty > 0)
			{
				for (auto& order : state->orders)
					order.qty = target_qty * npabs(order.security->leg_factor);
			}
			else
			{
				state->orders.clear();
			}			
		}
	};
}

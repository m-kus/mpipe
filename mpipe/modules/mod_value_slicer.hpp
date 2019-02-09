#pragma once

#include <core/mpipe.hpp>

namespace mpipe
{
	class ModValueSlicer : public Module
	{
	private:

		std::double_t min_slice_;

	protected:

		Side GetOrderSide(const Signal& signal, Security* sec)
		{
			switch (signal.side)
			{
				case Side::Buy: __fallthrough;
				case Side::Sell: return signal.side;
				default: break;
			}

			switch (signal.action)
			{
				case Action::Close: __fallthrough;
				case Action::ForceClose: return (Side)npsign(-sec->pos.position);
				default: break;
			}

			return signal.side;
		}

	public:

		ModValueSlicer(std::double_t min_slice)
			: min_slice_(min_slice)
		{
		}

		void Mutate(State* state) override
		{
			if (!state->signal.Tradable()
				|| !state->orders.empty())
			{
				return;
			}

			for (auto& security : state->securities)
			{
				if (security->leg_weight > 0)
				{
					Side side = GetOrderSide(state->signal, security);
					std::double_t margin = GetMargin(side, security);
					
					if (margin > 0)
					{
						std::int64_t qty = (std::int64_t)((std::double_t)(min_slice_ * security->leg_weight * std::pow(10, security->px_decimals)) / margin);
						if (qty > 0)
							state->orders.emplace_back(qty, side, security);
					}
				}
			}
		}
	};
}

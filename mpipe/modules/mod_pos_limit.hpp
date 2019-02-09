#pragma once

#include <core/mpipe.hpp>

namespace mpipe
{
	class ModPosLimit : public Module
	{
	private:

		std::int64_t max_pos_;

	protected:
	
		virtual std::int64_t HowMuchCanOpen(Side side, Security* sec)
		{
			return sec->pos.HowMuch(side, max_pos_ * npabs(sec->leg_factor));
		}

		virtual std::int64_t HowMuchCanClose(Side side, Security* sec)
		{
			if (side != npsign(-sec->pos.position))
				return 0;

			return sec->pos.HowMuch(side, 0);
		}

	public:

		ModPosLimit(std::int64_t max_pos = 1)
			: max_pos_(max_pos)
		{
		}

		virtual ~ModPosLimit()
		{
		}

		virtual void Mutate(State* state) override
		{
			for (auto& order : state->orders)
			{
				std::int64_t qty = 0;
				switch (state->signal.action)
				{
					case Action::ForceClose: __fallthrough;
					case Action::Close: qty = HowMuchCanClose(order.side, order.security); break;
					case Action::Open: qty = HowMuchCanOpen(order.side, order.security); break;
					default: break;
				}

				order.qty = npmin(order.qty, qty);
			}
		}
	};
}

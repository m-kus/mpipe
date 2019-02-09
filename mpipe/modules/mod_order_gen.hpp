#pragma once

#include <core/mpipe.hpp>

namespace mpipe
{
	class ModOrderGen : public Module
	{
	private:

		int64_t lot_size_;

	protected:

		Side GetOrderSide(const Signal& signal, Security* sec)
		{
			switch (signal.side)
			{
				case Side::Buy: __fallthrough;
				case Side::Sell: return (Side)(signal.side * npsign(sec->leg_factor));
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

		std::int64_t GetOrderSize(const Signal& signal, Security* sec)
		{
			switch (signal.action)
			{
				case Action::Open: __fallthrough;
				case Action::Close: return lot_size_ * npabs(sec->leg_factor);
				case Action::ForceClose: return npabs(sec->pos.position);
				default: break;
			}

			return 0;
		}

	public:

		ModOrderGen(std::int64_t lot_size = 1)
			: lot_size_(lot_size)
		{
		}

		void Mutate(State* state) override
		{
			if (!state->signal.Tradable()
				|| !state->orders.empty())
			{
				return;
			}
			
			for (auto& sec : state->securities)
			{
				if (sec->leg_factor == 0)
					continue;

				state->orders.emplace_back(
					GetOrderSize(state->signal, sec),
					GetOrderSide(state->signal, sec),
					sec
				);
			}
		}
	};
}

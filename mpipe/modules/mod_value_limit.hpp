#pragma once

#include <modules/mod_pos_limit.hpp>

#include <numeric>

namespace mpipe
{
	class ModValueLimit : public ModPosLimit
	{
	private:

		std::double_t max_value_;
		bool max_terminate_;

	protected:
	
		virtual std::int64_t HowMuchCanOpen(Side side, Security* security) override
		{
			if (security->leg_weight <= 0)
				return 0;

			std::double_t margin = GetMargin(side, security);
			if (margin == 0)
				return 0;
					
			std::int64_t max_qty = std::round(max_value_ * security->leg_weight * std::pow(10, security->px_decimals) / margin);
			return security->pos.HowMuch(side, max_qty);
		}

		void TerminateOnMaxValue(State* state)
		{
			if (max_terminate_
				&& !state->orders.empty()
				&& std::all_of(state->orders.begin(), state->orders.end(),
					[](Order& order) { return order.qty == 0; })
				&& std::all_of(state->securities.begin(), state->securities.end(),
					[](Security* sec) { return sec->pos.sell_limit == 0 && sec->pos.buy_limit == 0; }))
			{
				log_format("locked and loaded");
				state->signal.action = Action::Terminate;
			}
		}

	public:

		ModValueLimit(std::double_t max_value, bool max_terminate = false)
			: ModPosLimit(0)
			, max_value_(max_value)
			, max_terminate_(max_terminate)
		{
		}

		virtual ~ModValueLimit()
		{
		}
		
		void Mutate(State* state) override
		{
			ModPosLimit::Mutate(state);
			TerminateOnMaxValue(state);
		}
	};
}

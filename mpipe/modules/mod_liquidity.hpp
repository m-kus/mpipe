#pragma once

#include <core/mpipe.hpp>

namespace mpipe
{
	class ModLiquidity : public Module
	{
	private:

		std::double_t min_value_;
		bool min_terminate_;

	public:

		ModLiquidity(std::double_t min_value, bool min_terminate = false)
			: min_value_(min_value)
			, min_terminate_(min_terminate)
		{
		}

		virtual void Mutate(State* state) override
		{
			std::double_t factor = std::pow(10, 16);
			std::size_t empty_count = 0;

			for (auto& order : state->orders)
			{
				if (order.qty * order.px / factor <= min_value_)
					order.qty = 0;

				if (order.qty == 0)
					empty_count++;
			}

			if (min_terminate_
				&& state->orders.size() == empty_count)
			{
				log_format("all clean");
				state->signal.action = Action::Terminate;
			}
		}
	};
}

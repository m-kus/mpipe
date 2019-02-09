#pragma once

#include <modules/mod_value_limit.hpp>

namespace mpipe
{
	class ModValueUnhedged : public ModValueLimit
	{
	private:

		std::double_t max_unhedge_;

	protected:

		std::double_t GetExpandedValue(const Order& order)
		{
			if (order.security->leg_weight <= 0)
				return 0;

			std::int64_t position = HowMuchCanClose((Side)(-order.side), order.security);
			std::double_t value = position * GetMargin(order.side, order.security) / std::pow(10, order.security->px_decimals);
			return value / order.security->leg_weight;
		}

		void RemoveUnhedgedOrders(std::vector<Order>& orders)
		{
			if (orders.empty())
				return;

			std::vector<std::double_t> expanded_values(orders.size());
			std::transform(orders.begin(), orders.end(), expanded_values.begin(),
				[this](const Order& order) { return GetExpandedValue(order); });

			auto min_expanded_value = *std::min_element(expanded_values.begin(), expanded_values.end());

			for (auto i = 0; i < orders.size(); i++)
			{
				if (expanded_values[i] > min_expanded_value + max_unhedge_)
					orders[i].qty = 0;
			}
		}

	public:

		ModValueUnhedged(std::double_t max_value, std::double_t max_unhedge = 0, bool max_terminate = false)
			: ModValueLimit(max_value, max_terminate)
			, max_unhedge_(max_unhedge)
		{
		}

		void Mutate(State* state) override
		{
			RemoveUnhedgedOrders(state->orders);
			ModValueLimit::Mutate(state);
		}
	};
}

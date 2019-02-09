#include <modules/mod_value_limit.hpp>
#include <gtest/gtest.h>

#include <utils.hpp>

TEST(ModValueLimitTest, SimpleCase)
{
	Security sec1 = MakeSecurity();
	Security sec2 = MakeSecurity();
	State state({
		{ "sec1", &sec1 },
		{ "sec2", &sec2 }
	});
	ModValueLimit m(10, true);

	sec1.margin_buy = 0.1;
	sec1.leg_weight = 0.3;
	sec2.margin_buy = 0.2;
	sec2.leg_weight = 0.7;

	state.signal = MakeOpenLongNow();
	state.orders.emplace_back(1000, Side::Buy, &sec1);
	state.orders.emplace_back(1000, Side::Buy, &sec2);

	m.Mutate(&state);	
	ASSERT_EQ(state.orders.front().qty, 30);
	ASSERT_EQ(state.orders.back().qty, 35);

	for (auto& order : state.orders)
	{
		order.Add(time_now());
		order.Fill(time_now(), order.qty, 1);
	}

	m.Mutate(&state);
	ASSERT_EQ(state.signal.action, Action::Terminate);
}

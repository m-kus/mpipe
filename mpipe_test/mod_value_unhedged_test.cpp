#include <modules/mod_value_unhedged.hpp>
#include <gtest/gtest.h>

#include <utils.hpp>

TEST(ModValueUnhedgedTest, SimpleCase)
{
	Security sec1 = MakeSecurity();
	Security sec2 = MakeSecurity();
	State state({
		{ "sec1", &sec1 },
		{ "sec2", &sec2 }
	});
	ModValueUnhedged m(10, 1, true);
	
	sec1.margin_buy = 0.1;
	sec1.leg_weight = 0.3;
	sec2.margin_buy = 0.2;
	sec2.leg_weight = 0.7;

	state.signal = MakeOpenLongNow();
	state.orders.emplace_back(1000, Side::Buy, &sec1);
	state.orders.emplace_back(1000, Side::Buy, &sec2);

	m.Mutate(&state);
	ASSERT_EQ(state.orders.back().qty, 35);
		
	state.orders.back().Add(time_now());
	state.orders.back().Fill(time_now(), 35, 1);
	
	m.Mutate(&state);

	ASSERT_EQ(state.orders.front().qty, 30);
	ASSERT_EQ(state.orders.back().qty, 0);
}

TEST(ModValueUnhedgedTest, SimpleCase2)
{
	Security sec1 = MakeSecurity();
	Security sec2 = MakeSecurity();
	State state({});
	state.securities.push_back(&sec1);
	state.securities.push_back(&sec2);

	ModValueUnhedged m(10, 1, true);

	sec1.margin_buy = 0.0;
	sec1.leg_weight = 0.2;
	sec2.margin_buy = 0.1;
	sec2.leg_weight = 0.8;

	state.signal = MakeOpenLongNow();
	state.orders.emplace_back(1000, Side::Buy, &sec1);
	state.orders.emplace_back(1000, Side::Buy, &sec2);

	m.Mutate(&state);
	ASSERT_EQ(state.orders.back().qty, 80);

	state.orders.back().Add(time_now());
	state.orders.back().Fill(time_now(), 80, 1);

	m.Mutate(&state);

	ASSERT_EQ(state.orders.front().qty, 0);
	ASSERT_EQ(state.orders.back().qty, 0);
}
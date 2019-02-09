#include <modules/mod_balancer.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>

TEST(MpipeModuleTest, ModBalancerTestClose)
{
	Security sec1 = MakeSecurity(1, 0);
	Security sec2 = MakeSecurity(-1, -10);
	State state({
		{ "sec1", &sec1 }, 
		{ "sec2", &sec2 }
	});
	ModBalancer balancer;

	balancer.Mutate(&state);
	ASSERT_EQ(state.orders.size(), 1);
	ASSERT_EQ(state.orders.back().security, &sec2);
	ASSERT_EQ(state.orders.back().qty, 10);
	ASSERT_EQ(state.orders.back().side, Side::Buy);
}

TEST(MpipeModuleTest, ModBalancerTestOpen)
{
	Security sec1 = MakeSecurity(1, 0);
	Security sec2 = MakeSecurity(-1, -10);
	State state({ 
		{ "sec1", &sec1 }, 
		{ "sec2", &sec2 }
	});
	ModBalancer balancer;

	state.signal = MakeOpenLongNow();

	balancer.Mutate(&state);
	ASSERT_EQ(state.orders.size(), 1);
	ASSERT_EQ(state.orders.back().security, &sec1);
	ASSERT_EQ(state.orders.back().qty, 10);
	ASSERT_EQ(state.orders.back().side, Side::Buy);
}

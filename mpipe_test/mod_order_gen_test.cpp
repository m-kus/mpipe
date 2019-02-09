#include <modules/mod_order_gen.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>

TEST(MpipeModuleTest, ModOrderGenTest)
{
	Security sec = MakeSecurity(1);
	State state({ 
		{ "sec", &sec } 
	});
	ModOrderGen order_gen(1);

	state.signal = MakeOpenLongNow();

	order_gen.Mutate(&state);
	ASSERT_EQ(state.orders.size(), 1);
	ASSERT_EQ(state.orders.back().qty, 1);
	ASSERT_EQ(state.orders.back().side, Side::Buy);
	ASSERT_EQ(state.orders.back().security, &sec);
}

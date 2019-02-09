#include <modules/mod_time_close.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>

TEST(MpipeModuleTest, ModTimeCloseTest)
{
	Security sec = MakeSecurity(1, 10);
	State state({ 
		{ "sec", &sec } 
	});
	ModTimeClose time_close(10000);

	state.signal.ts = time_now() + 20000;
	time_close.Mutate(&state);

	ASSERT_EQ(state.orders.size(), 1);
	ASSERT_EQ(state.orders.back().qty, 10);
	ASSERT_EQ(state.orders.back().side, Side::Sell);
	ASSERT_EQ(state.orders.back().security, &sec);
}

#include <modules/mod_pos_limit.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>

TEST(MpipeModuleTest, ModPosLimitTest)
{
	Security sec = MakeSecurity(1, 1);
	State state({ 
		{ "sec", &sec } 
	});
	ModPosLimit pos_limit(1);

	state.signal = MakeOpenLongNow();
	state.orders.emplace_back(1, Side::Buy, &sec);

	pos_limit.Mutate(&state);
	ASSERT_EQ(state.orders.back().qty, 0);
}

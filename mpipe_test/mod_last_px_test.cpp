#include <modules/mod_last_px.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>

TEST(MpipeModuleTest, ModLastPxTest)
{
	Security sec = MakeSecurity(1, 0);
	State state({
		{ "sec", &sec }
	});
	ModLastPx mod_last_px(1);

	sec.last_px = 42;
	sec.last_qty = 1;
	sec.last_ts = time_now();
	state.signal = MakeOpenLongNow();
	state.orders.emplace_back(10, Side::Buy, &sec);

	mod_last_px.Mutate(&state);
	ASSERT_EQ(state.orders.back().px, 43);
}

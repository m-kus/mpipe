#include <modules/mod_value_slicer.hpp>
#include <gtest/gtest.h>

#include <utils.hpp>

TEST(ModValueSlicerTest, SimpleCase)
{
	Security sec1 = MakeSecurity();
	Security sec2 = MakeSecurity();
	Security sec3 = MakeSecurity();
	State state({});
	state.securities.push_back(&sec1);
	state.securities.push_back(&sec2);
	state.securities.push_back(&sec3);

	ModValueSlicer m(1);

	sec1.margin_buy = 0.0;
	sec1.leg_weight = 0.2;
	sec1.px_decimals = 8;
	sec2.margin_buy = 0.1;
	sec2.leg_weight = 0.3;
	sec2.px_decimals = 8;
	sec3.margin_buy = 0.1;
	sec3.leg_weight = 0.5;
	sec3.px_decimals = 8;

	state.signal = MakeOpenLongNow();
	m.Mutate(&state);

	ASSERT_EQ(state.orders.front().qty, 300000000);
	ASSERT_EQ(state.orders.back().qty, 500000000);
}

#include <modules/mod_equalizer.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>

TEST(MpipeModuleTest, ModEqualizerTest)
{
	Security sec1 = MakeSecurity(1, 0);
	Security sec2 = MakeSecurity(-1, 0);
	State state({
		{ "sec1", &sec1 },
		{ "sec2", &sec2 }
	});
	ModEqualizer equalizer;

	state.signal = MakeOpenLongNow();
	state.orders.emplace_back(10, Side::Buy, &sec1);
	state.orders.emplace_back(20, Side::Sell, &sec2);

	equalizer.Mutate(&state);
	ASSERT_EQ(state.orders[0].qty, 10);
	ASSERT_EQ(state.orders[1].qty, 10);
}

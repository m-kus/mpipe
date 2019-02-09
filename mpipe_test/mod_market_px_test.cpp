#include <modules/mod_market_px.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>

TEST(MpipeModuleTest, ModMarketPxTest)
{
	Security sec = MakeSecurity(1, 0);
	State state({ 
		{ "sec", &sec } 
	});
	ModMarketPx market_px(2, 1);
	
	sec.book.SetAsk(42, 1);
	sec.book.SetBid(42, 1);
	sec.book.ts = time_now();
	state.signal = MakeOpenLongNow();
	state.orders.emplace_back(10, Side::Buy, &sec);

	market_px.Mutate(&state);
	ASSERT_EQ(state.orders.back().qty, 2);
	ASSERT_EQ(state.orders.back().px, 43);
}
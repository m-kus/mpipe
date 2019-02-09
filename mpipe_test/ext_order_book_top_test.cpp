#include <extensions/ext_order_book_top.hpp>
#include <mcast/mcast_host.hpp>
#include <mcast/mcast_feed.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>

TEST(ExtOrderBookTopTest, Simple)
{
	Security sec = MakeSecurity();
	State state({
		{ "sec", &sec }
	});

	ExtOrderBookTop top;
	McastHost host(&sec, { &top });
	McastFeed feed({ &host });

	feed.UpdateBegin(time_now());
	feed.Update(MakeUpdate(sec.full_code, McastUpdateAction::New, McastEntryType::Bid, 49, 15, 1));
	feed.Update(MakeUpdate(sec.full_code, McastUpdateAction::New, McastEntryType::Ask, 51, 16, 2));
	feed.UpdateEnd(time_now());

	ASSERT_TRUE(BookGood(state.securities));
	ASSERT_EQ(sec.book.bid->second, 15);
	ASSERT_EQ(sec.book.ask->second, 16);
}

TEST(ExtOrderBookTopTest, Poloniex)
{
	Security sec = MakeSecurity(0, 0, "BTC_ETH");
	State state({
		{ "eth", &sec }
	});

	ExtOrderBookTop top;
	McastHost host(&sec, { &top });
	McastFeed feed({ &host });
	
	McastBookUpdate bid{};
	bid.full_code = "BTC_ETH";
	bid.action = McastUpdateAction::New;
	bid.type = McastEntryType::Bid;
	bid.ts = time_now();
	bid.px = 1070076;
	bid.qty = 66371544355;
	bid.rpt_seq = 1;

	McastBookUpdate ask{};
	ask.full_code = "BTC_ETH";
	ask.action = McastUpdateAction::New;
	ask.type = McastEntryType::Ask;
	ask.ts = time_now();
	ask.px = 1070096;
	ask.qty = 91180156631;
	ask.rpt_seq = 1;

	std::vector<McastBookUpdate> updates;
	updates.push_back(bid);
	updates.push_back(ask);

	feed.Start(&state, {});
	feed.ApplyUpdates(updates);

	ASSERT_TRUE(BookGood(state.securities));
	ASSERT_EQ(sec.book.bid->first, bid.px);
	ASSERT_EQ(sec.book.ask->first, ask.px);
}

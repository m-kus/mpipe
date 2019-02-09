#include <mcast/mcast_book.hpp>
#include <mcast/mcast_feed.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>

struct FakeSnapshotFeed : public McastFeed
{
	bool recover;

	FakeSnapshotFeed(State* state)
		: McastFeed({}, true)
		, recover(false)
	{
		state_ = state;
	}

	void Start(State* state, const std::vector<Module*>& pipe) override
	{
		recover = true;
	}
};

TEST(McastBookTest, Snapshot)
{
	Security sec = MakeSecurity();
	State state({
		{ "sec", &sec }
	});
	UpdatesCounterBook book(&sec);
	FakeSnapshotFeed feed(&state);
	
	book.Register(&feed);
	book.SnapshotUpdateBegin(time_now());
	
	book.SnapshotUpdate(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Bid, 49, 1, 42));
	book.SnapshotUpdate(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Ask, 51, 1, 42));

	book.Update(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Bid, 49, 1, 2));
	book.Update(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Ask, 51, 1, 43));
	book.SnapshotUpdateEnd(time_now());

	ASSERT_TRUE(book.Online());
	ASSERT_EQ(book.counter, 3);
}

TEST(McastBookTest, Recover)
{
	Security sec = MakeSecurity();
	State state({
		{ "sec", &sec }
	});
	UpdatesCounterBook book(&sec);
	FakeSnapshotFeed feed(&state);

	book.Register(&feed);
	book.ForceOnline(1);

	book.Update(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Bid, 49, 1, 2));
	book.Update(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Ask, 51, 1, 4));
	book.UpdateEnd(time_now());

	ASSERT_TRUE(feed.recover);
}

TEST(McastBookTest, Tolerance)
{
	Security sec = MakeSecurity();
	State state({
		{ "sec", &sec }
	});
	UpdatesCounterBook book(&sec, 2);
	FakeSnapshotFeed feed(&state);

	book.Register(&feed);
	book.ForceOnline(1);
	book.Update(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Bid, 49, 1, 2));
	book.Update(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Ask, 51, 1, 4));
	book.Update(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Bid, 48, 1, 5));
	book.Update(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Ask, 52, 1, 6));
	book.UpdateEnd(time_now());

	ASSERT_TRUE(feed.recover);
	book.Reset();

	book.SnapshotUpdateBegin(time_now());
	book.SnapshotUpdate(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Bid, 49, 1, 3));
	book.SnapshotUpdate(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Ask, 51, 1, 3));
	book.SnapshotUpdateEnd(time_now());

	ASSERT_TRUE(book.Online());

	book.Update(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Bid, 48, 1, 7));
	book.Update(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Ask, 52, 1, 9));

	ASSERT_TRUE(book.Online());
}

TEST(McastBookTest, BigRptSeq)
{
	Security sec = MakeSecurity();
	State state({
		{ "sec", &sec }
	});
	UpdatesCounterBook book(&sec);
	FakeSnapshotFeed feed(&state);

	book.Register(&feed);
	book.ForceOnline(1);
	book.Update(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Bid, 48, 1, 2));
	book.Update(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Ask, 52, 1, 1024));
	book.UpdateEnd(time_now());

	ASSERT_TRUE(feed.recover);
	feed.recover = false;
	book.Reset();

	book.SnapshotUpdateBegin(time_now());
	book.SnapshotUpdate(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Bid, 49, 1, 3));
	book.SnapshotUpdate(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Ask, 51, 1, 3));
	book.SnapshotUpdateEnd(time_now());

	ASSERT_FALSE(feed.recover);
	ASSERT_TRUE(book.Online());
}

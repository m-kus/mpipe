#include <mcast/mcast_feed.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>

struct FakeIncrementalFeed : public McastFeed
{
	std::unique_ptr<std::thread> loop;
	
	using McastFeed::McastFeed;

	void LoopProc()
	{
		std::vector<McastBookUpdate> updates;
		updates.push_back(MakeUpdate("sec", McastUpdateAction::New, McastEntryType::Bid, 49, 1, 1));
		ApplyUpdates(updates);
	}

	void StartImpl() override
	{
		loop.reset(new std::thread([this] { 
			LoopProc(); 
		}));
	}

	void StopImpl() override
	{
		loop->join();
	}
};

TEST(MpipeMcastTest, McastFeedTest)
{
	Security sec = MakeSecurity();
	State state({
		{ "sec", &sec }
	});
	UpdatesCounterBook book(&sec);
	FakeIncrementalFeed feed({ &book });

	ASSERT_TRUE(book.Online());

	feed.Start(&state, {});
	feed.Stop();
	ASSERT_EQ(book.counter, 1);
}

#include <core/mpipe_book.hpp>
#include <util/time.hpp>

#include <gtest/gtest.h>

using namespace mpipe;

TEST(MpipeBookTest, Invalid)
{
	Book book;	
	ASSERT_FALSE(book.Valid());
	ASSERT_EQ(book.bid, book.book.end());
	ASSERT_EQ(book.ask, book.book.end());
}

TEST(MpipeBookTest, SetBidAsk)
{
	Book book;
	book.SetBid(1, 1);
	book.SetAsk(2, 2);

	ASSERT_TRUE(book.Valid());
	ASSERT_EQ(book.bid->first, 1);
	ASSERT_EQ(book.bid->second, 1);
	ASSERT_EQ(book.ask->first, 2);
	ASSERT_EQ(book.ask->second, 2);
}

TEST(MpipeBookTest, Reset)
{
	Book book;
	book.SetBid(1, 1);
	book.SetAsk(2, 2);
	book.Commit(time_now());
	book.Reset();

	ASSERT_EQ(book.ts, 0);
	ASSERT_TRUE(book.book.empty());
	ASSERT_EQ(book.bid, book.book.end());
	ASSERT_EQ(book.ask, book.book.end());
}

TEST(MpipeBookTest,TrySetBidAsk)
{
	Book book;
	book.SetBid(48, 1);
	book.SetAsk(52, 1);
	book.TrySetBid(49, 1);
	book.TrySetAsk(51, 1);

	ASSERT_EQ(book.bid->first, 49);
	ASSERT_EQ(book.ask->first, 51);

	book.TrySetAsk(52, 2);
	book.TrySetBid(49, 2);

	ASSERT_EQ(book.bid->second, 2);
	ASSERT_EQ(book.ask->first, 51);
}

TEST(MpipeBookTest, Delete)
{
	Book book;
	book.Set(47, 1);
	book.Set(56, 0);
	book.Set(58, 1);
	book.SetBid(48, 1);
	book.SetAsk(52, 1);

	book.Delete(48);
	book.Delete(52);

	ASSERT_EQ(book.bid->first, 47);
	ASSERT_EQ(book.ask->first, 58);
}

TEST(MpipeBookTest, DeleteAll)
{
	Book book;
	book.SetBid(48, 1);
	book.SetAsk(52, 1);

	book.Delete(48);
	book.Delete(52);

	ASSERT_EQ(book.bid, book.book.end());
	ASSERT_EQ(book.ask, book.book.end());
}

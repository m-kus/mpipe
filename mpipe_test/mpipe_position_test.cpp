#include <core/mpipe_position.hpp>

#include <gtest/gtest.h>

using namespace mpipe;

TEST(MpipePositionTest, PositionChunkCollapse)
{
	PositionChunk c1(1, 10);
	PositionChunk c2(2, 10);

	ASSERT_TRUE(c1.Collapse(c2));
	ASSERT_EQ(c1.qty, 0);
	ASSERT_EQ(c2.qty, 0);
}

TEST(MpipePositionTest, PositionChunkCollapseNot)
{
	PositionChunk c1(1, 20);
	PositionChunk c2(2, 10);

	ASSERT_FALSE(c1.Collapse(c2));
	ASSERT_EQ(c1.qty, 10);
	ASSERT_EQ(c2.qty, 0);
}

TEST(MpipePositionTest, PositionChunkMerge)
{
	PositionChunk c1(1, 20);
	PositionChunk c2(2, 10);

	ASSERT_TRUE(c1.Merge(c2));
	ASSERT_EQ(c1.qty, 30);
	ASSERT_EQ(c2.qty, 10);
}

TEST(MpipePositionTest, PositionChunkMergeNot)
{
	PositionChunk c1(1, 20);
	PositionChunk c2(60002, 10);

	ASSERT_FALSE(c1.Merge(c2));
	ASSERT_EQ(c1.qty, 20);
	ASSERT_EQ(c2.qty, 10);
}

TEST(MpipePositionTest, HowMuchLong)
{
	Position p{};
	p.position = 3;
	p.buy_limit = 2;

	ASSERT_EQ(p.HowMuch(Side::Buy, 5), 0);
	ASSERT_EQ(p.HowMuch(Side::Buy, 0), 0);
	ASSERT_EQ(p.HowMuch(Side::Sell, 0), 3);
	ASSERT_EQ(p.HowMuch(Side::Sell, 5), 8);
}

TEST(MpipePositionTest, HowMuchShort)
{
	Position p{};
	p.position = -3;
	p.sell_limit = 2;

	ASSERT_EQ(p.HowMuch(Side::Sell, 5), 0);
	ASSERT_EQ(p.HowMuch(Side::Sell, 0), 0);
	ASSERT_EQ(p.HowMuch(Side::Buy, 0), 3);
	ASSERT_EQ(p.HowMuch(Side::Buy, 5), 8);
}

TEST(MpipePositionTest, PendingApply)
{
	Position p{};	
	p.PendingApply(1, Side::Buy);
	p.PendingApply(1, Side::Sell);

	ASSERT_EQ(p.buy_limit, 1);
	ASSERT_EQ(p.sell_limit, 1);
}

TEST(MpipePositionTest, ChunksApply)
{
	Position p{};
	p.ChunksApply(10000, 10, Side::Buy);
	p.ChunksApply(80000, 10, Side::Buy);
	p.ChunksApply(150000, 10, Side::Buy);	
	ASSERT_EQ(p.buy_chunks.size(), 3);

	p.ChunksApply(160000, 5, Side::Sell);
	ASSERT_EQ(p.sell_chunks.size(), 0);
	ASSERT_EQ(p.buy_chunks.size(), 3);
	ASSERT_EQ(p.buy_chunks.front().qty, 5);

	p.ChunksApply(160000, 25, Side::Sell);
	ASSERT_EQ(p.buy_chunks.size(), 0);
	ASSERT_EQ(p.sell_chunks.size(), 0);

	p.ChunksApply(160000, 10, Side::Sell);
	ASSERT_EQ(p.sell_chunks.size(), 1);
}

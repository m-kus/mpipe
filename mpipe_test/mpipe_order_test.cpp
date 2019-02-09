#include <core/mpipe_order.hpp>

#include <gtest/gtest.h>

using namespace mpipe;

TEST(MpipeOrderTest, AFFFC)
{
	Security s{};
	Order order(10, Side::Buy, &s);

	order.Add(1);
	ASSERT_EQ(s.pos.buy_limit, 10);

	order.Fill(2, 1, 1);
	ASSERT_EQ(order.qty, 9);
	ASSERT_EQ(s.pos.buy_limit, 9);
	ASSERT_EQ(s.pos.position, 1);
	ASSERT_EQ(s.pos.buy_chunks.size(), 1);
	ASSERT_EQ(s.pos.buy_chunks.front().qty, 1);

	order.Fill(3, 1, 1);
	ASSERT_EQ(order.qty, 8);
	ASSERT_EQ(s.pos.buy_limit, 8);
	ASSERT_EQ(s.pos.position, 2);
	ASSERT_EQ(s.pos.buy_chunks.size(), 1);
	ASSERT_EQ(s.pos.buy_chunks.back().qty, 2);

	order.Fill(60100, 1, 1);
	ASSERT_EQ(order.qty, 7);
	ASSERT_EQ(s.pos.buy_limit, 7);
	ASSERT_EQ(s.pos.position, 3);
	ASSERT_EQ(s.pos.buy_chunks.size(), 2);
	ASSERT_EQ(s.pos.buy_chunks.back().qty, 1);

	order.Cancel(60200);
	ASSERT_EQ(order.qty, 0);
	ASSERT_EQ(s.pos.buy_limit, 0);
}

TEST(MpipeOrderTest, AC)
{
	Security s{};
	Order order(10, Side::Sell, &s);

	order.Add(1); 
	ASSERT_EQ(order.qty, 10);
	ASSERT_EQ(s.pos.sell_limit, 10);
	
	order.Cancel(2);
	ASSERT_EQ(order.qty, 0);
	ASSERT_EQ(s.pos.sell_limit, 0);
}

TEST(MpipeOrderTest, AF)
{
	Security s{};
	Order order(10, Side::Sell, &s);

	order.Add(1);
	ASSERT_EQ(order.qty, 10);
	ASSERT_EQ(s.pos.sell_limit, 10);

	order.Fill(2, 10, 2);
	ASSERT_EQ(order.qty, 0);
	ASSERT_EQ(s.pos.sell_limit, 0);
	ASSERT_EQ(s.pos.position, -10);
	ASSERT_EQ(s.pos.sell_chunks.size(), 1);
	ASSERT_EQ(s.pos.sell_chunks.back().qty, 10);
}

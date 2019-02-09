#include <extensions/ext_order_book_agg.hpp>
#include <mcast/mcast_host.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>

TEST(ExtOrderBookTopAgg, Simple)
{
	Security s{};
	ExtOrderBookAgg agg;
	McastHost host(&s, { &agg });

		
}

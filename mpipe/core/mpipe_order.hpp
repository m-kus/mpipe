#pragma once

#include <core/mpipe_security.hpp>
#include <util/exception.hpp>
#include <util/logger.hpp>

namespace mpipe
{
	struct Order
	{
		Side side;
		std::int64_t ts;
		std::int64_t px;
		std::int64_t qty;
		OrderStatus status;
		Security *security;

		Order(std::int64_t _qty, Side _side, Security* _security)
			: side(_side)
			, ts(0)
			, px(0)
			, qty(_qty)
			, status(OrderStatus::Pending)
			, security(_security)
		{
		}

		void Place(std::int64_t last_ts)
		{
			ts = last_ts;
			status = OrderStatus::Placed;

			log_format(security->__str__().c_str());
		}

		void Add(std::int64_t last_ts, OrderStatus last_status = OrderStatus::Placed)
		{
			ts = last_ts;
			status = last_status;

			security->pos.PendingApply(qty, side);

			log_format(__str__(qty, px).c_str());
			log_format(security->__str__().c_str());
		}
		
		void Fill(std::int64_t last_ts, std::int64_t last_qty, std::int64_t last_px)
		{
			ts = last_ts;
			status = OrderStatus::Filled;

			security->pos.PendingApply(-last_qty, side);
			security->pos.ChunksApply(ts, last_qty, side);
			security->pos.position += side * last_qty;

			qty -= last_qty;	

			log_format( __str__(last_qty, last_px).c_str());
			log_format(security->__str__().c_str());

			if (qty < 0)
				throw mpipe_error_message("Filled more than requested\r\n");
		}

		void Cancel(std::int64_t last_ts)
		{
			ts = last_ts;
			status = OrderStatus::Canceled;

			security->pos.PendingApply(-qty, side);

			qty = 0;

			log_format(security->__str__().c_str());
		}

		std::string __repr__(const std::string& prefix = "")
		{
			std::stringstream ss;
			ss << prefix << "qty: " << qty << " (" << decimal_format(qty, security->qty_decimals) << ")" << std::endl;
			ss << prefix << "side: " << side_str(side) << std::endl;
			ss << prefix << "px: " << px << " (" << decimal_format(px, security->px_decimals) << ")" << std::endl;
			ss << prefix << "security: " << security->full_code << " @ " << security->class_code << std::endl;

			return ss.str();
		}

		std::string __str__(std::int64_t last_qty, std::int64_t last_px)
		{
			return string_format("%s %s: %s (%s) @ %s [%s]",
				side_str(side),
				security->full_code.c_str(),
				decimal_format(last_qty, security->qty_decimals).c_str(),
				decimal_format(qty, security->qty_decimals).c_str(),
				decimal_format(last_px, security->px_decimals).c_str(),
				ts_format_log(ts).c_str()
			);
		}
	};
}

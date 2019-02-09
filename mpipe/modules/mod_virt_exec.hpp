#pragma once

#include <core/mpipe.hpp>
#include <util/path.hpp>

#include <thread>
#include <iostream>
#include <fstream>

namespace mpipe
{
	class ModVirtExec : public Module
	{
	private:
		
		bool random_unfill_;
		std::int32_t counter_;
		std::int64_t prev_ts_;
		std::int64_t trade_timeout_;
		std::int64_t cancel_timeout_;

		std::fstream trades_file_;
		std::unordered_map<std::int32_t, Order> orders_;

		const char* trades_name_ = "virtual_trades.csv";

	protected:

		void OrderAdd(Order& order)
		{
			orders_.emplace(++counter_, order);
			
			if (random_unfill_
				&& rand() % 2 == 0)
			{
				return;
			}

			std::thread trade([&]() { 
				OnTrade(counter_, OrderStatus::Filled); });
			trade.detach();
		}

		void OrderDel(std::int64_t client_order_id)
		{
			std::thread trade([&]() { 
				OnTrade(client_order_id, OrderStatus::Canceled); });
			trade.detach();
		}

		void OnTrade(std::int64_t client_order_id, OrderStatus status)
		{
			ScopedState state(GetState());
			auto& order = orders_.at(client_order_id);

			if (status == OrderStatus::Filled)
			{
				double px = decimal_double(order.px, order.security->px_decimals);
				trades_file_ << string_format("%s,%s,%s,%lld,%f,%lld,%f,0,virt_exec\n",
					order.security->class_code.c_str(),
					order.security->full_code.c_str(),
					side_str(order.side),
					order.ts,
					px,
					order.qty,
					order.qty * px
				);
				trades_file_.flush();
				
				order.Fill(state->signal.ts, order.qty, order.px);
			}
			else
			{
				order.Cancel(state->signal.ts);
			}

			ExecutePipe(order.ts, false);
			auto _ = orders_.erase(client_order_id);
		}

	public:

		ModVirtExec(const std::string& base_dir,
			std::int64_t trade_timeout = 1000,
			std::int64_t cancel_timeout = 1000,	
			bool random_unfill = false)
			: random_unfill_(random_unfill)
			, counter_(0)
			, prev_ts_(0)
			, trade_timeout_(trade_timeout)
			, cancel_timeout_(cancel_timeout)
		{
			srand(std::time(nullptr));

			DirectoriesCreate(base_dir);
			auto path = string_format("%s/%s", base_dir.c_str(), trades_name_);

			trades_file_.open(path, std::fstream::in | std::fstream::out);
			if (trades_file_.is_open())
			{
				trades_file_.seekg(0, std::ios::end);
				if (trades_file_.tellg() > 0)
					return;
			}
			else
			{
				trades_file_.open(path, std::fstream::out);
			}

			trades_file_ << "class,sec,side,ts,px,qty,value,fee,comment\n";
		}

		void Mutate(State* state) override
		{
			if (!state->orders.empty()
				&& state->signal.ts >= prev_ts_ + trade_timeout_)
			{
				prev_ts_ = state->signal.ts;

				for (auto& order : state->orders)
				{
					if (order.qty > 0)
					{
						order.Add(state->signal.ts);
						OrderAdd(order);
					}
				}
			}

			for (auto& item : orders_)
			{
				if (state->signal.ts >= item.second.ts + cancel_timeout_)
				{
					item.second.ts = state->signal.ts;
					OrderDel(item.first);
				}
			}
		}
	};
}

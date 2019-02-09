#pragma once

#include <binding.hpp>
#include <core/mpipe_enum.hpp>

using namespace mpipe;

inline void MpipeEnumBinding(py::module& m)
{
	py::enum_<Action>(m, "Action", py::arithmetic())
		.value("Terminate", Action::Terminate)
		.value("Bypass", Action::Bypass)
		.value("ForceClose", Action::ForceClose)
		.value("Close", Action::Close)
		.value("Open", Action::Open)
		.export_values();

	py::enum_<Side>(m, "Side", py::arithmetic())
		.value("All", Side::All)
		.value("Buy", Side::Buy)
		.value("Sell", Side::Sell)
		.export_values();

	py::enum_<OrderStatus>(m, "OrderStatus", py::arithmetic())
		.value("Pending", OrderStatus::Pending)
		.value("Placed", OrderStatus::Placed)
		.value("Filled", OrderStatus::Filled)
		.value("Canceled", OrderStatus::Canceled)
		.export_values();
}

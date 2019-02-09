#pragma once

#include <binding.hpp>
#include <core/mpipe_order.hpp>

using namespace mpipe;

inline void MpipeOrderBinding(py::module& m)
{
	py::class_<Order>(m, "Order", py::dynamic_attr())
		.def(py::init<std::int64_t, Side, Security*>(),
			py::arg("qty"),
			py::arg("side"),
			py::arg("security"))
		.def_readwrite("ts", &Order::ts)
		.def_readwrite("qty", &Order::qty)
		.def_readwrite("side", &Order::side)
		.def_readwrite("px", &Order::px)
		.def_readwrite("security", &Order::security)
		.def("Fill", &Order::Fill,
			py::arg("ts"),
			py::arg("last_qty"),
			py::arg("last_px"))
		.def("Add", &Order::Add, py::arg("ts"), py::arg("status") = OrderStatus::Placed)
		.def("Cancel", &Order::Cancel, py::arg("ts"))
		.def("__repr__", &Order::__repr__, py::arg("prefix") = "");
}

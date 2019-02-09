#pragma once

#include <binding.hpp>
#include <core/mpipe_security.hpp>

using namespace mpipe;

inline void MpipeSecurityBinding(py::module& m)
{
	py::class_<Security>(m, "Security", py::dynamic_attr())
		.def(py::init<>())
		.def_readwrite("full_code", &Security::full_code)
		.def_readwrite("class_code", &Security::class_code)
		.def_readwrite("px_decimals", &Security::px_decimals)
		.def_readwrite("px_step", &Security::px_step)
		.def_readwrite("qty_decimals", &Security::qty_decimals)
		.def_readwrite("qty_step", &Security::qty_step, "minimal step in order")
		.def_readwrite("leg_factor", &Security::leg_factor)
		.def_readwrite("leg_weight", &Security::leg_weight)
		.def_readwrite("book", &Security::book)
		.def_readwrite("pos", &Security::pos)
		.def_readwrite("last_ts", &Security::last_ts, "last trade time")
		.def_readwrite("last_px", &Security::last_px)
		.def_readwrite("last_qty", &Security::last_qty)
		.def_readwrite("margin_buy", &Security::margin_buy)
		.def_readwrite("margin_sell", &Security::margin_sell)
		.def("__repr__", &Security::__repr__, py::arg("prefix") = "")
		.def("__str__", &Security::__str__);

	m.def("GetMargin", &GetMargin, py::arg("side"), py::arg("security"));
	m.def("BookGood", &BookGood, py::arg("securities"));
	m.def("LastGood", &LastGood, py::arg("securities"));
}

#pragma once

#include <binding.hpp>
#include <core/mpipe_signal.hpp>

using namespace mpipe;

inline void MpipeSignalBinding(py::module& m)
{
	py::class_<Signal>(m, "Signal", py::dynamic_attr())
		.def(py::init<>())
		.def_readwrite("ts", &Signal::ts)
		.def_readwrite("side", &Signal::side)
		.def_readwrite("action", &Signal::action)
		.def("Reset", &Signal::Reset)
		.def("Tradable", &Signal::Tradable)
		.def("__repr__", &Signal::__repr__, py::arg("prefix") = "");
}

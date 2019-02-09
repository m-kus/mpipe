#pragma once

#include <binding.hpp>
#include <unicodeobject.h>

#include <util/rolling.hpp>
#include <util/series.hpp>
#include <util/lazy.hpp>
#include <util/time.hpp>
#include <util/logger.hpp>

using namespace mpipe;

PYBIND11_MAKE_OPAQUE(std::vector<double>);

inline void PyLog(const std::string& message)
{
	PyThreadState *state = PyThreadState_Get();
	PyFrameObject *frame = state->frame;
	std::string function = frame ? PyUnicode_AsUTF8(frame->f_code->co_name) : "";
	Logger::WriteDefault(function, message);
}

inline void UtilBinding(py::module& m)
{
	DefineNumpyVector<double>(m, "VectorDouble");

	py::class_<Series>(m, "Series")
		.def(py::init<int64_t>(), py::arg("timeframe"))
		.def("Update", &Series::Update)
		.def_readwrite("index", &Series::index)
		.def_readwrite("values", &Series::values);

	py::class_<LazyTimer>(m, "LazyTimer")
		.def(py::init<int64_t, int64_t>(),
			py::arg("timeframe") = 60000,
			py::arg("offset") = 0)
		.def("TimerEvent", &LazyTimer::TimerEvent, 
			py::arg("ts"), 
			py::arg("include_first") = false);

	m.def("time_now", &time_now);
	m.def("log", &PyLog, py::arg("message"));
	m.def("log_init", &LogInit, py::arg("console"), py::arg("async"));
	m.def("sleep_mcs", &sleep_mcs, py::call_guard<py::gil_scoped_release>());
}

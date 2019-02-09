#pragma once

#include <binding.hpp>
#include <util/rolling.hpp>
#include <util/series.hpp>

using namespace mpipe;

class PyRollingBase : public RollingBase
{
public:

	using RollingBase::RollingBase;

	void OnPush(double prev_back, double new_back) override
	{
		PYBIND11_OVERLOAD(void, RollingBase, OnPush, prev_back, new_back);
	}

	void OnPop(double prev_front, double new_front) override
	{
		PYBIND11_OVERLOAD(void, RollingBase, OnPop, prev_front, new_front);
	}

	double calculate()
	{
		PYBIND11_OVERLOAD(double, RollingBase, calculate);
	}
};

inline void RollingBinding(py::module& m)
{
	py::class_<RollingBase, PyRollingBase>(m, "RollingBase")
		.def(py::init<int64_t, int64_t>(),
			py::arg("window"),
			py::arg("timeframe") = 60000)
		.def("Good", &RollingBase::Good, "ready to use, warmed")
		.def("Update", &RollingBase::Update,
			py::arg("ts"),
			py::arg("value"),
			"updates internal state, ts in millis and value is double")
		.def("calculate", &RollingBase::calculate, "get final value");

	py::class_<RollingMean, RollingBase>(m, "RollingMean")
		.def(py::init<int64_t, int64_t>(),
			py::arg("window"),
			py::arg("timeframe") = 60000);

	py::class_<RollingStd, RollingBase>(m, "RollingStd")
		.def(py::init<int64_t, int64_t>(),
			py::arg("window"),
			py::arg("timeframe") = 60000)
		.def("mean", &RollingStd::mean);

	py::class_<RollingMinMax, RollingBase>(m, "RollingMinMax")
		.def(py::init<int64_t, int64_t, const std::function<bool(double, double)>&>(),
			py::arg("window"),
			py::arg("timeframe") = 60000,
			py::arg("func"))
		.def("previous", &RollingMinMax::previous);

	py::class_<RollingMin, RollingMinMax>(m, "RollingMin")
		.def(py::init<int64_t, int64_t>(),
			py::arg("window"),
			py::arg("timeframe") = 60000);

	py::class_<RollingMax, RollingMinMax>(m, "RollingMax")
		.def(py::init<int64_t, int64_t>(),
			py::arg("window"),
			py::arg("timeframe") = 60000);

	py::class_<RSI, RollingBase>(m, "RSI")
		.def(py::init<int64_t, int64_t>(),
			py::arg("window"),
			py::arg("timeframe") = 60000);

	py::class_<RollingShift, RollingBase>(m, "RollingShift")
		.def(py::init<int64_t, int64_t>(),
			py::arg("window"),
			py::arg("timeframe") = 60000,
			"window must be shift + 1")
		.def("diff", &RollingShift::diff);
}

#pragma once

#include <binding.hpp>
#include <mcast/mcast_feed.hpp>

using namespace mpipe;

class PyMcastFeed : public McastFeed
{
public:

	using McastFeed::McastFeed;

	void StartImpl() override
	{
		py::gil_scoped_acquire gil_acquire;
		try
		{
			PYBIND11_OVERLOAD(void, McastFeed, StartImpl);
		}
		catch (std::exception& e)
		{
			log_format("%s\r\n", e.what());
		}
	}

	void StopImpl() override
	{
		py::gil_scoped_acquire gil_acquire;
		try
		{
			PYBIND11_OVERLOAD(void, McastFeed, StopImpl);
		}
		catch (std::exception& e)
		{
			log_format("%s\r\n", e.what());
		}
	}
};

inline void McastFeedBinding(py::module& m)
{
	py::class_<McastFeed, Module, PyMcastFeed>(m, "McastFeed")
		.def(py::init<const std::vector<McastBook*>&, bool>(),
			py::arg("books"),
			py::arg("snapshot"))
		.def("UpdateBegin", &McastFeed::UpdateBegin,
			py::arg("ts"),
			py::call_guard<py::gil_scoped_release>())
		.def("Update", &McastFeed::Update,
			py::arg("update"),
			py::call_guard<py::gil_scoped_release>())
		.def("UpdateEnd", &McastFeed::UpdateEnd,
			py::arg("ts"),
			py::call_guard<py::gil_scoped_release>())
		.def("ApplyUpdates", &McastFeed::ApplyUpdates,
			py::arg("updates"),
			py::call_guard<py::gil_scoped_release>());
}

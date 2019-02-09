#pragma once

#include <binding.hpp>
#include <core/mpipe.hpp>

using namespace mpipe;

class PyModule : public Module
{
public:

	using Module::Module;

	void StartImpl() override
	{
		py::gil_scoped_acquire gil_acquire;
		try
		{
			PYBIND11_OVERLOAD(void, Module, StartImpl);
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
			PYBIND11_OVERLOAD(void, Module, StopImpl);
		}
		catch (std::exception& e)
		{
			log_format("%s\r\n", e.what());
		}
	}

	void Mutate(State* state) override
	{
		py::gil_scoped_acquire gil_acquire;
		try
		{
			PYBIND11_OVERLOAD(void, Module, Mutate, state);
		}
		catch (std::exception& e)
		{
			log_format("%s\r\n", e.what());
			state->signal.action = Action::Terminate;
		}
	}
};

inline void MpipeBinding(py::module& m)
{
	py::class_<Module, PyModule>(m, "Module", py::dynamic_attr())
		.def(py::init<>())
		.def("GetState", &Module::GetState)
		.def("StartImpl", &Module::StartImpl)
		.def("StopImpl", &Module::StopImpl)
		.def("Start", &Module::Start,
			py::arg("state"),
			py::arg("pipe"),
			py::call_guard<py::gil_scoped_release>())
		.def("Mutate", &Module::Mutate,
			py::arg("state"))
		.def("Stop", &Module::Stop,
			py::call_guard<py::gil_scoped_release>())
		.def("Join", &Module::Join,
			py::call_guard<py::gil_scoped_release>())
		.def("ExecutePipe", &Module::ExecutePipe,
			py::arg("ts"),
			py::arg("reset_signal") = true,
			py::call_guard<py::gil_scoped_release>());
}

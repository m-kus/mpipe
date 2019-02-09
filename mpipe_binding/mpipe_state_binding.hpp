#pragma once

#include <binding.hpp>
#include <core/mpipe_state.hpp>

using namespace mpipe;

inline void MpipeStateBinding(py::module& m)
{
	py::class_<spin_lock>(m, "spin_lock")
		.def(py::init<>())
		.def("try_lock", &spin_lock::try_lock)
		.def("lock", &spin_lock::lock)
		.def("unlock", &spin_lock::unlock)
		.def("__enter__", &spin_lock::lock)
		.def("__exit__",
			[](spin_lock &self, void* type, void* value, void* traceback) {
				self.unlock();
			}
		);

	py::class_<State, spin_lock>(m, "State", py::dynamic_attr())
		.def(py::init<const std::unordered_map<std::string, Security*>&>(), py::arg("securities"))
		.def_readwrite("signal", &State::signal, py::return_value_policy::copy)
		.def_readwrite("orders", &State::orders, py::return_value_policy::copy)
		.def_readonly("securities", &State::securities)
		.def("BookPending", &State::BookPending, py::arg("securities"))
		.def("LastPending", &State::LastPending, py::arg("securities"))
		.def("GetSecurity", &State::GetSecurity, py::arg("key"))
		.def("__repr__", &State::__repr__);
}

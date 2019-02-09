#pragma once

#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#undef min
#undef max

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

template<class T>
inline void DefineNumpyVector(py::module& m, const char* name)
{
	py::class_<std::vector<T>>(m, name, py::buffer_protocol())
		.def_buffer([](std::vector<T> &v) -> py::buffer_info {
		return py::buffer_info(
			v.data(),
			sizeof(T),
			py::format_descriptor<T>::format(),
			1,
			{ v.size() },
			{ sizeof(T) }
		);
	});
};

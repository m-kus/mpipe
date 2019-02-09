#pragma once

#include <binding.hpp>
#include <core/mpipe_order.hpp>

using namespace mpipe;

inline void MpipePositionBinding(py::module& m)
{
	py::class_<PositionChunk>(m, "PositionChunk")
		.def(py::init<std::int64_t, std::int64_t>(), 
			py::arg("ts"), 
			py::arg("qty"))
		.def_readwrite("ts", &PositionChunk::ts)
		.def_readwrite("qty", &PositionChunk::qty)
		.def("Collapse", &PositionChunk::Merge, py::arg("chunk"))
		.def("Merge", &PositionChunk::Merge, py::arg("chunk"));
	
	py::class_<Position>(m, "Position", py::dynamic_attr())
		.def(py::init<>())
		.def_readwrite("position", &Position::position)
		.def_readwrite("buy_limit", &Position::buy_limit)
		.def_readwrite("sell_limit", &Position::sell_limit)
		.def_readwrite("buy_chunks", &Position::buy_chunks)
		.def_readwrite("sell_chunks", &Position::sell_chunks)
		.def("HowMuch", &Position::HowMuch, 
			py::arg("side"), 
			py::arg("max_position") = 0)
		.def("ChunksApply", &Position::ChunksApply, 
			py::arg("ts"), 
			py::arg("qty"), 
			py::arg("side"))
		.def("PendingApply", &Position::PendingApply, 
			py::arg("delta"), 
			py::arg("side"))
		.def("__repr__", &Position::__repr__, py::arg("prefix") = "");
}

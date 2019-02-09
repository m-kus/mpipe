#pragma once

#include <binding.hpp>
#include <core/mpipe_book.hpp>

using namespace mpipe;

PYBIND11_MAKE_OPAQUE(book_map_t);

inline void MpipeBookBinding(py::module& m)
{
	py::bind_map<book_map_t>(m, "book_map_t");

	py::class_<Book>(m, "Book", py::dynamic_attr())
		.def(py::init<>())
		.def_readwrite("book", &Book::book)
		.def_readwrite("bid", &Book::bid)
		.def_readwrite("ask", &Book::ask)
		.def_readwrite("ts", &Book::ts)
		.def("Set", &Book::Set, py::arg("px"), py::arg("qty"))
		.def("SetBid", &Book::SetBid, py::arg("px"), py::arg("qty"))
		.def("SetAsk", &Book::SetAsk, py::arg("px"), py::arg("qty"))
		.def("TrySetBid", &Book::TrySetBid, py::arg("px"), py::arg("qty"))
		.def("TrySetAsk", &Book::TrySetAsk, py::arg("px"), py::arg("qty"))
		.def("Delete", &Book::Delete, py::arg("px"))
		.def("Valid", &Book::Valid)
		.def("Reset", &Book::Reset)
		.def("Commit", &Book::Commit, py::arg("ts"))
		.def("__repr__", &Book::__repr__, py::arg("prefix") = "");
}

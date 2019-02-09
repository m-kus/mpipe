#pragma once

#include <binding.hpp>
#include <mcast/mcast_extension.hpp>
#include <extensions/ext_order_book_top.hpp>
#include <extensions/ext_order_book_agg.hpp>

using namespace mpipe;

inline void ExtensionsBinding(py::module& m)
{
	py::class_<McastExtension>(m, "McastExtension")
		.def(py::init<const std::string&>(), py::arg("suffix"))
		.def_readwrite("host", &McastExtension::host)
		.def_readwrite("suffix", &McastExtension::suffix)
		.def("Reset", &McastExtension::Reset)
		.def("Online", &McastExtension::Online)
		.def("UpdateBegin", &McastExtension::UpdateBegin, py::arg("ts"))
		.def("Update", &McastExtension::Update, py::arg("update"))
		.def("UpdateEnd", &McastExtension::UpdateEnd, py::arg("ts"));
	
	py::class_<ExtOrderBookTop, McastExtension>(m, "ExtOrderBookTop")
		.def(py::init<>());

	py::class_<ExtOrderBookAgg, McastExtension>(m, "ExtOrderBookAgg")
		.def(py::init<>());
}

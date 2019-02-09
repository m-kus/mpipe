#pragma once

#include <binding.hpp>
#include <mcast/mcast_host.hpp>

using namespace mpipe;

inline void McastBinding(py::module& m)
{
	py::enum_<McastUpdateAction>(m, "McastUpdateAction", py::arithmetic())
		.value("New", McastUpdateAction::New)
		.value("Change", McastUpdateAction::Change)
		.value("Delete", McastUpdateAction::Delete)
		.value("Trade", McastUpdateAction::Trade)
		.value("Heartbeat", McastUpdateAction::Heartbeat)
		.export_values();

	py::enum_<McastEntryType>(m, "McastEntryType", py::arithmetic())
		.value("Bid", McastEntryType::Bid)
		.value("Ask", McastEntryType::Ask)
		.value("Buy", McastEntryType::Buy)
		.value("Sell", McastEntryType::Sell)
		.value("Empty", McastEntryType::Empty)
		.export_values();

	py::class_<McastBookUpdate>(m, "McastBookUpdate")
		.def(py::init<>())
		.def_readwrite("full_code", &McastBookUpdate::full_code)
		.def_readwrite("action", &McastBookUpdate::action)
		.def_readwrite("type", &McastBookUpdate::type)
		.def_readwrite("ts", &McastBookUpdate::ts)
		.def_readwrite("entry_id", &McastBookUpdate::entry_id)
		.def_readwrite("rpt_seq", &McastBookUpdate::rpt_seq)
		.def_readwrite("px", &McastBookUpdate::px)
		.def_readwrite("qty", &McastBookUpdate::qty)
		.def("__repr__", &McastBookUpdate::__repr__);
		
	py::class_<McastBook>(m, "McastBook")
		.def(py::init<Security*, std::size_t>(),
			py::arg("security"), 
			py::arg("tolerance") = 0)
		.def_readwrite("security", &McastBook::security)
		.def("Reset", &McastBook::Reset)
		.def("Online", &McastBook::Online)
		.def("Register", &McastBook::Register, py::arg("feed"))
		.def("UpdateBegin", &McastBook::UpdateBegin, py::arg("ts"))
		.def("Update", &McastBook::Update, py::arg("update"))
		.def("UpdateEnd", &McastBook::UpdateEnd, py::arg("ts"))
		.def("SnapshotUpdateBegin", &McastBook::SnapshotUpdateBegin, py::arg("ts"))
		.def("SnapshotUpdate", &McastBook::SnapshotUpdate, py::arg("update"))
		.def("SnapshotUpdateEnd", &McastBook::SnapshotUpdateEnd, py::arg("ts"));

	py::class_<McastHost, McastBook>(m, "McastHost")
		.def(py::init<Security*, const std::vector<McastExtension*>&, std::size_t>(),
			py::arg("security"), 
			py::arg("extensions"),
			py::arg("tolerance") = 0);
}

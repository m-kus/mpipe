#pragma once

#include <binding.hpp>
#include <modules/mod_balancer.hpp>
#include <modules/mod_hours.hpp>
#include <modules/mod_pos_limit.hpp>
#include <modules/mod_market_px.hpp>
#include <modules/mod_liquidity.hpp>
#include <modules/mod_last_px.hpp>
#include <modules/mod_virt_exec.hpp>
#include <modules/mod_time_close.hpp>
#include <modules/mod_equalizer.hpp>
#include <modules/mod_order_gen.hpp>
#include <modules/mod_restore_point.hpp>
#include <modules/mod_signal_gen.hpp>
#include <modules/mod_value_limit.hpp>
#include <modules/mod_value_slicer.hpp>
#include <modules/mod_value_unhedged.hpp>

using namespace mpipe;

inline void ModulesBinding(py::module& m)
{
	py::class_<ModBalancer, Module>(m, "ModBalancer")
		.def(py::init<>());
	
	py::class_<ModHoursRule>(m, "ModHoursRule")
		.def(py::init<const std::string&, Side, Action>(),
			py::arg("time"),
			py::arg("side") = Side::All,
			py::arg("rule"));
	
	py::class_<ModHours, Module>(m, "ModHours")
		.def(py::init<std::vector<ModHoursRule>>(), py::arg("rules"))
		.def("ForceClose", &ModHours::ForceClose, py::arg("min_period"));

	py::class_<ModPosLimit, Module>(m, "ModPosLimit")
		.def(py::init<int64_t>(), py::arg("max_pos") = 1);

	py::class_<ModEqualizer, Module>(m, "ModEqualizer")
		.def(py::init<>());

	py::class_<ModMarketPx, Module>(m, "ModMarketPx")
		.def(py::init<int64_t, int64_t>(),
			py::arg("min_qty") = 1,
			py::arg("slippage") = 0);

	py::class_<ModLiquidity, Module>(m, "ModLiquidity")
		.def(py::init<std::double_t, bool>(),
			py::arg("min_value"),
			py::arg("min_terminate") = false);

	py::class_<ModOrderGen, Module>(m, "ModOrderGen")
		.def(py::init<int64_t>(), py::arg("lot_size") = 1);

	py::class_<ModLastPx, Module>(m, "ModLastPx")
		.def(py::init<int64_t>(), py::arg("spread_pt") = 0);

	py::class_<ModVirtExec, Module>(m, "ModVirtExec")
		.def(py::init<const std::string&, int64_t, int64_t, bool>(),
			py::arg("base_dir"),
			py::arg("trade_timeout") = 1000,
			py::arg("cancel_timeout") = 1000,
			py::arg("random_unfill") = false);

	py::class_<ModTimeClose, Module>(m, "ModTimeClose")
		.def(py::init<int64_t>(), py::arg("timeout"));	
	
	py::class_<ModRestorePoint, Module>(m, "ModRestorePoint")
		.def(py::init<const std::string&>(), py::arg("base_dir"));

	py::class_<ModSignalGen, Module>(m, "ModSignalGen")
		.def(py::init<Action, Side>(), 
			py::arg("action"), 
			py::arg("side"));

	py::class_<ModValueLimit, ModPosLimit>(m, "ModValueLimit")
		.def(py::init<std::double_t, bool>(),
			py::arg("max_value"),
			py::arg("max_terminate") = false);

	py::class_<ModValueUnhedged, ModValueLimit>(m, "ModValueUnhedged")
		.def(py::init<std::double_t, std::double_t, bool>(),
			py::arg("max_value"),
			py::arg("max_unhedge") = 0,
			py::arg("max_terminate") = false);

	py::class_<ModValueSlicer, Module>(m, "ModValueSlicer")
		.def(py::init<std::double_t>(), py::arg("min_slice"));
}

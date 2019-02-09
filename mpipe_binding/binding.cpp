#include <mpipe_binding.hpp>
#include <mpipe_enum_binding.hpp>
#include <mpipe_book_binding.hpp>
#include <mpipe_order_binding.hpp>
#include <mpipe_position_binding.hpp>
#include <mpipe_security_binding.hpp>
#include <mpipe_signal_binding.hpp>
#include <mpipe_state_binding.hpp>
#include <mcast_binding.hpp>
#include <mcast_feed_binding.hpp>
#include <modules_binding.hpp>
#include <extensions_binding.hpp>
#include <rolling_binding.hpp>
#include <util_binding.hpp>

PYBIND11_PLUGIN(mpipe)
{
	py::module m("mpipe", "mpipe python binding");

	MpipeBinding(m);
	MpipeEnumBinding(m);
	MpipeBookBinding(m);
	MpipeOrderBinding(m);
	MpipePositionBinding(m);
	MpipeSecurityBinding(m);
	MpipeSignalBinding(m);
	MpipeStateBinding(m);
	McastBinding(m);
	McastFeedBinding(m);
	ModulesBinding(m);
	ExtensionsBinding(m);
	RollingBinding(m);
	UtilBinding(m);

	return m.ptr();
}

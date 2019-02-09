#include <modules/mod_restore_point.hpp>
#include <utils.hpp>

#include <gtest/gtest.h>


TEST(MpipeModuleTest, ModRestorePointTest)
{
	Security sec1 = MakeSecurity(1, 10, "sec1");
	Security sec2 = MakeSecurity(-1, -10, "sec2");
	State state({ 
		{ "sec1", &sec1 }, 
		{ "sec2", &sec2 } 
	});
	ModRestorePoint restore(".restore");

	restore.Mutate(&state);
	sec1.pos.Clear();
	sec2.pos.Clear();
	
	restore.Start(&state, {});
	ASSERT_EQ(sec1.pos.position, 10);
	ASSERT_EQ(sec2.pos.position, -10);
}

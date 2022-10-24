#include <ultra64.h>
#include "bss.h"
#include "constants.h"
#include "types.h"

struct buddy g_Buddies[NUM_BUDDIES] = {
    {
        BUDDY_NONE,
        CHEAT_NONE,
    	L_MPMENU_045,    // "None"
        GAILIST_INIT_DEFAULT_BUDDY, // AI List
        BODY_DARK_COMBAT,
        HEAD_VD,
        CHRFLAG0_NONE,
        CHRFLAG1_IGNORECOVER,
        VOICEBOX_FEMALE,
        50,  // Team Scan Distance
        100, // Accuracy Rating
        100, // Speed Rating
        20,  // Max Damage
        4,   // Health
        WEAPON_FALCON2,
        WEAPON_NONE,
    },
	{
        BUDDY_VELVETDARK,
        CHEAT_VELVETDARK,
		L_MPWEAPONS_117, // "Velvet Dark"
        GAILIST_INIT_DEFAULT_BUDDY, // AI List
        BODY_DARK_COMBAT,
        HEAD_VD,
        CHRFLAG0_NONE,
        CHRFLAG1_IGNORECOVER,
        VOICEBOX_FEMALE,
        50,  // Team Scan Distance
        100, // Accuracy Rating
        100, // Speed Rating
        20,  // Max Damage
        4,   // Health
        WEAPON_FALCON2,
        WEAPON_NONE,
    },
	{
        BUDDY_PUGILIST,
        CHEAT_PUGILIST,
		L_MPWEAPONS_097, // "Pugilist"
        GAILIST_INIT_PUGILIST_BUDDY,
        BODY_CARRINGTON,
        HEAD_JAMIE,
        CHRFLAG0_CHUCKNORRIS,
        CHRFLAG1_IGNORECOVER | CHRFLAG1_ADJUSTPUNCHSPEED | CHRFLAG1_HANDCOMBATONLY,
        VOICEBOX_MALE1,
        100, // Team Scan Distance
        100, // Accuracy Rating
        100, // Speed Rating
        20,  // Max Damage
        20,  // Health
        WEAPON_NONE,
        WEAPON_NONE,
    },
	{
        BUDDY_HOTSHOT,
        CHEAT_HOTSHOT,
		L_MPWEAPONS_098, // "Hotshot"
        GAILIST_INIT_DEFAULT_BUDDY,
        BODY_CISOLDIER,
        HEAD_CHRIST,
        CHRFLAG0_NONE,
        CHRFLAG1_IGNORECOVER,
        VOICEBOX_MALE0,
        100, // Team Scan Distance
        50,  // Accuracy Rating
        100, // Speed Rating
        20,  // Max Damage
        10,  // Health
        WEAPON_DY357LX,
        WEAPON_DY357MAGNUM,
    },
	{
        BUDDY_HITANDRUN,
        CHEAT_HITANDRUN,
		L_MPWEAPONS_099, // "Hit and Run"
        GAILIST_INIT_DEFAULT_BUDDY,
        BODY_MRBLONDE,
        HEAD_MARK2,
        CHRFLAG0_NONE,
        CHRFLAG1_PUNCHHARDER,
        VOICEBOX_MALE2,
        50,  // Team Scan Distance
        50,  // Accuracy Rating
        100, // Speed Rating
        10,  // Max Damage
        10,  // Health
        WEAPON_K7AVENGER,
        WEAPON_NONE,
    },
	{
        BUDDY_ALIEN,
        CHEAT_ALIEN,
		L_MPWEAPONS_100, // "Alien"
        GAILIST_INIT_DEFAULT_BUDDY,
        BODY_ELVIS1,
        HEAD_MAIAN_S,
        CHRFLAG0_NONE,
        CHRFLAG1_IGNORECOVER | CHRFLAG1_PUNCHHARDER,
        VOICEBOX_MALE0,
        150, // Team Scan Distance
        100, // Accuracy Rating
        100, // Speed Rating
        20,  // Max Damage
        10,  // Health
        WEAPON_RCP120,
        WEAPON_NONE,
    },
};

s32 playerSimulantBuddiesCount()
{
	s32 i;
	s32 count = 0;
	for (i = 0; i < ARRAYCOUNT(g_Vars.aibuddies); i++) {
		count += g_Vars.aibuddytype[i] != BUDDY_NONE;
	}
	return count;
}

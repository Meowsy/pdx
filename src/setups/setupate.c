//
// Duel
//

#include "stagesetup.h"

// Characters
#define CHR_GUARD    0x00
#define CHR_JONATHAN 0x01
#define CHR_TRENT    0x02

// Stage flags
#define STAGEFLAG_GUARD_DEAD      0x00000100
#define STAGEFLAG_JONATHAN_DEAD   0x00000200
#define STAGEFLAG_TRENT_DEAD      0x00000400
#define STAGEFLAG_SA_WALK_TRIGGER 0x00000800
#define STAGEFLAG_PA_WALK_TRIGGER 0x00001000

// AI Lists
#define AILIST_MOVE_TO_PAD  0x0401
#define AILIST_GUARD_COMBAT 0x0402
#define AILIST_JON_COMBAT   0x0403
#define AILIST_TRENT_COMBAT 0x0404
#define AILIST_GUARD_INIT   0x0405
#define AILIST_JON_INIT     0x0406
#define AILIST_TRENT_INIT   0x0407

extern s32 intro[];
extern u32 props[];
extern struct path paths[];
extern struct ailist ailists[];

struct stagesetup setup = {
	NULL,
	NULL,
	NULL,
	intro,
	props,
	paths,
	NULL,
	NULL,
};

u32 props[] = {
	briefing(BRIEFINGTYPE_LOCATION, L_ATE_001)
	briefing(BRIEFINGTYPE_TEXT_PA,  L_ATE_000)
	briefing(BRIEFINGTYPE_TEXT_SA,  L_ATE_002)
	briefing(BRIEFINGTYPE_TEXT_A,   L_ATE_003)

	beginobjective(0, L_ATE_004, (DIFFBIT_A | DIFFBIT_SA | DIFFBIT_PA | DIFFBIT_PD)) // "Defeat dataDyne guard"
		complete_flags(STAGEFLAG_GUARD_DEAD)
	endobjective

	beginobjective(1, L_ATE_005, (DIFFBIT_SA | DIFFBIT_PA | DIFFBIT_PD)) // "Defeat Jonathan Dark"
		complete_flags(STAGEFLAG_JONATHAN_DEAD)
	endobjective

	beginobjective(2, L_ATE_006, (DIFFBIT_PA | DIFFBIT_PD)) // "Defeat Trent Easton"
		complete_flags(STAGEFLAG_TRENT_DEAD)
	endobjective

	chr(SPAWNFLAG_00000200, 0x00, PAD_ATE_0279, BODY_DDSHOCK, HEAD_RANDOM, AILIST_GUARD_INIT, -1, -1, 100, 100, CHRFLAG0_CAN_THROW_GRENADES | CHRFLAG0_SQUADALERTANYDIST | CHRFLAG0_HOLD_POSITION | CHRFLAG0_COVER_TYPE2 | CHRFLAG0_NOHEAR, CHRFLAG1_STOPTRACKINGIFLOOKEDAT, TEAM_ENEMY, SQUADRON_04, -1, 0, 0, 0, 0, 0)
	weapon(0x0100, MODEL_CHRFALCON2, CHR_GUARD, OBJFLAG_ASSIGNEDTOCHR, 0, 0, WEAPON_FALCON2_SCOPE)
	chr(SPAWNFLAG_00000200, 0x01, PAD_ATE_0023, BODY_CISOLDIER, HEAD_JONATHAN, AILIST_JON_INIT, -1, -1, 100, 100, CHRFLAG0_CAN_THROW_GRENADES | CHRFLAG0_SQUADALERTANYDIST | CHRFLAG0_HOLD_POSITION | CHRFLAG0_COVER_TYPE2 | CHRFLAG0_NOHEAR, CHRFLAG1_STOPTRACKINGIFLOOKEDAT, TEAM_ENEMY, SQUADRON_04, -1, 0, 0, 0, 0, 0)
	weapon(0x0100, MODEL_CHRDY357, CHR_JONATHAN, OBJFLAG_ASSIGNEDTOCHR, 0, 0, WEAPON_DY357MAGNUM)
	chr(SPAWNFLAG_00000200, 0x02, PAD_ATE_0024, BODY_TRENT, HEAD_TRENT, AILIST_TRENT_INIT, -1, -1, 100, 100, CHRFLAG0_CAN_THROW_GRENADES | CHRFLAG0_SQUADALERTANYDIST | CHRFLAG0_HOLD_POSITION | CHRFLAG0_COVER_TYPE2 | CHRFLAG0_NOHEAR, CHRFLAG1_STOPTRACKINGIFLOOKEDAT, TEAM_ENEMY, SQUADRON_04, -1, 0, 0, 0, 0, 0)
	weapon(0x0100, MODEL_CHRDY357TRENT, CHR_TRENT, OBJFLAG_ASSIGNEDTOCHR, 0, 0, WEAPON_DY357LX)
	door(0x0100, MODEL_DD_OFFICEDOOR, PAD_ATE_0158, OBJFLAG_IGNOREFLOORCOLOUR, OBJFLAG2_AICANNOTUSE, 0, 1000, 0x0000f333, 0x00010000, 0x00007fff, 0x00014000, 0x00000666, DOORFLAG_0004, DOORTYPE_SLIDING, 0x00000040, 0x00000384, 0x00000000, 0x00000001, 0x00000800)
	door(0x0100, MODEL_DD_OFFICEDOOR, PAD_ATE_0159, OBJFLAG_IGNOREFLOORCOLOUR, OBJFLAG2_AICANNOTUSE, 0, 1000, 0x0000f333, 0x00010000, 0x00007fff, 0x00014000, 0x00000666, DOORFLAG_0004, DOORTYPE_SLIDING, 0x00000040, 0x00000384, 0x00000000, 0xffffffff, 0x00000000)
	endprops
};

s32 intro[] = {
	outfit(OUTFIT_DEFAULT)
	spawn(PAD_ATE_027A)
	intro_weapon(WEAPON_FALCON2_SCOPE, -1)
	ammo(AMMOTYPE_PISTOL, 8)
	endintro
};

struct path paths[] = {
	{ NULL, 0, 0 },
};

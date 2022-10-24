#include <ultra64.h>
#include "constants.h"
#include "lib/sched.h"
#include "lib/str.h"
#include "game/bondgun.h"
#include "game/camdraw.h"
#include "game/cheats.h"
#include "game/inv.h"
#include "game/playermgr.h"
#include "game/training.h"
#include "game/gamefile.h"
#include "game/menu.h"
#include "game/lang.h"
#include "game/pak.h"
#include "bss.h"
#include "data.h"
#include "types.h"

u32 g_CurCheatCategory;
u32 g_CurCheatCategoryOptions;

#define TIME(mins, secs) (mins * 60 + secs)
#define m
#define s

struct cheat g_Cheats[NUM_CHEATS] = {
	{ CHEATCAT_FUN,             CHEAT_DKMODE,                 L_MPWEAPONS_082,                0,                 SOLOSTAGEINDEX_CHICAGO,        DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // DK Mode
	{ CHEATCAT_FUN,             CHEAT_SMALLJO,                L_MPWEAPONS_085,                0,                 SOLOSTAGEINDEX_G5BUILDING,     DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // Small Jo
	{ CHEATCAT_FUN,             CHEAT_SMALLCHARACTERS,        L_MPWEAPONS_086,                0,                 SOLOSTAGEINDEX_INFILTRATION,   DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // Small Characters
	{ CHEATCAT_FUN,             CHEAT_TEAMHEADSONLY,          L_MPWEAPONS_091,                0,                 SOLOSTAGEINDEX_AIRBASE,        DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // Team Heads Only
	{ CHEATCAT_FUN,             CHEAT_PLAYASELVIS,            L_MPWEAPONS_092,                TIME(7 m, 59 s),   SOLOSTAGEINDEX_RESCUE,         DIFF_PA,           CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_NONE }, }, // Play as Elvis
	{ CHEATCAT_FUN,             CHEAT_SLOMO,                  L_MPWEAPONS_081,                0,                 SOLOSTAGEINDEX_INVESTIGATION,  DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // Slo-mo Single Player
	{ CHEATCAT_GAMEPLAY,        CHEAT_INVINCIBLE,             L_MPWEAPONS_077,                TIME(3 m, 50 s),   SOLOSTAGEINDEX_ESCAPE,         DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_NONE }, }, // Invincible
	{ CHEATCAT_GAMEPLAY,        CHEAT_CLOAKINGDEVICE,         L_MPWEAPONS_076,                TIME(1 m, 40 s),   SOLOSTAGEINDEX_G5BUILDING,     DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED | CHEATFLAG_TRANSFERPAK,                       { CHEAT_NONE }, }, // Cloaking Device
	{ CHEATCAT_GAMEPLAY,        CHEAT_MARQUIS,                L_MPWEAPONS_095,                TIME(1 m, 30 s),   SOLOSTAGEINDEX_DEFECTION,      DIFF_SA,           CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_ENEMYROCKETS }, }, // Marquis of Queensbury Rules
	{ CHEATCAT_GAMEPLAY,        CHEAT_JOSHIELD,               L_MPWEAPONS_088,                0,                 SOLOSTAGEINDEX_DEEPSEA,        DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // Jo Shield
	{ CHEATCAT_GAMEPLAY,        CHEAT_SUPERSHIELD,            L_MPWEAPONS_089,                TIME(1 m, 45 s),   SOLOSTAGEINDEX_DEFENSE,        DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_NONE }, }, // Super Shield
	{ CHEATCAT_GAMEPLAY,        CHEAT_ENEMYSHIELDS,           L_MPWEAPONS_087,                0,                 SOLOSTAGEINDEX_DEFENSE,        DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // Enemy Shields
	{ CHEATCAT_GAMEPLAY,        CHEAT_ENEMYROCKETS,           L_MPWEAPONS_093,                0,                 SOLOSTAGEINDEX_PELAGIC,        DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_MARQUIS }, }, // Enemy Rockets
	{ CHEATCAT_GAMEPLAY,        CHEAT_PERFECTDARKNESS,        L_MPWEAPONS_096,                0,                 SOLOSTAGEINDEX_CRASHSITE,      DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // Perfect Darkness
	{ CHEATCAT_WEAPONSFORJO,    CHEAT_ROCKETLAUNCHER,         L_MPWEAPONS_027,                0,                 SOLOSTAGEINDEX_EXTRACTION,     DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // Rocket Launcher
	{ CHEATCAT_WEAPONSFORJO,    CHEAT_SNIPERRIFLE,            L_MPWEAPONS_103,                0,                 SOLOSTAGEINDEX_VILLA,          DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // Sniper Rifle
	{ CHEATCAT_WEAPONSFORJO,    CHEAT_SUPERDRAGON,            L_MPWEAPONS_105,                0,                 SOLOSTAGEINDEX_ESCAPE,         DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // SuperDragon
	{ CHEATCAT_WEAPONSFORJO,    CHEAT_LAPTOPGUN,              L_MPWEAPONS_106,                0,                 SOLOSTAGEINDEX_AIRFORCEONE,    DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // Laptop Gun
	{ CHEATCAT_WEAPONSFORJO,    CHEAT_PHOENIX,                L_MPWEAPONS_107,                0,                 SOLOSTAGEINDEX_ATTACKSHIP,     DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION,                                          { CHEAT_NONE }, }, // Phoenix
	{ CHEATCAT_WEAPONSFORJO,    CHEAT_PSYCHOSISGUN,           L_MPWEAPONS_108,                TIME(2 m,  0 s),   SOLOSTAGEINDEX_CHICAGO,        DIFF_PA,           CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_NONE }, }, // Psychosis Gun
	{ CHEATCAT_WEAPONSFORJO,    CHEAT_TRENTSMAGNUM,           L_MPWEAPONS_083,                TIME(2 m, 50 s),   SOLOSTAGEINDEX_CRASHSITE,      DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_NONE }, }, // Trent's Magnum
	{ CHEATCAT_WEAPONSFORJO,    CHEAT_FARSIGHT,               L_MPWEAPONS_084,                TIME(7 m, 27 s),   SOLOSTAGEINDEX_DEEPSEA,        DIFF_PA,           CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_NONE }, }, // FarSight
	{ CHEATCAT_CLASSICWEAPONS,  CHEAT_PP9I,                   L_MPWEAPONS_109,                0,                 0,                             0,                 CHEATFLAG_INVALIDATING | CHEATFLAG_FIRINGRANGE,                                         { CHEAT_NONE }, { { WEAPON_FALCON2,      FRDIFFICULTY_GOLD, }, { WEAPON_FALCON2_SCOPE, FRDIFFICULTY_GOLD, }, { WEAPON_FALCON2_SILENCER, FRDIFFICULTY_GOLD, },                                                                                         }, }, // PP9i
	{ CHEATCAT_CLASSICWEAPONS,  CHEAT_CC13,                   L_MPWEAPONS_110,                0,                 0,                             0,                 CHEATFLAG_INVALIDATING | CHEATFLAG_FIRINGRANGE,                                         { CHEAT_NONE }, { { WEAPON_MAGSEC4,      FRDIFFICULTY_GOLD, }, { WEAPON_MAULER,        FRDIFFICULTY_GOLD, }, { WEAPON_PHOENIX,          FRDIFFICULTY_GOLD, }, { WEAPON_DY357MAGNUM, FRDIFFICULTY_GOLD, }, { WEAPON_DY357LX,     FRDIFFICULTY_GOLD, }, }, }, // CC13
	{ CHEATCAT_CLASSICWEAPONS,  CHEAT_KL01313,                L_MPWEAPONS_111,                0,                 0,                             0,                 CHEATFLAG_INVALIDATING | CHEATFLAG_FIRINGRANGE,                                         { CHEAT_NONE }, { { WEAPON_CMP150,       FRDIFFICULTY_GOLD, }, { WEAPON_CYCLONE,       FRDIFFICULTY_GOLD, }, { WEAPON_CALLISTO,         FRDIFFICULTY_GOLD, }, { WEAPON_RCP120,      FRDIFFICULTY_GOLD, },                                             }, }, // KL01313
	{ CHEATCAT_CLASSICWEAPONS,  CHEAT_KF7SPECIAL,             L_MPWEAPONS_112,                0,                 0,                             0,                 CHEATFLAG_INVALIDATING | CHEATFLAG_FIRINGRANGE,                                         { CHEAT_NONE }, { { WEAPON_LAPTOPGUN,    FRDIFFICULTY_GOLD, }, { WEAPON_DRAGON,        FRDIFFICULTY_GOLD, }, { WEAPON_K7AVENGER,        FRDIFFICULTY_GOLD, }, { WEAPON_AR34,        FRDIFFICULTY_GOLD, }, { WEAPON_SUPERDRAGON, FRDIFFICULTY_GOLD, }, }, }, // KF7 Special
	{ CHEATCAT_CLASSICWEAPONS,  CHEAT_ZZT,                    L_MPWEAPONS_113,                0,                 0,                             0,                 CHEATFLAG_INVALIDATING | CHEATFLAG_FIRINGRANGE,                                         { CHEAT_NONE }, { { WEAPON_SHOTGUN,      FRDIFFICULTY_GOLD, }, { WEAPON_SNIPERRIFLE,   FRDIFFICULTY_GOLD, }, { WEAPON_ROCKETLAUNCHER,   FRDIFFICULTY_GOLD, }, { WEAPON_SLAYER,      FRDIFFICULTY_GOLD, },                                             }, }, // ZZT (9mm)
	{ CHEATCAT_CLASSICWEAPONS,  CHEAT_DMC,                    L_MPWEAPONS_114,                0,                 0,                             0,                 CHEATFLAG_INVALIDATING | CHEATFLAG_FIRINGRANGE,                                         { CHEAT_NONE }, { { WEAPON_TIMEDMINE,    FRDIFFICULTY_GOLD, }, { WEAPON_PROXIMITYMINE, FRDIFFICULTY_GOLD, }, { WEAPON_REMOTEMINE,       FRDIFFICULTY_GOLD, },                                                                                         }, }, // DMC
	{ CHEATCAT_CLASSICWEAPONS,  CHEAT_AR53,                   L_MPWEAPONS_115,                0,                 0,                             0,                 CHEATFLAG_INVALIDATING | CHEATFLAG_FIRINGRANGE,                                         { CHEAT_NONE }, { { WEAPON_FARSIGHT,     FRDIFFICULTY_GOLD, }, { WEAPON_CROSSBOW,      FRDIFFICULTY_GOLD, }, { WEAPON_COMBATKNIFE,      FRDIFFICULTY_GOLD, }, { WEAPON_GRENADE,     FRDIFFICULTY_GOLD, },                                             }, }, // AR53
	{ CHEATCAT_CLASSICWEAPONS,  CHEAT_RCP45,                  L_MPWEAPONS_116,                0,                 0,                             0,                 CHEATFLAG_INVALIDATING | CHEATFLAG_FIRINGRANGE,                                         { CHEAT_NONE }, { { WEAPON_TRANQUILIZER, FRDIFFICULTY_GOLD, }, { WEAPON_REAPER,        FRDIFFICULTY_GOLD, }, { WEAPON_DEVASTATOR,       FRDIFFICULTY_GOLD, },                                                                                         }, }, // RC-P45
	{ CHEATCAT_WEAPONS,         CHEAT_CLASSICSIGHT,           L_MPWEAPONS_090,                0,                 SOLOSTAGEINDEX_DEFECTION,      DIFF_A,            CHEATFLAG_COMPLETION,                                                                   { CHEAT_NONE }, }, // Classic Sight
	{ CHEATCAT_WEAPONS,         CHEAT_UNLIMITEDAMMOLAPTOP,    L_MPWEAPONS_094,                TIME(3 m, 55 s),   SOLOSTAGEINDEX_AIRFORCEONE,    DIFF_PA,           CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_NONE }, }, // Unlimited Ammo - Laptop Sentry Gun
	{ CHEATCAT_WEAPONS,         CHEAT_HURRICANEFISTS,         L_MPWEAPONS_075,                TIME(2 m,  3 s),   SOLOSTAGEINDEX_EXTRACTION,     DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED | CHEATFLAG_TRANSFERPAK,                       { CHEAT_NONE }, }, // Hurricane Fists
	{ CHEATCAT_WEAPONS,         CHEAT_UNLIMITEDAMMO,          L_MPWEAPONS_079,                TIME(7 m,  7 s),   SOLOSTAGEINDEX_PELAGIC,        DIFF_SA,           CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_NONE }, }, // Unlimited Ammo
	{ CHEATCAT_WEAPONS,         CHEAT_UNLIMITEDAMMONORELOADS, L_MPWEAPONS_080,                TIME(3 m, 11 s),   SOLOSTAGEINDEX_AIRBASE,        DIFF_SA,           CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_NONE }, }, // Unlimited Ammo, No Reloads
	{ CHEATCAT_WEAPONS,         CHEAT_XRAYSCANNER,            L_MPWEAPONS_104,                0,                 SOLOSTAGEINDEX_RESCUE,         DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION | CHEATFLAG_SOLOONLY,                     { CHEAT_NONE }, }, // X-Ray Scanner
	{ CHEATCAT_WEAPONS,         CHEAT_RTRACKER,               L_MPWEAPONS_101,                0,                 SOLOSTAGEINDEX_SKEDARRUINS,    DIFF_A,            CHEATFLAG_INVALIDATING | CHEATFLAG_COMPLETION | CHEATFLAG_TRANSFERPAK,                  { CHEAT_NONE }, }, // R-Tracker/Weapon Cache Locations
	{ CHEATCAT_WEAPONS,         CHEAT_ALLGUNS,                L_MPWEAPONS_078,                TIME(5 m, 31 s),   SOLOSTAGEINDEX_SKEDARRUINS,    DIFF_PA,           CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED | CHEATFLAG_TRANSFERPAK | CHEATFLAG_SOLOONLY,  { CHEAT_NONE }, }, // All Guns in Solo
    { CHEATCAT_BUDDIES,         CHEAT_VELVETDARK,             L_MPWEAPONS_117,                0,                 0,                             0,                 CHEATFLAG_ALWAYSAVAILABLE,                                                              { CHEAT_PUGILIST,   CHEAT_HOTSHOT,  CHEAT_HITANDRUN, CHEAT_ALIEN,     CHEAT_NONE }, }, // Velvet Dark
	{ CHEATCAT_BUDDIES,         CHEAT_PUGILIST,               L_MPWEAPONS_097,                TIME(6 m, 30 s),   SOLOSTAGEINDEX_INVESTIGATION,  DIFF_PA,           CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_VELVETDARK, CHEAT_HOTSHOT,  CHEAT_HITANDRUN, CHEAT_ALIEN,     CHEAT_NONE }, }, // Pugilist
	{ CHEATCAT_BUDDIES,         CHEAT_HOTSHOT,                L_MPWEAPONS_098,                TIME(5 m,  0 s),   SOLOSTAGEINDEX_INFILTRATION,   DIFF_SA,           CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_VELVETDARK, CHEAT_PUGILIST, CHEAT_HITANDRUN, CHEAT_ALIEN,     CHEAT_NONE }, }, // Hotshot
	{ CHEATCAT_BUDDIES,         CHEAT_HITANDRUN,              L_MPWEAPONS_099,                TIME(2 m, 30 s),   SOLOSTAGEINDEX_VILLA,          DIFF_SA,           CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_VELVETDARK, CHEAT_PUGILIST, CHEAT_HOTSHOT,   CHEAT_ALIEN,     CHEAT_NONE }, }, // Hit and Run
	{ CHEATCAT_BUDDIES,         CHEAT_ALIEN,                  L_MPWEAPONS_100,                TIME(5 m, 17 s),   SOLOSTAGEINDEX_ATTACKSHIP,     DIFF_SA,           CHEATFLAG_INVALIDATING | CHEATFLAG_TIMED,                                               { CHEAT_VELVETDARK, CHEAT_PUGILIST, CHEAT_HOTSHOT,   CHEAT_HITANDRUN, CHEAT_NONE }, }, // Alien
	{ CHEATCAT_DEBUG,           CHEAT_UNLOCKALLCONTENT,       L_MPWEAPONS_UNLOCKALLCONTENT,   0,                 0,                             0,                 CHEATFLAG_ALWAYSAVAILABLE,                                                              { CHEAT_NONE }, }, // Unlock All Content
};

u16 cheatcategorynames[NUM_CHEATCATS] = {
	L_MPWEAPONS_118,   // Fun
	L_MPWEAPONS_119,   // Gameplay
	L_MPWEAPONS_122,   // Weapons for Jo in Solo
	L_MPWEAPONS_123,   // Classic Weapons for Jo in Solo
	L_MPWEAPONS_120,   // Weapons
	L_MPWEAPONS_121,   // Buddies
	L_MPWEAPONS_DEBUG, // Debug
	L_MPWEAPONS_060,   // Disabled
};

u32 cheatIsUnlocked(s32 cheat_id)
{
	struct cheat *cheat;
	u32 unlocked = 0;

	if (cheat_id == CHEAT_NONE) {
		return true;
	}

	if (cheatIsActive(CHEAT_UNLOCKALLCONTENT)) {
		return true;
	}

	cheat = cheatGetFromCheatnum(cheat_id);

	if (cheat->flags & CHEATFLAG_ALWAYSAVAILABLE) {
		return true;
	} else if (cheat->flags & CHEATFLAG_FIRINGRANGE) {
		if (frIsCheatUnlocked(cheat)) {
			unlocked++;
		}
	} else if (cheat->flags & CHEATFLAG_COMPLETION) {
		if (g_GameFile.besttimes[cheat->stage_index][0]) {
			unlocked++;
		}
		if (g_GameFile.besttimes[cheat->stage_index][1]) {
			unlocked++;
		}
		if (g_GameFile.besttimes[cheat->stage_index][2]) {
			unlocked++;
		}
	} else {
		if (g_GameFile.besttimes[cheat->stage_index][cheat->difficulty] &&
				g_GameFile.besttimes[cheat->stage_index][cheat->difficulty] <= cheat->time) {
			unlocked++;
		}
	}

	if ((cheat->flags & CHEATFLAG_TRANSFERPAK) && gamefileHasFlag(GAMEFILEFLAG_USED_TRANSFERPAK)) {
		unlocked++;
	}

	return unlocked;
}

bool cheatIsActive(s32 cheat_id)
{
	struct cheat *cheat = cheatGetFromCheatnum(cheat_id);
	if (cheat) {
		return cheat->active;
	}

	return false;
}

void cheatActivate(s32 cheat_id)
{
	u32 prevplayernum;
	s32 playernum;
	s32 weaponnum;
	struct cheat *cheat = cheatGetFromCheatnum(cheat_id);
	bool isMulti = g_Vars.normmplayerisrunning == true;

	if ((isMulti && cheat->flags & CHEATFLAG_SOLOONLY) || (!isMulti && cheat->flags & CHEATFLAG_MULTIONLY)) {
		cheat->active = false;
		return;
	}

	switch (cheat_id) {
	case CHEAT_INVINCIBLE:
		// Make all players invincible
		prevplayernum = g_Vars.currentplayernum;

		for (playernum = 0; playernum < PLAYERCOUNT(); playernum++) {
			setCurrentPlayerNum(playernum);
			g_Vars.currentplayer->invincible = 1;
		}

		setCurrentPlayerNum(prevplayernum);
		break;
	case CHEAT_ALLGUNS:
		prevplayernum = g_Vars.currentplayernum;

		for (playernum = 0; playernum < PLAYERCOUNT(); playernum++) {
			setCurrentPlayerNum(playernum);
			invSetAllGuns(true);
		}

		setCurrentPlayerNum(prevplayernum);
		break;
	}

	cheatGetFromCheatnum(cheat_id)->active = true;
}

void cheatDeactivate(s32 cheat_id)
{
	u32 prevplayernum;
	s32 playernum;

	switch (cheat_id) {
	case CHEAT_INVINCIBLE:
		prevplayernum = g_Vars.currentplayernum;

		for (playernum = 0; playernum < PLAYERCOUNT(); playernum++) {
			setCurrentPlayerNum(playernum);
			g_Vars.currentplayer->invincible = false;
		}

		setCurrentPlayerNum(prevplayernum);
		break;
	case CHEAT_ALLGUNS:
		if (PLAYERCOUNT() == 1 && g_Vars.normmplayerisrunning == false) {
			prevplayernum = g_Vars.currentplayernum;

			for (playernum = 0; playernum < PLAYERCOUNT(); playernum++) {
				setCurrentPlayerNum(playernum);
				invSetAllGuns(false);
			}

			setCurrentPlayerNum(prevplayernum);
		}
		break;
	}

	cheatGetFromCheatnum(cheat_id)->active = false;
}

#define STARTWITHCHEATON(cheat_id) cheat = cheatGetFromCheatnum(cheat_id); cheat->enabled = true; cheat->active = true; cheatActivate(cheat_id);

void cheatsInit(void)
{
	struct cheat *cheat;

	s32 i;
	for (i = 0; i < NUM_CHEATS; i++) {
		g_Cheats[i].active = false;
		g_Cheats[i].enabled = false;
		g_Cheats[i].unlocked = cheatIsUnlocked(g_Cheats[i].cheat_id);
	}

	// TODO: Apply cheats from save file.
	STARTWITHCHEATON(CHEAT_UNLOCKALLCONTENT);
}

/**
 * Apply cheats at level startup.
 */
void cheatsReset(void)
{
	s32 i;
	for (i = 0; i < NUM_CHEATS; i++) {
		g_Cheats[i].unlocked = cheatIsUnlocked(g_Cheats[i].cheat_id);
	}

	// Copy enabled cheats to active cheats
	g_Cheats[i].active = g_Cheats[i].enabled;
	if (g_Cheats[i].active) {
		cheatActivate(g_Cheats[i].cheat_id);
	}

	// Set any "always on" cheats to active and properly activate all active cheats
	for (i = 0; i != NUM_CHEATS; i++) {
		if (g_Cheats[i].flags & CHEATFLAG_ALWAYSON) {
			g_Cheats[i].active = g_Cheats[i].unlocked;
		}

		if (g_Cheats[i].active) {
			cheatActivate(g_Cheats[i].cheat_id);
		}
	}
}

struct cheat *cheatGetNthInCategory(u8 n, u8 category)
{
	u32 i, j;
	for (i = 0, j = 0; i < ARRAYCOUNT(g_Cheats); i++) {
		if (g_Cheats[i].category == category) {
			if (j == n) {
				return &g_Cheats[i];
			} else {
				j++;
			}
		}
	}
	return NULL;
}

s32 cheatGetCountForCategory(s32 category)
{
	u32 i, count;
	for (i = 0, count = 0; i < ARRAYCOUNT(g_Cheats); i++) {
		if (g_Cheats[i].category == category) {
			count++;
		}
	}
	return count;
}

struct cheat *cheatGetFromCheatnum(u8 cheatnum)
{
	u32 i, j;
	for (i = 0; i < ARRAYCOUNT(g_Cheats); i++) {
		if (g_Cheats[i].cheat_id == cheatnum) {
			return &g_Cheats[i];
		}
	}
	return NULL;
}

s32 cheatCheckboxMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	struct cheat *cheat;
	s32 i;
	switch (operation) {
	case MENUOP_CHECKHIDDEN:
		return item->param >= g_CurCheatCategoryOptions;
	case MENUOP_GET:
		cheat = cheatGetNthInCategory(item->param, g_CurCheatCategory);
		if (cheat->cheat_id == CHEAT_VELVETDARK
				|| cheat->cheat_id == CHEAT_PUGILIST
				|| cheat->cheat_id == CHEAT_HOTSHOT
				|| cheat->cheat_id == CHEAT_HITANDRUN
				|| cheat->cheat_id == CHEAT_ALIEN) {
			return cheat->unlocked;
		}
		return cheat->enabled;
	case MENUOP_SET:
		cheat = cheatGetNthInCategory(item->param, g_CurCheatCategory);
		if (!cheatIsUnlocked(cheat->cheat_id)) {
			break;
		}

		if (cheat->enabled) {
			cheat->enabled = false;
			cheatDeactivate(cheat->cheat_id);
		} else {
			cheat->enabled = true;
			// Disable incompatible cheats.
			for (i = 0; i < ARRAYCOUNT(cheat->incompatiblecheats); i++) {
				if (cheat->incompatiblecheats[i] != CHEAT_NONE) {
					cheatGetFromCheatnum(cheat->incompatiblecheats[i])->enabled = false;
					cheatDeactivate(cheat->incompatiblecheats[i]);
				}
			}
			cheatActivate(cheat->cheat_id);
		}

		for (i = 0; i < NUM_CHEATS; i++) {
			if (g_Cheats[i].enabled) {
				cheatActivate(i);
			}
		}

		break;
	}

	return 0;
}

char *cheatGetNameIfUnlocked(struct menuitem *item)
{
	struct cheat *cheat;

	cheat = cheatGetNthInCategory(item->param, g_CurCheatCategory);
	if (cheat && cheatIsUnlocked(cheat->cheat_id)) {
		return langGet(cheat->nametextid);
	}

	return langGet(L_MPWEAPONS_074); // "----------"
}

s32 cheatMenuHandleDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	if (operation == MENUOP_OPEN) {
		func0f14a52c();

		if (gbpakIsAnyPerfectDark()) {
			gamefileSetFlag(GAMEFILEFLAG_USED_TRANSFERPAK);
		}

#if PIRACYCHECKS
		{
			u32 *ptr = (u32 *)&__scHandleTasks;
			u32 *end = (u32 *)&__scHandleRSP;
			u32 checksum = 0;

			while (ptr < end) {
				checksum ^= ~*ptr;
				ptr++;
			}

			if (checksum != CHECKSUM_PLACEHOLDER) {
				ptr = (u32 *)&__scHandleTasks + 20;
				if (1);
				end = &ptr[4];

				while (ptr < end) {
					*ptr = 0x00000012;
					ptr++;
				}
			}
		}
#endif
	}

	if (operation == MENUOP_CLOSE) {
		if (gbpakIsAnyPerfectDark()) {
			gamefileSetFlag(GAMEFILEFLAG_USED_TRANSFERPAK);
		}

		func0f14a560();
	}

	return 0;
}

struct menuitem g_CheatsWarningMenuItems[] = {
	{
		MENUITEMTYPE_LABEL,
		0,
		0,
		L_MPMENU_479, // "If you activate any cheats, then you will be unable to progress further in the game while those cheats are active."
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_480, // "OK"
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_481, // "Cancel"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CheatsWarningMenuDialog = {
	MENUDIALOGTYPE_SUCCESS,
	L_MPMENU_478, // "Warning"
	g_CheatsWarningMenuItems,
	NULL,
	0,
	NULL,
};

#define FastFoward(ptr) while (*ptr != '\n' && *ptr != '\0') { ptr++; }

/**
 * NTSC Beta uses g_StringPointer while newer versions use g_CheatMarqueeString.
 *
 * PAL Final introduces a string length check which ultimately doesn't do
 * anything.
 *
 * JPN final removes the colon characters from the format strings.
 */
char *cheatGetMarquee(struct menuitem *arg0)
{
	char *ptr;
	char difficultyname[256];
	char cheatname[256];
	char weaponname[256];

	if (g_Menus[g_MpPlayerNum].curdialog
			&& g_Menus[g_MpPlayerNum].curdialog->focuseditem
			&& g_Menus[g_MpPlayerNum].curdialog->focuseditem->type == MENUITEMTYPE_CHECKBOX) {
		struct cheat *cheat = cheatGetNthInCategory(g_Menus[g_MpPlayerNum].curdialog->focuseditem->param, g_CurCheatCategory);
		if (cheat) {
			if (cheatIsUnlocked(cheat->cheat_id)) {
				// Show cheat name
				sprintf(g_CheatMarqueeString, "%s: %s\n",
						g_CurCheatCategory == CHEATCAT_BUDDIES ? langGet(L_MPWEAPONS_143) : langGet(L_MPWEAPONS_136), // "Buddy available", "Cheat available"
						langGet(cheat->nametextid)
				);
			} else {
				// Locked
				strcpy(cheatname, langGet(cheat->nametextid));
				ptr = cheatname;

				FastFoward(ptr);

				*ptr = '\0';

				if (cheat->flags & CHEATFLAG_COMPLETION) {
					sprintf(g_CheatMarqueeString, "%s %s: %s %s %s",
							langGet(L_MPWEAPONS_137), // "Complete"
							langGet(g_StageNames[cheat->stage_index].name1),
							langGet(g_StageNames[cheat->stage_index].name2),
							langGet(L_MPWEAPONS_138), // "for cheat:"
							&cheatname
					);
				} else if (cheat->flags & CHEATFLAG_FIRINGRANGE) {
					// Firing Range (TODO: Make this language-dependent)
					char *strptr = g_CheatMarqueeString;
					s32 count;
					s32 i;

					for (count = 0; count < 5; count++) {
						if (cheat->frdata[count].weaponnum == 0 && cheat->frdata[count].difficulty == 0) {
							break;
						}
					}

					strcpy(strptr, langGet(L_MPWEAPONS_137)); // "Complete"
					FastFoward(strptr);
					*strptr = ' '; strptr++;

					for (i = 0; i < count; i++) {
						strcpy(strptr, langGet(L_PROPOBJ_008)); // "the "
						FastFoward(strptr);

						strcpy(strptr, langGet(g_FiringRangeDifficultyNames[cheat->frdata[i].difficulty]));
						FastFoward(strptr);

						*strptr = ' '; strptr++;

						strcpy(strptr, bgunGetName(cheat->frdata[i].weaponnum));
						FastFoward(strptr);

						if (i == count - 1) {
							*strptr = ' '; strptr++;

							if (count > 1) {
								strcpy(strptr, langGet(L_MPWEAPONS_FIRINGRANGECHALLENGES)); // "firing range challenges"
							} else {
								strcpy(strptr, langGet(L_MPWEAPONS_FIRINGRANGECHALLENGE)); // "firing range challenge"
							}
							FastFoward(strptr);
						}

						if (i == count - 2) {
							strcpy(strptr, langGet(L_MPWEAPONS_OXFORDAND)); // ", and "
							FastFoward(strptr);
						} else if (i < count - 2) {
							strcpy(strptr, langGet(L_MPWEAPONS_COMMA)); // ", "
							FastFoward(strptr);
						}
					}

					*strptr = ' '; strptr++;

					strcpy(strptr, langGet(L_MPWEAPONS_138)); // "for cheat:"
					FastFoward(strptr);

					*strptr = ' '; strptr++;

					strcpy(strptr, cheatname);
				} else {
					// Timed
					strcpy(difficultyname, langGet(L_OPTIONS_251 + cheat->difficulty));
					ptr = difficultyname;

					FastFoward(ptr);

					*ptr = '\0';

					sprintf(g_CheatMarqueeString, "%s %s: %s %s %s %s %d:%02d %s %s",
							langGet(L_MPWEAPONS_137), // "Complete"
							langGet(g_StageNames[cheat->stage_index].name1),
							langGet(g_StageNames[cheat->stage_index].name2),
							langGet(L_MPWEAPONS_139), // "on"
							&difficultyname,
							langGet(L_MPWEAPONS_140), // "in under"
							cheat->time / 60,
							cheat->time % 60,
							langGet(L_MPWEAPONS_138), // "for cheat:"
							&cheatname
					);
				}

				if (cheat->flags & CHEATFLAG_TRANSFERPAK) {
					strcat(g_CheatMarqueeString, langGet(L_MPWEAPONS_141)); // " or insert Game Boy ..."
				}

				strcat(g_CheatMarqueeString, "\n");
			}

			return g_CheatMarqueeString;
		}
	}

	// No cheat selected
	return langGet(L_MPWEAPONS_142); // "Select cheat for information"
}

s32 cheatMenuHandleTurnOffAllCheats(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 i;
		for (i = 0; i < NUM_CHEATS; i++) {
			g_Cheats[i].enabled = false;
			g_Cheats[i].active = false;
		}

		for (i = 0; i < NUM_CHEATS; i++) {
			g_Cheats[i].unlocked = cheatIsUnlocked(g_Cheats[i].cheat_id);
		}
	}

	return false;
}

#if VERSION >= VERSION_NTSC_1_0
s32 cheatGetByTimedStageIndex(s32 stage_index, s32 difficulty)
{
	s32 i;

	for (i = 0; i < NUM_CHEATS; i++) {
		if (g_Cheats[i].stage_index == stage_index &&
				g_Cheats[i].difficulty == difficulty &&
				(g_Cheats[i].flags & CHEATFLAG_COMPLETION) == 0 &&
				(g_Cheats[i].flags & CHEATFLAG_FIRINGRANGE) == 0) {
			return g_Cheats[i].cheat_id;
		}
	}

	return -1;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
s32 cheatGetByCompletedStageIndex(s32 stage_index)
{
	s32 i;

	for (i = 0; i < NUM_CHEATS; i++) {
		if (g_Cheats[i].stage_index == stage_index && (g_Cheats[i].flags & CHEATFLAG_COMPLETION)) {
			return g_Cheats[i].cheat_id;
		}
	}

	return -1;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
s32 cheatGetTime(s32 cheat_id)
{
	return cheatGetFromCheatnum(cheat_id)->time;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
char *cheatGetName(s32 cheat_id)
{
	return langGet(cheatGetFromCheatnum(cheat_id)->nametextid);
}
#endif

struct menuitem g_CheatsGenericMenuItems[] = {
	{
		MENUITEMTYPE_CHECKBOX,
		0x00000000,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0x00000001,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0x00000002,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0x00000003,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0x00000004,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0x00000005,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0x00000006,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0x00000007,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0x00000008,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0x00000009,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0x0000000a,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0x0000000b,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_CHECKBOX,
		0x0000000c,
		0,
		(u32)&cheatGetNameIfUnlocked,
		0,
		cheatCheckboxMenuHandler,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_MARQUEE,
		0,
		MENUITEMFLAG_SMALLFONT | MENUITEMFLAG_MARQUEE_FADEBOTHSIDES,
		(u32)&cheatGetMarquee,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_477, // "Done"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menuitem g_CheatsMenuItems[] = {
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_118, // "Fun"
		CHEATCAT_FUN,
		menuhandlerCheatCategory
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_119, // "Gameplay"
		CHEATCAT_GAMEPLAY,
		menuhandlerCheatCategory
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_122, // "Weapons for Jo in Solo"
		CHEATCAT_WEAPONSFORJO,
		menuhandlerCheatCategory,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_123, // "Classic Weapons for Jo in Solo"
		CHEATCAT_CLASSICWEAPONS,
		menuhandlerCheatCategory,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_120, // "Weapons"
		CHEATCAT_WEAPONS,
		menuhandlerCheatCategory,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_121, // "Buddies"
		CHEATCAT_BUDDIES,
		menuhandlerCheatCategory,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_DEBUG, // "Fun"
		CHEATCAT_DEBUG,
		menuhandlerCheatCategory,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		0,
		L_MPWEAPONS_217, // "Turn off all Cheats"
		0,
		cheatMenuHandleTurnOffAllCheats,
	},
	{
		MENUITEMTYPE_SEPARATOR,
		0,
		0,
		0x00000096,
		0,
		NULL,
	},
	{
		MENUITEMTYPE_SELECTABLE,
		0,
		MENUITEMFLAG_SELECTABLE_CLOSESDIALOG | MENUITEMFLAG_SELECTABLE_CENTRE,
		L_MPMENU_477, // "Done"
		0,
		NULL,
	},
	{ MENUITEMTYPE_END },
};

struct menudialogdef g_CheatsMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_476, // "Cheats"
	g_CheatsMenuItems,
	cheatMenuHandleDialog,
	MENUDIALOGFLAG_MPLOCKABLE,
	NULL,
};

struct menudialogdef g_CheatsGenericMenuDialog = {
	MENUDIALOGTYPE_DEFAULT,
	L_MPMENU_476, // "Cheats"
	g_CheatsGenericMenuItems,
	cheatMenuHandleDialog,
	0,
	NULL,
};

s32 menuhandlerCheatCategory(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 i, j;

	if (operation == MENUOP_SET) {
		g_CurCheatCategory = item->param3;
		for (i = 0, j = 0; i < ARRAYCOUNT(g_Cheats); i++) {
			if (g_Cheats[i].category == item->param3) {
				j++;
			}
		}
		g_CurCheatCategoryOptions = j;
		g_CheatsGenericMenuDialog.title = cheatcategorynames[g_CurCheatCategory];
		menuPushDialog(&g_CheatsGenericMenuDialog);
	}

	return 0;
}

s32 cheatAreInvalidatingCheatsActive()
{
	s32 i;
	for (i = 0; i < ARRAYCOUNT(g_Cheats); i++) {
		if (g_Cheats[i].active && g_Cheats[i].flags & CHEATFLAG_INVALIDATING) {
			return true;
		}
	}
	return false;
}

s32 cheatAreInvalidatingCheatsEnabled()
{
	s32 i;
	for (i = 0; i < ARRAYCOUNT(g_Cheats); i++) {
		if (g_Cheats[i].enabled && g_Cheats[i].flags & CHEATFLAG_INVALIDATING) {
			return true;
		}
	}
	return false;
}

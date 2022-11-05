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
#include "game/lang.h"
#include "game/pak.h"
#include "bss.h"
#include "data.h"
#include "types.h"

u32 cheatIsUnlocked(s32 cheat_id)
{
	struct cheat *cheat;
	u32 unlocked = 0;

	if (cheat_id == CHEAT_NONE) {
		return true;
	}

	if (cheatIsInvalid(cheat_id)) {
		return false;
	}

	if (cheatIsAllContentUnlocked()) {
		return true;
	}

	cheat = &g_Cheats[cheat_id];

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
	if (cheat_id == CHEAT_NONE) {
		return true;
	}

	if (cheatIsInvalid(cheat_id)) {
		return false;
	}

	return g_Cheats[cheat_id].active;
}

void cheatActivate(s32 cheat_id)
{
	u32 prevplayernum;
	s32 playernum;
	bool isMulti = g_Vars.normmplayerisrunning == true;

	if (cheat_id == CHEAT_NONE) {
		return;
	}

	if (cheatIsInvalid(cheat_id)) {
		return;
	}

	if ((isMulti && g_Cheats[cheat_id].flags & CHEATFLAG_SOLOONLY) || (!isMulti && g_Cheats[cheat_id].flags & CHEATFLAG_MULTIONLY)) {
		g_Cheats[cheat_id].active = false;
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

	g_Cheats[cheat_id].active = true;
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

	g_Cheats[cheat_id].active = false;
}

void cheatsInit(void)
{
	struct cheat *cheat;

	s32 cheat_id;
	for (cheat_id = 0; cheat_id < NUM_CHEATS; cheat_id++) {
		g_Cheats[cheat_id].active = false;
		g_Cheats[cheat_id].enabled = false;
		g_Cheats[cheat_id].unlocked = cheatIsUnlocked(cheat_id);
	}

	// TODO: Apply cheats from save file.
	g_Cheats[CHEAT_UNLOCKALLCONTENT].enabled = true;
	g_Cheats[CHEAT_UNLOCKALLCONTENT].active = true;
	cheatActivate(CHEAT_UNLOCKALLCONTENT);
}

/**
 * Apply cheats at level startup.
 */
void cheatsReset(void)
{
	s32 cheat_id;
	for (cheat_id = 0; cheat_id < NUM_CHEATS; cheat_id++) {
		g_Cheats[cheat_id].unlocked = cheatIsUnlocked(cheat_id);
	}

	// Copy enabled cheats to active cheats
	g_Cheats[cheat_id].active = g_Cheats[cheat_id].enabled;
	if (g_Cheats[cheat_id].active) {
		cheatActivate(cheat_id);
	}

	// Set any "always on" cheats to active and properly activate all active cheats
	for (cheat_id = 0; cheat_id != NUM_CHEATS; cheat_id++) {
		if (g_Cheats[cheat_id].flags & CHEATFLAG_ALWAYSON) {
			g_Cheats[cheat_id].active = g_Cheats[cheat_id].unlocked;
		}

		if (g_Cheats[cheat_id].active) {
			cheatActivate(cheat_id);
		}
	}
}

s32 cheatCheckboxMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data)
{
	s32 i;
	switch (operation) {
	case MENUOP_GET:
		/*
		if (g_Cheats[item->param].flags & CHEATFLAG_BUDDY) {
			return g_Cheats[item->param].unlocked;
		}
		*/
		if (item->param == CHEAT_VELVETDARK) {
			return !g_Cheats[CHEAT_PUGILIST].enabled
				&& !g_Cheats[CHEAT_HOTSHOT].enabled
				&& !g_Cheats[CHEAT_HITANDRUN].enabled
				&& !g_Cheats[CHEAT_ALIEN].enabled;
		}
		return g_Cheats[item->param].enabled;
	case MENUOP_SET:
		if (!cheatIsUnlocked(item->param)) {
			break;
		}

		if (g_Cheats[item->param].enabled) {
			g_Cheats[item->param].enabled = false;
			cheatDeactivate(item->param);
		} else {
			g_Cheats[item->param].enabled = true;
			// Disable incompatible cheats.
			for (i = 0; i < ARRAYCOUNT(g_Cheats[item->param].incompatiblecheats); i++) {
				if (g_Cheats[item->param].incompatiblecheats[i] != CHEAT_NONE) {
					g_Cheats[g_Cheats[item->param].incompatiblecheats[i]].enabled = false;
					cheatDeactivate(g_Cheats[item->param].incompatiblecheats[i]);
				}
			}
			cheatActivate(item->param);
		}

		for (i = 0; i < NUM_CHEATS; i++) {
			if (g_Cheats[i].enabled) {
				cheatActivate(item->param);
			}
		}

		break;
	}

	return 0;
}

char *cheatGetNameIfUnlocked(struct menuitem *item)
{
	if (cheatIsUnlocked(item->param)) {
		return langGet(g_Cheats[item->param].nametextid);
	}

	return langGet(L_MPWEAPONS_074); // "----------"
}

s32 cheatMenuHandleDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data)
{
	switch (operation) {
	case MENUOP_OPEN:
	case MENUOP_CLOSE:
		if (gbpakIsAnyPerfectDark()) {
			gamefileSetFlag(GAMEFILEFLAG_USED_TRANSFERPAK);
		}
		break;
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
		u32 cheat_id = g_Menus[g_MpPlayerNum].curdialog->focuseditem->param;
		struct cheat *cheat = &g_Cheats[cheat_id];

		if (cheatIsUnlocked(cheat_id)) {
			// Show cheat name
			sprintf(g_CheatMarqueeString, "%s: %s\n",
					(cheat->flags & CHEATFLAG_BUDDY) ? langGet(L_MPWEAPONS_143) : langGet(L_MPWEAPONS_136), // "Buddy available", "Cheat available"
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
						langGet(g_StageNames[g_Cheats[cheat_id].stage_index].name1),
						langGet(g_StageNames[g_Cheats[cheat_id].stage_index].name2),
						langGet(L_MPWEAPONS_139), // "on"
						&difficultyname,
						langGet(L_MPWEAPONS_140), // "in under"
						g_Cheats[cheat_id].time / 60,
						g_Cheats[cheat_id].time % 60,
						langGet(L_MPWEAPONS_138), // "for cheat:"
						&cheatname
				);
			}

			if (g_Cheats[cheat_id].flags & CHEATFLAG_TRANSFERPAK) {
				strcat(g_CheatMarqueeString, langGet(L_MPWEAPONS_141)); // " or insert Game Boy ..."
			}

			strcat(g_CheatMarqueeString, "\n");
		}

		return g_CheatMarqueeString;
	}

	// No cheat selected
	return langGet(L_MPWEAPONS_142); // "Select cheat for information"
}

s32 cheatMenuHandleTurnOffAllCheats(s32 operation, struct menuitem *item, union handlerdata *data)
{
	if (operation == MENUOP_SET) {
		s32 cheat_id;
		for (cheat_id = 0; cheat_id < NUM_CHEATS; cheat_id++) {
			g_Cheats[cheat_id].enabled = false;
			g_Cheats[cheat_id].active = false;
		}

		for (cheat_id = 0; cheat_id < NUM_CHEATS; cheat_id++) {
			g_Cheats[cheat_id].unlocked = cheatIsUnlocked(cheat_id);
		}
	}

	return false;
}

#if VERSION >= VERSION_NTSC_1_0
s32 cheatGetByTimedStageIndex(s32 stage_index, s32 difficulty)
{
	s32 cheat_id;

	for (cheat_id = 0; cheat_id < NUM_CHEATS; cheat_id++) {
		if (g_Cheats[cheat_id].stage_index == stage_index &&
				g_Cheats[cheat_id].difficulty == difficulty &&
				(g_Cheats[cheat_id].flags & CHEATFLAG_COMPLETION) == 0 &&
				(g_Cheats[cheat_id].flags & CHEATFLAG_FIRINGRANGE) == 0) {
			return cheat_id;
		}
	}

	return -1;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
s32 cheatGetByCompletedStageIndex(s32 stage_index)
{
	s32 cheat_id;

	for (cheat_id = 0; cheat_id < NUM_CHEATS; cheat_id++) {
		if (g_Cheats[cheat_id].stage_index == stage_index && (g_Cheats[cheat_id].flags & CHEATFLAG_COMPLETION)) {
			return cheat_id;
		}
	}

	return -1;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
s32 cheatGetTime(s32 cheat_id)
{
	return g_Cheats[cheat_id].time;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
char *cheatGetName(s32 cheat_id)
{
	return langGet(g_Cheats[cheat_id].nametextid);
}
#endif

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

s32 cheatAreSoloInvalidatingCheatsEnabled()
{
	s32 i;
	for (i = 0; i < ARRAYCOUNT(g_Cheats); i++) {
		if (g_Cheats[i].enabled && g_Cheats[i].flags & CHEATFLAG_INVALIDATING && !(g_Cheats[i].flags & CHEATFLAG_MULTIONLY)) {
			return true;
		}
	}
	return false;
}

s32 cheatAreMultiInvalidatingCheatsEnabled()
{
	s32 i;
	for (i = 0; i < ARRAYCOUNT(g_Cheats); i++) {
		if (g_Cheats[i].enabled && g_Cheats[i].flags & CHEATFLAG_INVALIDATING && !(g_Cheats[i].flags & CHEATFLAG_SOLOONLY)) {
			return true;
		}
	}
	return false;
}

#ifndef _IN_GAME_MPLAYER_SCENARIOS_H
#define _IN_GAME_MPLAYER_SCENARIOS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

extern struct menudialogdef g_MpScenarioMenuDialog;
extern struct menudialogdef g_MpQuickTeamScenarioMenuDialog;

/**
 * Each scenario registers callback functions for certain events. For example,
 * code elsewhere in the game may call scenarioTick, then scenarioTick checks
 * if the current scenario has a tick callback defined (eg. htbTick). If so,
 * that scenario's callback is run.
 */
struct mpscenario {
	u16 name;
	u16 shortname;
	u8 requirefeature;
	u8 teamonly;
	struct menudialogdef *optionsdialog;
	void (*initfunc)(void);
	s32 (*numpropsfunc)(void);
	void (*initpropsfunc)(void);
	void (*tickfunc)(void);
	void (*tickchrfunc)(struct chrdata *chr);
	Gfx *(*hudfunc)(Gfx *gdl);
	void (*calcscorefunc)(struct mpchrconfig *mpchr, s32 chrnum, s32 *score, s32 *deaths);
	Gfx *(*radarextrafunc)(Gfx *gdl);
	bool (*radarchrfunc)(Gfx **gdl, struct prop *prop);
	bool (*highlightpropfunc)(struct prop *prop, s32 *colour);
	bool (*spawnfunc)(f32 arg0, struct coord *pos, s16 *rooms, struct prop *prop, f32 *arg4);
	s32 (*maxteamsfunc)(void);
	bool (*isroomhighlightedfunc)(s16 room);
	void (*highlightroomfunc)(s16 room, s32 *arg1, s32 *arg2, s32 *arg3);
	void *unk3c; // never hooked into nor fired
	void (*readsavefunc)(struct savebuffer *buffer);
	void (*writesavefunc)(struct savebuffer *buffer);
	void (*handledeathfunc)(s32 aplayernum, s32 vplayernum, struct gset gset);
};

extern struct mpscenario g_MpScenarios[NUM_MPSCENARIOS];

char *scenarioRemoveLineBreaks(char *src, s32 stringnum);
s32 menuhandlerMpOpenOptions(s32 operation, struct menuitem *item, union handlerdata *data);
void scenarioReadSave(struct savebuffer *buffer);
void scenarioWriteSave(struct savebuffer *buffer);
void scenarioInit(void);
s32 scenarioNumProps(void);
void scenarioInitProps(void);
void scenarioTick(void);
void scenarioTickChr(struct chrdata *chr);
Gfx *scenarioRadarExtra(Gfx *gdl);
bool scenarioRadarChr(Gfx **gdl, struct prop *prop);
f32 scenarioChooseSpawnLocation(f32 chrradius, struct coord *pos, s16 *rooms, struct prop *prop);
s32 scenarioGetMaxTeams(void);
void scenarioHighlightRoom(s16 room, s32 *arg1, s32 *arg2, s32 *arg3);

#endif

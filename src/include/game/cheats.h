#ifndef IN_GAME_CHEATS_H
#define IN_GAME_CHEATS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

#define cheatIsAllContentUnlocked()	(g_Cheats[CHEAT_UNLOCKALLCONTENT].active)
#define cheatIsInvalid(cheat_id)	(cheat_id < 0 || cheat_id >= NUM_CHEATS)

extern struct cheat g_Cheats[NUM_CHEATS];

u32 cheatIsUnlocked(s32 cheat_id);
bool cheatIsActive(s32 cheat_id);
void cheatActivate(s32 cheat_id);
void cheatDeactivate(s32 cheat_id);
void cheatsInit(void);
void cheatsReset(void);
char *cheatGetNameIfUnlocked(struct menuitem *item);
char *cheatGetMarquee(struct menuitem *item);
s32 cheatGetByTimedStageIndex(s32 stage_index, s32 difficulty);
s32 cheatGetByCompletedStageIndex(s32 stage_index);
s32 cheatGetTime(s32 cheat_id);
char *cheatGetName(s32 cheat_id);
s32 cheatMenuHandleDialog(s32 operation, struct menudialogdef *dialogdef, union handlerdata *data);
s32 cheatCheckboxMenuHandler(s32 operation, struct menuitem *item, union handlerdata *data);
s32 cheatMenuHandleTurnOffAllCheats(s32 operation, struct menuitem *item, union handlerdata *data);
s32 cheatAreInvalidatingCheatsActive();
s32 cheatAreSoloInvalidatingCheatsEnabled();
s32 cheatAreMultiInvalidatingCheatsEnabled();

#endif

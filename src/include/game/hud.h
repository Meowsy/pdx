#ifndef IN_GAME_HUD_H
#define IN_GAME_HUD_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

char *hudGetPrettyCommand(struct aibot *aibot, s32 stringnum);
char* hudGetPropName(struct prop *prop);
Gfx *hudRenderOrders(Gfx *gdl);
Gfx *hudRenderBotDebug(Gfx *gdl);
Gfx *hudRenderTeamWeapons(Gfx *gdl);
Gfx *hudRenderRoomDebug(Gfx *gdl);
Gfx *hudRenderPlayerSelected(Gfx *gdl);

#endif

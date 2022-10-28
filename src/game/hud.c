#include <ultra64.h>
#include "constants.h"
#include "game/chraction.h"
#include "game/dlights.h"
#include "game/game_006900.h"
#include "game/setup.h"
#include "game/prop.h"
#include "game/propsnd.h"
#include "game/atan2f.h"
#include "game/bondgun.h"
#include "game/player.h"
#include "game/savebuffer.h"
#include "game/hudmsg.h"
#include "game/menu.h"
#include "game/inv.h"
#include "game/playermgr.h"
#include "game/game_1531a0.h"
#include "game/mplayer/setup.h"
#include "game/mplayer/scenarios.h"
#include "game/radar.h"
#include "game/bot.h"
#include "game/botcmd.h"
#include "game/botinv.h"
#include "game/challenge.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/pad.h"
#include "game/propobj.h"
#include "game/cheats.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/snd.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/str.h"
//#include "lib/mtx.h"
//#include "lib/anim.h"
//#include "lib/collision.h"
#include "data.h"
#include "types.h"

char *hudGetPrettyCommand(struct aibot *aibot, s32 stringnum)
{
	static char strings[2][64];
	char *out = strings[stringnum];
	s32 i;

	switch (aibot->command) {
	case AIBOTCMD_FOLLOW:
	case AIBOTCMD_PROTECT:
		sprintf(strings[stringnum], "%s %s", scenarioRemoveLineBreaks(botGetCommandName(aibot->command), 0), g_MpAllChrConfigPtrs[mpPlayerGetIndex((g_Vars.props + aibot->followprotectpropnum)->chr)]->name);
		break;
	case AIBOTCMD_ATTACK:
		sprintf(strings[stringnum], "%s %s", scenarioRemoveLineBreaks(botGetCommandName(aibot->command), 0), g_MpAllChrConfigPtrs[mpPlayerGetIndex((g_Vars.props + aibot->attackpropnum)->chr)]->name);
		break;
	case AIBOTCMD_POPCAP: {
		struct prop *victimprop = g_MpAllChrPtrs[g_ScenarioData.pac.victims[g_ScenarioData.pac.victimindex]]->prop;
		struct chrdata *victimchr = victimprop->chr;

		if (victimprop == g_MpAllChrPtrs[i]->prop) {
			sprintf(strings[stringnum], "%s: %s", scenarioRemoveLineBreaks(botGetCommandName(aibot->command), 0), botGetCommandName(AIBOTCMD_NORMAL));
			break;
		}

		if ((g_MpSetup.options & MPOPTION_TEAMSENABLED) && victimchr->team == g_MpAllChrPtrs[i]->team) {
			if (botCanFollow(g_MpAllChrPtrs[i], victimchr)) {
				if (g_MpAllChrPtrs[i]->aibot->canbreakfollow) {
					sprintf(strings[stringnum], "%s: %s %s", scenarioRemoveLineBreaks(botGetCommandName(aibot->command), 0), scenarioRemoveLineBreaks(botGetCommandName(AIBOTCMD_FOLLOW), 1), g_MpAllChrConfigPtrs[mpPlayerGetIndex(victimchr)]->name);
					break;
				} else {
					sprintf(strings[stringnum], "%s: %s %s", scenarioRemoveLineBreaks(botGetCommandName(aibot->command), 0), scenarioRemoveLineBreaks(botGetCommandName(AIBOTCMD_PROTECT), 1), g_MpAllChrConfigPtrs[mpPlayerGetIndex(victimchr)]->name);
					break;
				}
			}
		} else if (!botIsTargetInvisible(g_MpAllChrPtrs[i], victimchr) && botPassesCowardCheck(g_MpAllChrPtrs[i], victimchr)) {
			sprintf(strings[stringnum], "%s %s", scenarioRemoveLineBreaks(botGetCommandName(AIBOTCMD_ATTACK), 0), g_MpAllChrConfigPtrs[mpPlayerGetIndex(victimchr)]->name);
			break;
		}
	}
	default:
		sprintf(strings[stringnum], "%s", scenarioRemoveLineBreaks(botGetCommandName(aibot->command), 0));
		break;
	}

	return strings[stringnum];
}

char* hudGetPropName(struct prop *prop)
{
	static char string[64];

	if (!prop) {
		return langGet(L_MPWEAPONS_058); // "Nothing"
	}

	switch (prop->type) {
	case PROPTYPE_WEAPON:					return bgunGetName(prop->weapon->weaponnum);
	case PROPTYPE_OBJ:
		switch (prop->obj->type) {
		case OBJTYPE_DOOR:					return "Door\n";
		case OBJTYPE_DOORSCALE:				return "Door Scale\n";
		case OBJTYPE_BASIC:					return "Basic\n";
		case OBJTYPE_KEY:					return "Key\n";
		case OBJTYPE_ALARM:					return "Alarm\n";
		case OBJTYPE_CCTV:					return "Camera\n";
		case OBJTYPE_AMMOCRATE: {
			struct ammocrateobj *crate = (struct ammocrateobj *) prop->obj;
			string[0] = '\0';
			ammotypeGetPickupName(string, crate->ammotype, ammocrateGetPickupAmmoQty(crate));
			strcat(string, "\n");
			return string;
		}
		case OBJTYPE_WEAPON:				return bgunGetName(prop->weapon->weaponnum);
		case OBJTYPE_CHR:					return "Character\n";
		case OBJTYPE_SINGLEMONITOR:			return "Monitor\n";
		case OBJTYPE_MULTIMONITOR:			return "Multi-Monitor\n";
		case OBJTYPE_HANGINGMONITORS:		return "Hanging Monitors\n";
		case OBJTYPE_AUTOGUN:				return "Autogun\n";
		case OBJTYPE_LINKGUNS:				return "Link Guns\n";
		case OBJTYPE_DEBRIS:				return "Debris\n";
		case OBJTYPE_10:					return "ObjType 10\n";
		case OBJTYPE_HAT:					return "Hat\n";
		case OBJTYPE_GRENADEPROB:			return "Grenade Prob.\n";
		case OBJTYPE_LINKLIFTDOOR:			return "Link Lift & Door\n";
		case OBJTYPE_MULTIAMMOCRATE:		return "Ammo Crate\n";
		case OBJTYPE_SHIELD:				return langGet(L_GUN_240); // "Shield"
		case OBJTYPE_TAG:					return "Tag\n";
		case OBJTYPE_BEGINOBJECTIVE:		return "Begin Objective\n";
		case OBJTYPE_ENDOBJECTIVE:			return "End Objective\n";
		case OBJECTIVETYPE_DESTROYOBJ:		return "Objective: Destroy Object\n";
		case OBJECTIVETYPE_COMPFLAGS:		return "Objective: Complete Flags\n";
		case OBJECTIVETYPE_FAILFLAGS:		return "Objective: Fail Flags\n";
		case OBJECTIVETYPE_COLLECTOBJ:		return "Objective: Collect Object\n";
		case OBJECTIVETYPE_THROWOBJ:		return "Objective: Throw Object\n";
		case OBJECTIVETYPE_HOLOGRAPH:		return "Objective: Holograph\n";
		case OBJECTIVETYPE_1F:				return "Objective: Type 1F\n";
		case OBJECTIVETYPE_ENTERROOM:		return "Objective: Enter Room\n";
		case OBJECTIVETYPE_THROWINROOM:		return "Objective: Throw in Room\n";
		case OBJTYPE_22:					return "Objective: Type 22\n";
		case OBJTYPE_BRIEFING:				return "Briefing\n";
		case OBJTYPE_GASBOTTLE:				return "Gas Bottle\n";
		case OBJTYPE_RENAMEOBJ:				return "Rename Object\n";
		case OBJTYPE_PADLOCKEDDOOR:			return "Pad-Locked Door\n";
		case OBJTYPE_TRUCK:					return "Truck\n";
		case OBJTYPE_HELI:					return "Helicopter\n";
		case OBJTYPE_29:					return "ObjType 29\n";
		case OBJTYPE_GLASS:					return "Glass\n";
		case OBJTYPE_SAFE:					return "Safe\n";
		case OBJTYPE_SAFEITEM:				return "Safe Item\n";
		case OBJTYPE_TANK:					return "Tank\n";
		case OBJTYPE_CAMERAPOS:				return "Camera Position\n";
		case OBJTYPE_TINTEDGLASS:			return "Tinted Glass\n";
		case OBJTYPE_LIFT:					return "Lift\n";
		case OBJTYPE_CONDITIONALSCENERY:	return "Conditional Scenery\n";
		case OBJTYPE_BLOCKEDPATH:			return "Blocked Path\n";
		case OBJTYPE_HOVERBIKE:				return langGet(L_MISC_HOVERBIKE); // "HoverBike"
		case OBJTYPE_END:					return "End\n";
		case OBJTYPE_HOVERPROP:
			switch (prop->obj->modelnum) {
			case MODEL_HOVERBED:
			case MODEL_A51TROLLEY:
				return "Hoverbed";
			case MODEL_A51_CRATE2:
			case MODEL_HOVERCRATE1:
			default:
				return langGet(L_MISC_HOVERCRATE); // "HoverCrate"
			}
		case OBJTYPE_FAN:					return "Fan\n";
		case OBJTYPE_HOVERCAR:
			switch (prop->obj->modelnum) {
			case MODEL_DD_HOVCOP:
			case MODEL_DD_HOVCAR:
			case MODEL_DD_HOVTRUCK:
				return "Hovercar\n";
			case MODEL_DD_HOVMOTO:
				return langGet(L_MISC_HOVERBIKE); // "HoverBike"
			case MODEL_DD_HOVCAB:
			case MODEL_TAXICAB:
				return "Hovertaxi\n";
			case MODEL_LIMO:
				return "Hoverlimo\n";
			case MODEL_TESTERBOT:
			case MODEL_HOOVERBOT:
			default:
				return langGet(L_MISC_CLEANINGHOVBOT); // "Cleaning Hovbot"
			}
		case OBJTYPE_PADEFFECT:				return "Pad Effect\n";
		case OBJTYPE_CHOPPER:
			switch (prop->obj->modelnum) {
			case MODEL_A51INTERCEPTOR:
				return langGet(L_MISC_A51INTERCEPTOR); // "A51 Interceptor"
			case MODEL_DD_HOVERCOPTER:
			default:
				return langGet(L_MISC_HOVERCOPTER); // "Hovercopter"
			}
		case OBJTYPE_MINE:					return "Land Mine\n";
		case OBJTYPE_ESCASTEP:				return "Escalator\n";
		default:
			break;
		}
		break;
	default:
		break;
	}
	return langGet(L_MPWEAPONS_062); // "Unknown"
}

Gfx *hudRenderCombatSimulatorOrders(Gfx *gdl)
{
	s32 textwidth;
	s32 textheight;
	s32 x = viGetViewLeft() + 10;
	s32 x2;
	s32 y = viGetViewTop() + 10;
	s32 i;
	char text[64];
	char text2[128];
	char *aibotname;
	u32 teamcolor = 0x00ff00ff;
	u32 leftmargin = 0;

	if (!(g_MpSetup.options & MPOPTION_TEAMSENABLED)) {
		return gdl;
	}

	for (i = PLAYERCOUNT(); i < g_MpNumChrs; i++) {
		if (g_Vars.currentplayer->prop->chr->team != g_MpAllChrConfigPtrs[i]->team) {
			continue;
		}

		sprintf(text, "%s:-\n", scenarioRemoveLineBreaks(g_MpAllChrConfigPtrs[i]->name, 0));
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
		if (leftmargin < textwidth) {
			leftmargin = textwidth;
		}
	}

	x2 = x + leftmargin;

	for (i = PLAYERCOUNT(); i < g_MpNumChrs; i++) {
		s32 yold = y;

		if (g_Vars.currentplayer->prop->chr->team != g_MpAllChrConfigPtrs[i]->team) {
			continue;
		}

		aibotname = g_MpAllChrConfigPtrs[i]->name;
		if (g_MpAllChrPtrs[i]->aibot) {
			sprintf(text2, "%s", hudGetPrettyCommand(g_MpAllChrPtrs[i]->aibot, 0));
		} else {
			sprintf(text2, "%s", langGet(L_OPTIONS_290)); // "Deceased"
		}

		if (g_MpSetup.options & MPOPTION_TEAMSENABLED) {
			teamcolor = g_TeamColours[radarGetTeamIndex(g_MpAllChrPtrs[i]->team)] | 0x000000ff;
		}

		sprintf(text, "%s:\n", scenarioRemoveLineBreaks(aibotname, 0));

		gdl = text0f153628(gdl);
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
		textMeasure(&textheight, &textwidth, text2, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

		textwidth += x;
		textheight += y;

		gdl = text0f153990(gdl, x, y, leftmargin + textwidth, textheight);
		gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, teamcolor, 0xa0, viGetWidth(), viGetHeight(), 0, 0);
		y = yold;
		gdl = textRender(gdl, &x2, &y, text2, g_CharsHandelGothicXs, g_FontHandelGothicXs, teamcolor, 0xa0, viGetWidth(), viGetHeight(), 0, 0);
		gdl = text0f153780(gdl);

		y += 1;
	}

	return gdl;
}

Gfx *hudRenderCoopSimulantOrders(Gfx *gdl)
{
	s32 textwidth;
	s32 textheight;
	s32 x = viGetViewLeft() + 10;
	s32 x2;
	s32 y = viGetViewTop() + 10;
	s32 i;
	char text[64];
	char text2[128];
	char *aibotname;
	u32 teamcolor = 0x00ff00ff;
	u32 leftmargin = 0;

	for (i = 0; i < ARRAYCOUNT(g_Vars.aibuddies); i++) {
		if (g_Vars.aibuddies[i]) {
			sprintf(text, "%s:-\n", scenarioRemoveLineBreaks(langGet(g_Buddies[g_Vars.aibuddytype[i]].nametextid), 0));
			textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
			if (leftmargin < textwidth) {
				leftmargin = textwidth;
			}
		}
	}

	x2 = x + leftmargin;

	for (i = 0; i < ARRAYCOUNT(g_Vars.aibuddies); i++) {
		s32 yold = y;

		if (g_Vars.aibuddytype[i] == BUDDY_NONE) {
			continue;
		}

		aibotname = langGet(g_Buddies[g_Vars.aibuddytype[i]].nametextid);
		if (g_Vars.aibuddies[i] && g_Vars.aibuddies[i]->chr) {
			if (chrIsDead(g_Vars.aibuddies[i]->chr)) {
				sprintf(text2, "%s", langGet(L_OPTIONS_290)); // "Deceased"
			} else {
				switch (g_Vars.aibuddies[i]->chr->actiontype) {
				case ACT_INIT:
				case ACT_STAND:
				case ACT_KNEEL:
				case ACT_ANIM:
				case ACT_SIDESTEP:
				case ACT_JUMPOUT:
				case ACT_RUNPOS:
				case ACT_PATROL:
				case ACT_GOPOS:
				case ACT_LOOKATTARGET:
				case ACT_SURPRISED:
				case ACT_STARTALARM:
				case ACT_TURNDIR:
				case ACT_TEST:
				case ACT_BONDINTRO:
				case ACT_BONDMULTI:
				case ACT_DRUGGEDCOMINGUP:
				case ACT_SKJUMP:
				default:
					sprintf(text2, "%s", langGet(L_OPTIONS_291)); // "Active"
					break;
				case ACT_DIE:
				case ACT_DEAD:
				case ACT_BONDDIE:
					sprintf(text2, "%s", langGet(L_OPTIONS_290)); // "Deceased"
					break;
				case ACT_ARGH:
				case ACT_PREARGH:
					sprintf(text2, "%s", "Injured\n");
					break;
				case ACT_ATTACK:
				case ACT_ATTACKWALK:
				case ACT_ATTACKROLL:
				case ACT_THROWGRENADE:
				case ACT_BOT_ATTACKSTAND:
				case ACT_BOT_ATTACKKNEEL:
				case ACT_BOT_ATTACKSTRAFE:
				case ACT_ATTACKAMOUNT:
				case ACT_ROBOTATTACK:
				case ACT_PUNCH:
				case ACT_CUTFIRE:
					sprintf(text2, "%s", langGet(L_MISC_176)); // "Attack"
					break;
				case ACT_SURRENDER:
					sprintf(text2, "%s", langGet(L_OPTIONS_292)); // "Disavowed"
					break;
				case ACT_NULL:
					sprintf(text2, "%s", langGet(L_MPWEAPONS_062)); // "Unknown"
					break;
				case ACT_DRUGGEDDROP:
				case ACT_DRUGGEDKO:	
					sprintf(text2, "%s", "Incapacitated\n");
					break;
				}
			}
		} else {
			sprintf(text2, "%s", langGet(L_OPTIONS_290)); // "Deceased"
			break;
		}

		sprintf(text, "%s:\n", scenarioRemoveLineBreaks(aibotname, 0));

		gdl = text0f153628(gdl);
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
		textMeasure(&textheight, &textwidth, text2, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

		textwidth += x;
		textheight += y;

		gdl = text0f153990(gdl, x, y, leftmargin + textwidth, textheight);
		gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, teamcolor, 0xa0, viGetWidth(), viGetHeight(), 0, 0);
		y = yold;
		gdl = textRender(gdl, &x2, &y, text2, g_CharsHandelGothicXs, g_FontHandelGothicXs, teamcolor, 0xa0, viGetWidth(), viGetHeight(), 0, 0);
		gdl = text0f153780(gdl);

		y += 1;
	}

	return gdl;
}

Gfx *hudRenderCoopSimulantDebug(Gfx *gdl)
{
	return hudRenderCoopSimulantOrders(gdl);
}

Gfx *hudRenderCombatSimulantDebug(Gfx *gdl)
{
	s32 textwidth;
	s32 textheight;
	s32 x = viGetViewLeft() + 10;
	s32 x2;
	s32 y = viGetViewTop() + 10;
	s32 i;
	char text[64];
	char text2[128];
	char *aibotname;
	u32 teamcolor = 0x00ff00ff;
	u32 leftmargin = 0;

	for (i = PLAYERCOUNT(); i < g_MpNumChrs; i++) {
		sprintf(text, "%s:-\n", scenarioRemoveLineBreaks(g_MpAllChrConfigPtrs[i]->name, 0));
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
		if (leftmargin < textwidth) {
			leftmargin = textwidth;
		}
	}

	x2 = x + leftmargin;

	for (i = PLAYERCOUNT(); i < g_MpNumChrs; i++) {
		s32 yold = y;

		aibotname = g_MpAllChrConfigPtrs[i]->name;
		if (g_MpAllChrPtrs[i]->aibot) {
			switch (g_MpAllChrPtrs[i]->myaction) {
			case MA_NONE:				sprintf(text2, "%s", langGet(L_MPWEAPONS_058)); break; // "Nothing"
			case MA_NORMAL:				sprintf(text2, "MA_NORMAL\n"); break;
			case MA_COVERWAIT:			sprintf(text2, "MA_COVERWAIT\n"); break;
			case MA_GRENADEWAIT:		sprintf(text2, "MA_GRENADEWAIT\n"); break;
			case MA_WAITING:			sprintf(text2, "MA_WAITING\n"); break;
			case MA_COVERGOTO:			sprintf(text2, "MA_COVERGOTO\n"); break;
			case MA_COVERBREAK:			sprintf(text2, "MA_COVERBREAK\n"); break;
			case MA_COVERSEEN:			sprintf(text2, "MA_COVERSEEN\n"); break;
			case MA_FLANKLEFT:			sprintf(text2, "MA_FLANKLEFT\n"); break;
			case MA_FLANKRIGHT:			sprintf(text2, "MA_FLANKRIGHT\n"); break;
			case MA_DODGE:				sprintf(text2, "MA_DODGE\n"); break;
			case MA_GRENADE:			sprintf(text2, "MA_GRENADE\n"); break;
			case MA_WAITSEEN:			sprintf(text2, "MA_WAITSEEN\n"); break;
			case MA_WITHDRAW:			sprintf(text2, "MA_WITHDRAW\n"); break;
			case MA_SHOOTING:			sprintf(text2, "MA_SHOOTING\n"); break;
			case MA_SYNCSHOOT:			sprintf(text2, "MA_SYNCSHOOT\n"); break;
			case MA_WAITTIMEOUT:		sprintf(text2, "MA_WAITTIMEOUT\n"); break;
			case MA_COVERTIMEOUT:		sprintf(text2, "MA_COVERTIMEOUT\n"); break;
			case MA_TRACKING:			sprintf(text2, "MA_TRACKING\n"); break;
			case MA_RETREAT:			sprintf(text2, "MA_RETREAT\n"); break;
			case MA_SURRENDER:			sprintf(text2, "MA_SURRENDER\n"); break;
			case MA_TALKING:			sprintf(text2, "MA_TALKING\n"); break;
			case MA_LISTENING:			sprintf(text2, "MA_LISTENING\n"); break;
			case MA_GOTOALARM:			sprintf(text2, "MA_GOTOALARM\n"); break;
			case MA_BOTFRIENDFOLLOW:	sprintf(text2, "MA_BOTFRIENDFOLLOW\n"); break;
			case MA_BOTHIDE:			sprintf(text2, "MA_BOTHIDE\n"); break;
			case MA_BOTPATH:			sprintf(text2, "MA_BOTPATH\n"); break;
			case MA_BOTINJURED:			sprintf(text2, "MA_BOTINJURED\n"); break;
			case MA_BOTNORMAL:			sprintf(text2, "MA_BOTNORMAL\n"); break;
			case MA_BOTSHOOTING:		sprintf(text2, "MA_BOTSHOOTING\n"); break;
			case MA_DRUGGED:			sprintf(text2, "MA_DRUGGED\n"); break;
			case MA_PANIC:				sprintf(text2, "MA_PANIC\n"); break;
			case MA_RUNFROMGRENADE:		sprintf(text2, "MA_RUNFROMGRENADE\n"); break;
			case MA_UNARMEDATTACK:		sprintf(text2, "MA_UNARMEDATTACK\n"); break;
			case MA_SKJUMP_START:		sprintf(text2, "MA_SKJUMP_START\n"); break;
			case MA_SKJUMP_AIR:			sprintf(text2, "MA_SKJUMP_AIR\n"); break;
			case MA_SKJUMP_LAND:		sprintf(text2, "MA_SKJUMP_LAND\n"); break;
			case MA_SKJUMP_LANDLOOP:	sprintf(text2, "MA_SKJUMP_LANDLOOP\n"); break;
			case MA_SKJUMP_SHOT:		sprintf(text2, "MA_SKJUMP_SHOT\n"); break;
			case MA_AIBOTDEADLIST:		sprintf(text2, "%s", langGet(L_OPTIONS_290)); break; // "Deceased"
			case MA_AIBOTGETITEM:		sprintf(text2, "Get %s", hudGetPropName(g_MpAllChrPtrs[i]->aibot->gotoprop)); break;
			case MA_AIBOTGOTOPOS:		sprintf(text2, "Go to Room %i\n", g_MpAllChrPtrs[i]->aibot->gotorooms[0]); break;
			case MA_AIBOTGOTOPROP:		sprintf(text2, "Go to %s", hudGetPropName(g_MpAllChrPtrs[i]->aibot->gotoprop)); break;
			case MA_AIBOTRUNAWAY:		sprintf(text2, "MA_AIBOTRUNAWAY\n"); break;
			case MA_AIBOTDOWNLOAD:		sprintf(text2, "%s", langGet(L_MISC_180)); break; // "Download"
			case MA_AIBOTATTACK:		sprintf(text2, "%s", langGet(L_MISC_176)); break; // "Attack"
			case MA_AIBOTFOLLOW: {
				if (g_MpAllChrPtrs[i]->aibot->followingplayernum >= 0) {
					sprintf(text2, "%s %s", scenarioRemoveLineBreaks(langGet(L_MISC_175), 0), g_MpAllChrConfigPtrs[g_MpAllChrPtrs[i]->aibot->followingplayernum]->name); // "Follow [Target]"
					break;
				} else {
					sprintf(text2, "%s", langGet(L_MISC_175)); // "Follow"
					break;
				}
			}
			case MA_AIBOTDEFEND:		sprintf(text2, "%s", langGet(L_MISC_177)); break; // "Defend"
			case MA_FLANKBEST:			sprintf(text2, "MA_FLANKBEST\n"); break;
			case MA_FACING:				sprintf(text2, "MA_FACING\n"); break;
			case MA_PUNCHING:			sprintf(text2, "%s", langGet(L_GUN_100)); break; // "Punch"
			case MA_AIBOTINIT:
			case MA_AIBOTMAINLOOP:
			default:
				sprintf(text2, "%s", langGet(L_OPTIONS_291)); // "Active"
				break;
			}
		} else {
			sprintf(text2, "%s", langGet(L_OPTIONS_290)); // "Deceased"
		}

		if (g_MpSetup.options & MPOPTION_TEAMSENABLED) {
			teamcolor = g_TeamColours[radarGetTeamIndex(g_MpAllChrPtrs[i]->team)] | 0x000000ff;
		}

		sprintf(text, "%s:\n", scenarioRemoveLineBreaks(aibotname, 0));

		gdl = text0f153628(gdl);
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
		textMeasure(&textheight, &textwidth, text2, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

		textwidth += x;
		textheight += y;

		gdl = text0f153990(gdl, x, y, leftmargin + textwidth, textheight);
		gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, teamcolor, 0xa0, viGetWidth(), viGetHeight(), 0, 0);
		y = yold;
		gdl = textRender(gdl, &x2, &y, text2, g_CharsHandelGothicXs, g_FontHandelGothicXs, teamcolor, 0xa0, viGetWidth(), viGetHeight(), 0, 0);
		gdl = text0f153780(gdl);

		y += 1;
	}

	return gdl;
}

Gfx *hudRenderTeamWeapons(Gfx *gdl)
{
	s32 textwidth;
	s32 textheight;
	s32 x = viGetViewLeft() + 10;
	s32 x2;
	s32 y = viGetViewTop() + 10;
	s32 i;
	s32 weaponnum;
	char text[64];
	char text2[64];
	char *aibotname;
	u32 teamcolor = 0x00ff00ff;
	u32 leftmargin = 0;

	if (!(g_MpSetup.options & MPOPTION_TEAMSENABLED)) {
		return gdl;
	}

	for (i = PLAYERCOUNT(); i < g_MpNumChrs; i++) {
		if (g_Vars.currentplayer->prop->chr->team != g_MpAllChrConfigPtrs[i]->team) {
			continue;
		}

		sprintf(text, "%s:-\n", scenarioRemoveLineBreaks(g_MpAllChrConfigPtrs[i]->name, 0));
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
		if (leftmargin < textwidth) {
			leftmargin = textwidth;
		}
	}

	x2 = x + leftmargin;

	for (i = PLAYERCOUNT(); i < g_MpNumChrs; i++) {
		s32 yold = y;

		if (g_Vars.currentplayer->prop->chr->team != g_MpAllChrConfigPtrs[i]->team) {
			continue;
		}

		aibotname = g_MpAllChrConfigPtrs[i]->name;
		if (g_MpAllChrPtrs[i]->aibot) {
			weaponnum = g_MpAllChrPtrs[i]->aibot->weaponnum;
			if (weaponnum >= WEAPON_FALCON2 && weaponnum <= WEAPON_HORIZONSCANNER) {
				sprintf(text2, "%s", bgunGetShortName(weaponnum));
			} else {
				sprintf(text2, "%s", langGet(L_MISC_173)); // "No Weapon"
			}
		} else {
			sprintf(text2, "%s", langGet(L_MISC_173)); // "No Weapon"
		}

		if (g_MpSetup.options & MPOPTION_TEAMSENABLED) {
			teamcolor = g_TeamColours[radarGetTeamIndex(g_MpAllChrPtrs[i]->team)] | 0x000000ff;
		}

		sprintf(text, "%s:\n", scenarioRemoveLineBreaks(aibotname, 0));

		gdl = text0f153628(gdl);
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
		textMeasure(&textheight, &textwidth, text2, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

		textwidth += x;
		textheight += y;

		gdl = text0f153990(gdl, x, y, leftmargin + textwidth, textheight);
		gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, teamcolor, 0xa0, viGetWidth(), viGetHeight(), 0, 0);
		y = yold;
		gdl = textRender(gdl, &x2, &y, text2, g_CharsHandelGothicXs, g_FontHandelGothicXs, teamcolor, 0xa0, viGetWidth(), viGetHeight(), 0, 0);
		gdl = text0f153780(gdl);

		y += 1;
	}

	return gdl;
}

Gfx *hudRenderRoomDebug(Gfx *gdl)
{
	s32 textwidth;
	s32 textheight;
	s32 x = viGetViewLeft() + 10;
	s32 y = viGetViewTop() + 10;
	char text[128];

	sprintf(text, "Room %i\n", g_Vars.currentplayer->cam_room);

	gdl = text0f153628(gdl);
	textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

	textwidth += x;
	textheight += y;

	gdl = text0f153990(gdl, x, y, textwidth, textheight);
	gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0x00ff00ff, 0xa0, viGetWidth(), viGetHeight(), 0, 0);
	gdl = text0f153780(gdl);

	return gdl;
}

Gfx *hudRenderPlayerSelected(Gfx *gdl)
{
	if (g_Vars.normmplayerisrunning) {
		if (cheatIsActive(CHEAT_HUD_ORDERS)) {
			return hudRenderCombatSimulatorOrders(gdl);
		} else if (cheatIsActive(CHEAT_HUD_BOTDEBUG)) {
			return hudRenderCombatSimulantDebug(gdl);
		}
	} else {
		if (cheatIsActive(CHEAT_HUD_ORDERS)) {
			return hudRenderCoopSimulantOrders(gdl);
		} else if (cheatIsActive(CHEAT_HUD_BOTDEBUG)) {
			return hudRenderCoopSimulantDebug(gdl);
		}
	}

	return gdl;
}

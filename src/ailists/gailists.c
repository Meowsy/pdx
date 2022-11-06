#include <ultra64.h>
#include "game/chraicommands.h"
#include "game/prop.h"
#include "stagesetup.h"
#include "data.h"
#include "types.h"

/**
 * gailists - Global AI Lists
 *
 * AI lists are bytecode that is interpreted by the game engine to implement AI.
 * The u8 arrays below make heavy use of macros for readability purposes.
 *
 * The file contains the AI lists that are common to all stages such as general
 * guard combat.
 */

struct ailist *tmp = g_GlobalAilists;

/**
 * @ailist GAILIST_IDLE
 */
u8 func0000_idle[] = {
	terminate
	endlist
};

/**
 * @ailist GAILIST_END_CINEMA
 */
u8 func0005_end_cinema[] = {
	enter_firstperson
	terminate
	endlist
};

#define LABEL_AIVSAIFAIL         0x16
#define LABEL_DISGUISE_UNCOVERED 0x16
#define LABEL_SCAN_START         0x1a
#define LABEL_SEE_DETECT         0x1e
#define LABEL_HEAR_DETECT        0x1f
#define LABEL_NEAR_MISS          0x20
#define LABEL_SEEFRIENDDIE       0x21
#define LABEL_HEARD_ALARM        0x23
#define LABEL_HEARSPAWN          0xda

/**
 * @ailist GAILIST_UNALERTED
 */
u8 func0006_unalerted[] = {
	// Handle psychosis
	if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_PSYCHOSISED, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_returnlist(CHR_SELF, GAILIST_INIT_PSYCHOSIS)
	set_ailist(CHR_SELF, GAILIST_INIT_PSYCHOSIS)

	// Check state of chr
	label(0x16)
	set_target_chr(CHR_P1P2)
	set_shotlist(GAILIST_UNALERTED)
	if_chr_deadish(CHR_SELF, /*goto*/ 0x15)
	if_just_injured(CHR_SELF, /*goto*/ 0x16)
	if_has_gun(CHR_SELF, /*goto*/ 0x13)
	goto_next(0x16)

	// Has gun
	label(0x13)
	goto_next(LABEL_SCAN_START)

	// Dying
	label(0x15)
	set_shotlist(GAILIST_IDLE)
	terminate

	// Injured or gun has just been shot out of hand
	label(0x16)
	say_quip(CHR_BOND, QUIP_SHOTUNALERT, 0x78, 0x03, 0x00, BANK_0, 0x00, 0x00)
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, FALSE, BANK_1, /*goto*/ 0x06)
	unset_self_flag_bankx(CHRFLAG1_DOINGIDLEANIMATION, BANK_1)
	if_chr_idle_action_eq(IDLEACTION_SITTING_TYPING, /*goto*/ 0x05)
	if_chr_idle_action_eq(IDLEACTION_SITTING_DORMANT, /*goto*/ 0x05)
	goto_next(0x06)

	// Injured or gun dropped while sitting -> stand up
	label(0x05)
	set_chr_maxdamage(CHR_SELF, 40)
	object_do_animation(ANIM_025A, 0xff, 0x02, 0xffff)
	chr_do_animation(ANIM_STAND_UP_FROM_SITTING, 0, -1, CHRANIMFLAG_MOVEWHENINVIS, 16, CHR_SELF, 2)
	unset_self_flag_bankx(CHRFLAG1_DOINGIDLEANIMATION, BANK_1)
	set_chr_special_death_animation(CHR_SELF, SPECIALDIE_NONE)
	goto_next(0x0c)

	// Injured or gun dropped while standing
	label(0x06)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_ASKEDABOUTGUN, /*goto*/ 0x14)
	goto_next(0x13)

	// Has asked about gun
	label(0x14)
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_04000000)

	// Wait for the chr's injured animation to end
	label(0x13)
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_ALMOSTUNCOVERED)
	if_chr_idle(/*goto*/ 0x13)
	goto_next(0x03)

	label(0x13)
	stop_chr
	goto_next(0x16)

	beginloop(0x03)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0x03)

	// If warned already, go straight to scan
	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG1_PREWARNED, TRUE, BANK_1, /*goto*/ LABEL_SCAN_START)

	say_quip(CHR_BOND, QUIP_SHOTUNALERT, 0xff, 0x03, 0x00, BANK_0, 0x00, 0x00)
	if_self_flag_bankx_eq(CHRFLAG0_CANT_ALERT_GROUP, TRUE, BANK_0, /*goto*/ 0x13)
	increase_squadron_alertness(100)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_WAKEUP)

	// Armed
	label(LABEL_SCAN_START)
	set_action(MA_NORMAL, TRUE)
	yield
	chr_toggle_p1p2(CHR_SELF)
	set_target_chr(CHR_P1P2)

	label(0x13)
	if_alertness(100, OPERATOR_LESS_THAN, /*goto*/ 0x16)

	// Chr is already alert
	goto_next(0x78)

	// If not alert, check for witnessing death or injury
	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG1_PREWARNED, TRUE, BANK_1, /*goto*/ 0x13)
	if_saw_death(1, /*goto*/ LABEL_SEEFRIENDDIE)
	if_saw_injury(1, /*goto*/ LABEL_NEAR_MISS)

	// Check if target is in sight
	label(0x13)
	if_target_outside_my_yvisang(/*goto*/ 0x16)
	if_target_in_sight(/*goto*/ LABEL_SEE_DETECT)

	// Check if AIVSAI is enabled and can see AI enemy
	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_AIVSAI, FALSE, BANK_0, /*goto*/ LABEL_AIVSAIFAIL)
	if_enemy_distance_lt_and_los(2540, /*goto*/ 0x13)
	goto_next(LABEL_AIVSAIFAIL)

	label(0x13)
	if_target_is_player(/*goto*/ LABEL_AIVSAIFAIL)
	goto_next(0x12)

	// Check for near miss and hear
	label(LABEL_AIVSAIFAIL)
	if_near_miss(/*goto*/ LABEL_SEE_DETECT)
	if_num_times_shot_gt(0, /*goto*/ LABEL_SEE_DETECT)
	if_self_flag_bankx_eq(CHRFLAG0_NOHEAR, FALSE, BANK_0, /*goto*/ 0x16)
	if_can_see_target(/*goto*/ 0x16)
	goto_next(0x13)

	// Hear enabled, or hear disabled but can see target
	label(0x16)
	if_heard_target_recently(/*goto*/ LABEL_HEAR_DETECT)

	label(0x13)

	// Check alarm
	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_HEAR_ALARMS, FALSE, BANK_0, /*goto*/ 0x16)
	if_alarm_active(/*goto*/ LABEL_HEARD_ALARM)

	// Hear alarm is disabled for this chr, or alarm is off
	label(0x16)
	if_alarm_active(/*goto*/ 0x13)
	goto_next(0x16)

	// Alarm on but chr is not allowed to hear it
	label(0x13)
	if_stage_is_not(STAGE_PELAGIC, /*goto*/ 0x16)
	if_onscreen(/*goto*/ LABEL_HEARD_ALARM)

	// Check if can see camspy
	label(0x16)
	set_target_to_camspy_if_in_sight(/*goto*/ 0x16)
	goto_next(0x13)

	// Can see camspy
	label(0x16)
	if_patrolling(/*goto*/ 0x15)
	set_returnlist(CHR_SELF, GAILIST_UNALERTED)
	set_ailist(CHR_SELF, GAILIST_OBSERVE_CAMSPY)

	// Patroller seeing camspy
	label(0x15)
	set_returnlist(CHR_SELF, GAILIST_PATROLLER_DIS_TALKING)
	set_ailist(CHR_SELF, GAILIST_OBSERVE_CAMSPY)

	// Can't see camspy
	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_NOIDLEANIMS, TRUE, BANK_1, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, TRUE, BANK_1, /*goto*/ 0x13)
	if_self_flag_bankx_eq(CHRFLAG1_TALKINGTODISGUISE, FALSE, BANK_1, /*goto*/ 0x15)
	if_chr_not_talking(CHR_SELF, /*goto*/ 0x28)
	if_timer_gt(60, /*goto*/ 0x28)
	goto_next(0x15)

	// Chr is not talking
	label(0x28)
	unset_self_flag_bankx(CHRFLAG1_TALKINGTODISGUISE, BANK_1)
	stop_chr

	// CHRFLAG1_TALKINGTODISGUISE was off, or timer not at 1 second yet
	// Consider doing an idle animation
	label(0x15)
	if_chr_idle(/*goto*/ 0x16)
	call_rng
	if_rand_gt(1, /*goto*/ 0x16)
	call_rng
	if_rand_gt(20, /*goto*/ 0x16)
	if_patrolling(/*goto*/ 0x15)
	set_returnlist(CHR_SELF, GAILIST_UNALERTED)
	set_ailist(CHR_SELF, GAILIST_DO_BORED_ANIMATION)

	label(0x15)
	set_returnlist(CHR_SELF, GAILIST_PATROLLER_DIS_TALKING)
	set_ailist(CHR_SELF, GAILIST_DO_BORED_ANIMATION)

	// Consider looking around
	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_LOOK_AROUND, FALSE, BANK_1, /*goto*/ 0x16)
	call_rng
	if_rand_gt(10, /*goto*/ 0x16)
	call_rng
	if_rand_gt(20, /*goto*/ 0x16)
	if_patrolling(/*goto*/ 0x15)
	set_returnlist(CHR_SELF, GAILIST_UNALERTED)
	set_ailist(CHR_SELF, GAILIST_LOOK_AROUND)

	label(0x15)
	set_returnlist(CHR_SELF, GAILIST_PATROLLER_DIS_TALKING)
	set_ailist(CHR_SELF, GAILIST_LOOK_AROUND)

	// Go back to scan start
	label(0x16)
	goto_first(LABEL_SCAN_START)

	/***************************************************************************
	 * Idle animations
	 **************************************************************************/

	label(0x13)
	if_chr_idle(/*goto*/ 0x16)
	set_returnlist(CHR_SELF, GAILIST_UNALERTED)
	set_ailist(CHR_SELF, GAILIST_BUSY)

	label(0x16)
	goto_first(LABEL_SCAN_START)

	/***************************************************************************
	 * Seeing player
	 **************************************************************************/

	label(LABEL_SEE_DETECT)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DISGUISE_UNCOVERED, /*goto*/ LABEL_DISGUISE_UNCOVERED)

	label(0x13)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_ALMOSTUNCOVERED, /*goto*/ 0x14)
	goto_next(0x13)

	label(0x14)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_NONE, /*goto*/ 0x15)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_UNARMED, /*goto*/ 0x15)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_COMBATBOOST, /*goto*/ 0x15)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_HORIZONSCANNER, /*goto*/ 0x15)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_SUITCASE, /*goto*/ 0x15)
	if_chr_in_view(/*goto*/ LABEL_DISGUISE_UNCOVERED)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_04000000, /*goto*/ LABEL_DISGUISE_UNCOVERED)

	label(0x15)

	label(0x13)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DISGUISED, /*goto*/ 0x13)
	goto_next(LABEL_DISGUISE_UNCOVERED)

	label(0x13)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_NONE, /*goto*/ 0x15)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_UNARMED, /*goto*/ 0x15)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_COMBATBOOST, /*goto*/ 0x15)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_HORIZONSCANNER, /*goto*/ 0x15)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_SUITCASE, /*goto*/ 0x15)
	goto_next(0x14)

	label(0x15)
	if_chr_soundtimer(600, OPERATOR_GREATER_THAN, /*goto*/ 0x15)
	goto_next(0xea)

	label(0x14)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_ALMOSTUNCOVERED, /*goto*/ 0xe8)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_ASKEDABOUTGUN, /*goto*/ 0xe8)

	label(0xea)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_NONE, /*goto*/ 0xe6)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_UNARMED, /*goto*/ 0xe6)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_COMBATBOOST, /*goto*/ 0xe6)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_HORIZONSCANNER, /*goto*/ 0xe6)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_SUITCASE, /*goto*/ 0xe6)
	if_distance_to_target_gt(900, /*goto*/ 0x15)
	goto_next(0xe8)

	label(0xe6)
	if_distance_to_target_gt(450, /*goto*/ 0x15)

	label(0xe8)
	if_can_see_target(/*goto*/ 0xe9)
	goto_next(LABEL_DISGUISE_UNCOVERED)

	label(0xe9)
	label(0x13)
	if_within_targets_fovx_by_angle(10, /*goto*/ 0x13)
	goto_next(0x15)

	label(0x13)
	if_patrolling(/*goto*/ 0x14)
	set_returnlist(CHR_SELF, GAILIST_UNALERTED)
	set_ailist(CHR_SELF, GAILIST_CIVILIAN_SAY_COMMENT)

	label(0x14)
	set_returnlist(CHR_SELF, GAILIST_PATROLLER_DIS_TALKING)
	set_ailist(CHR_SELF, GAILIST_CIVILIAN_SAY_COMMENT)

	label(0x15)
	goto_first(LABEL_SCAN_START)

	/***************************************************************************
	 * Disguise uncovered
	 **************************************************************************/

	label(LABEL_DISGUISE_UNCOVERED)
	if_near_miss(/*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG0_SURPRISABLE, TRUE, BANK_0, /*goto*/ 0x94)

	// Near miss or cannot be surprised
	label(0x16)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DISGUISED, /*goto*/ 0x14)
	if_chr_in_squadron_doing_action(MA_GOTOALARM, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG0_ACTIVATEALARM, TRUE, BANK_0, /*goto*/ 0x9f)

	label(0x16)
	say_quip(CHR_BOND, QUIP_SEEPLAYER, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)

	label(0x9f)
	increase_squadron_alertness(100)
	goto_next(0x77)

	label(0xdb) // jumped to from below
	label(0x14)
	say_quip(CHR_BOND, QUIP_UNCOVERDISGUISE1, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	increase_squadron_alertness(100)
	goto_next(0x77)

	// Unreachable?
	label(0x16)
	yield
	yield
	say_quip(CHR_BOND, QUIP_SHOTUNALERT, 0xff, 0x03, 0x00, BANK_0, 0x00, 0x00)
	increase_squadron_alertness(100)
	goto_next(0x77)

	// Surprised due to disguise broken
	label(0x94)
	label(0x16)
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_DETECTED)
	say_quip(CHR_BOND, QUIP_SURPRISED, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	restart_timer
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, FALSE, BANK_1, /*goto*/ 0x16)
	if_chr_idle_action_eq(IDLEACTION_SITTING_TYPING, /*goto*/ 0x13)
	if_chr_idle_action_eq(IDLEACTION_SITTING_DORMANT, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_chr_maxdamage(CHR_SELF, 40)
	goto_next(0x73)

	label(0x16)
	chr_do_animation(ANIM_SURPRISED_0202, 0, -1, 0, 16, CHR_SELF, 2)

	beginloop(0x73)
		if_timer_gt(60, /*goto*/ 0x77)
	endloop(0x73)

	label(0x77)
	set_alertness(255)
	if_self_flag_bankx_eq(CHRFLAG0_CANT_ALERT_GROUP, TRUE, BANK_0, /*goto*/ 0x13)
	increase_squadron_alertness(100)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_WAKEUP)

	/***************************************************************************
	 * Hearing player
	 **************************************************************************/

	label(LABEL_HEAR_DETECT)
	yield
	say_quip(CHR_BOND, QUIP_HEARNOISE, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_HEARSPAWN, TRUE, BANK_0, /*goto*/ LABEL_HEARSPAWN)
	restart_timer
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, TRUE, BANK_1, /*goto*/ 0x80)
	chr_do_animation(ANIM_YAWN, 0, 193, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)

	beginloop(0x80)
		if_timer_gt(60, /*goto*/ 0x81)
	endloop(0x80)

	label(0x81)
	if_self_flag_bankx_eq(CHRFLAG0_GP1, TRUE, BANK_0, /*goto*/ 0x13)
	if_self_flag_bankx_eq(CHRFLAG1_LOOKINGFORTARGET, TRUE, BANK_1, /*goto*/ 0x13)
	set_ailist(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER)

	label(0x13)
	increase_squadron_alertness(100)
	set_ailist(CHR_SELF, GAILIST_WAKEUP)

	/***************************************************************************
	 * Hearing player + spawn clone
	 **************************************************************************/

	label(LABEL_HEARSPAWN)
	if_never_been_onscreen(/*goto*/ 0x0e)
	set_ailist(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER)

	label(0x0e)
	if_never_been_onscreen(/*goto*/ 0x13)
	set_ailist(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER)

	// Never been on screen
	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG0_ALERT_ON_HEARSPAWN, FALSE, BANK_0, /*goto*/ 0x13)
	set_alertness(100)

	label(0x13)
	if_chr_death_animation_finished(CHR_CLONE, /*goto*/ 0x0e)
	if_chr_knockedout(CHR_CLONE, /*goto*/ 0x0e)
	set_ailist(CHR_SELF, GAILIST_UNALERTED)

	label(0x0e)
	try_spawn_clone2(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER, 0, /*goto*/ 0x13)
	if_self_flag_bankx_eq(CHRFLAG0_ALERT_ON_HEARSPAWN, TRUE, BANK_0, /*goto*/ 0x13)
	set_ailist(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_UNALERTED)

	/***************************************************************************
	 * Near miss
	 **************************************************************************/

	label(LABEL_NEAR_MISS)
	increase_self_alertness(100)
	say_quip(CHR_BOND, QUIP_SHOTUNALERT, 0xff, 0x03, 0x00, BANK_0, 0x00, 0x00)
	set_target_chr(CHR_BOND)
	if_nearly_in_targets_sight(30, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DISGUISED, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_ALMOSTUNCOVERED)
	goto_next(0x94)

	label(0x16)
	if_chr_death_animation_finished(CHR_COOP, /*goto*/ 0x16)
	set_target_chr(CHR_COOP)
	if_nearly_in_targets_sight(30, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DISGUISED, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_ALMOSTUNCOVERED)
	goto_next(0x94)

	label(0x16)
	goto_next(0x94)

	/***************************************************************************
	 * Seeing another chr die
	 **************************************************************************/

	label(LABEL_SEEFRIENDDIE)
	if_self_flag_bankx_eq(CHRFLAG0_SURPRISABLE, FALSE, BANK_0, /*goto*/ 0x17)

	// Surprised due to seeing friend die
	label(0x94)
	label(0x16)
	say_quip(CHR_BOND, QUIP_SURPRISED, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	restart_timer
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, TRUE, BANK_1, /*goto*/ 0x76)
	chr_do_animation(ANIM_SURPRISED_0202, 0, -1, 0, 16, CHR_SELF, 2)

	beginloop(0x76)
		if_timer_gt(60, /*goto*/ 0x17)
	endloop(0x76)

	// Go to body
	label(0x17)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_EXAMINE_BODY, FALSE, BANK_0, /*goto*/ 0x7d)
	try_jog_to_chr(CHR_SEEDIE, /*goto*/ 0x7e)

	beginloop(0x7e)
		if_saw_injury(0x00, /*goto*/ 0x16)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_target_outside_my_yvisang(/*goto*/ 0x13)
		if_target_in_sight(/*goto*/ 0x16)

		label(0x13)
		if_distance_to_chr_lt(150, CHR_SEEDIE, /*goto*/ 0x7f)
		if_chr_stopped(/*goto*/ 0x7f)
	endloop(0x7e)

	label(0x16)
	goto_first(LABEL_SEE_DETECT)

	// At body
	label(0x7f)
	restart_timer
	kneel

	beginloop(0x7c)
		if_saw_injury(0, /*goto*/ 0x16)
		set_hear_distance(10000)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_hears_target(/*goto*/ 0x16)
		if_target_outside_my_yvisang(/*goto*/ 0x13)
		if_target_in_sight(/*goto*/ 0x17)

		label(0x13)
		if_self_flag_bankx_eq(CHRFLAG0_CAN_EXAMINE_BODY, FALSE, BANK_0, /*goto*/ 0x13)
		if_timer_lt(120, /*goto*/ 0x13)
		say_quip(CHR_BOND, QUIP_INSPECTBODY, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
		unset_self_flag_bankx(CHRFLAG0_CAN_EXAMINE_BODY, BANK_0)

		label(0x13)
		if_timer_gt(600, /*goto*/ 0x7d)
	endloop(0x7c)

	// Heard gunfire
	label(0x16)
	set_alertness(100)
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_ALMOSTUNCOVERED)
	goto_first(LABEL_SCAN_START)

	// Saw player
	label(0x17)
	set_alertness(100)
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_ALMOSTUNCOVERED)
	goto_first(LABEL_SCAN_START)

	// Examination finished
	label(0x7d)
	set_target_chr(CHR_BOND)
	if_nearly_in_targets_sight(30, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_alertness(100)
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_ALMOSTUNCOVERED)
	goto_first(0xdb)

	// Can't see Joanna
	label(0x16)
	if_chr_death_animation_finished(CHR_COOP, /*goto*/ 0x16)
	set_target_chr(CHR_COOP)
	if_nearly_in_targets_sight(30, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_alertness(100)
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_ALMOSTUNCOVERED)
	goto_first(0xdb)

	label(0x16)
	set_target_chr(CHR_P1P2)
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, FALSE, BANK_1, /*goto*/ 0x13)
	set_ailist(CHR_SELF, GAILIST_WAKEUP)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER)

	// Unreachable - nothing jumps here
	label(0x22)
	increase_self_alertness(100)
	goto_next(0x16)

	/***************************************************************************
	 * Hearing alarm
	 **************************************************************************/

	label(LABEL_HEARD_ALARM)
	if_self_flag_bankx_eq(CHRFLAG1_HEARSPAWN_ALERT_IF_ALARM, FALSE, BANK_1, /*goto*/ 0x13)
	set_self_flag_bankx(CHRFLAG0_ALERT_ON_HEARSPAWN, BANK_0)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_HEARSPAWN, TRUE, BANK_0, /*goto*/ LABEL_HEARSPAWN)
	restart_timer
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, TRUE, BANK_1, /*goto*/ 0x82)
	chr_do_animation(ANIM_YAWN, 0, 193, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)

	beginloop(0x82)
		if_timer_gt(60, /*goto*/ 0x81)
	endloop(0x82)

	label(0x81)
	increase_squadron_alertness(100)
	set_alertness(255)
	set_ailist(CHR_SELF, GAILIST_WAKEUP)

	/***************************************************************************
	 * Hearing alarm + spawn clone
	 **************************************************************************/

	label(LABEL_HEARSPAWN)
	yield
	if_never_been_onscreen(/*goto*/ 0x0e)
	set_ailist(CHR_SELF, GAILIST_WAKEUP)

	label(0x0e)
	if_never_been_onscreen(/*goto*/ 0x13)
	set_ailist(CHR_SELF, GAILIST_WAKEUP)

	label(0x13)
	if_chr_death_animation_finished(CHR_CLONE, /*goto*/ 0x0e)
	if_chr_knockedout(CHR_CLONE, /*goto*/ 0x0e)
	if_self_flag_bankx_eq(CHRFLAG0_ALERT_ON_HEARSPAWN, FALSE, BANK_0, /*goto*/ 0x13)
	goto_first(LABEL_HEARSPAWN)

	label(0x13)
	goto_first(LABEL_SCAN_START)

	label(0x0e)
	try_spawn_clone2(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER, 0, /*goto*/ 0x13)
	if_self_flag_bankx_eq(CHRFLAG0_ALERT_ON_HEARSPAWN, FALSE, BANK_0, /*goto*/ 0x16)
	goto_first(LABEL_HEARSPAWN)

	label(0x16)
	goto_first(LABEL_SCAN_START)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG0_ALERT_ON_HEARSPAWN, FALSE, BANK_0, /*goto*/ 0x13)
	goto_first(LABEL_HEARSPAWN)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_UNALERTED)

	// Unreachable due to set_ailist above
	increase_self_alertness(255)
	goto_next(0x16)

	label(0x16)
	goto_first(LABEL_SCAN_START)

	/***************************************************************************
	 * Warned by friend
	 **************************************************************************/

	label(0x78)
	if_self_flag_bankx_eq(CHRFLAG0_ALERT_ON_HEARSPAWN, FALSE, BANK_0, /*goto*/ 0x13)
	goto_first(0x0e) // create spawn

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_HEARSPAWN, FALSE, BANK_0, /*goto*/ 0x13)
	set_alertness(0)
	goto_first(LABEL_HEARSPAWN)

	label(0x13)
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0x7b)
	restart_timer
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, TRUE, BANK_1, /*goto*/ 0x7a)
	chr_do_animation(ANIM_YAWN, 0, 193, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)

	beginloop(0x7a)
		if_timer_gt(60, /*goto*/ 0x7b)
	endloop(0x7a)

	label(0x7b)
	set_ailist(CHR_SELF, GAILIST_WAKEUP)

	// Unreachable - nothing jumps here
	// Likely a copy/paste error as this code appears in another ailist
	label(0xe4)
	set_returnlist(CHR_SELF, GAILIST_UNALERTED)
	set_ailist(CHR_SELF, GAILIST_FLEE_FROM_GRENADE)

	/***************************************************************************
	 * AI vs AI
	 **************************************************************************/

	// When two AIs engage each other, the one with CHRFLAG1_AIVSAI_ADVANTAGED
	// will attack first and the other will go unalert until they are attacked.
	// This allows the good AI to get the drop on the enemy AI.
	label(0x12)
	if_self_flag_bankx_eq(CHRFLAG1_AIVSAI_ADVANTAGED, TRUE, BANK_1, /*goto*/ 0x13)
	set_returnlist(CHR_SELF, GAILIST_UNALERTED)

	// Surprised at seeing another enemy AI?
	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG0_SURPRISABLE, FALSE, BANK_0, /*goto*/ 0x77)
	say_quip(CHR_BOND, QUIP_SURPRISED, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	restart_timer
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, TRUE, BANK_1, /*goto*/ 0x75)
	chr_do_animation(ANIM_SURPRISED_0202, 0, -1, 0, 16, CHR_SELF, 2)

	beginloop(0x75)
		if_timer_gt(60, /*goto*/ 0x77)
	endloop(0x75)

	label(0x77)
	if_chr_in_squadron_doing_action(MA_GOTOALARM, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG0_ACTIVATEALARM, TRUE, BANK_0, /*goto*/ 0x13)

	label(0x16)
	set_ailist(CHR_SELF, GAILIST_INIT_COMBAT)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_WAKEUP)

	/***************************************************************************
	 * Standing up from sitting
	 **************************************************************************/

	label(0x0c)
	yield
	if_chr_stopped(/*goto*/ 0x13)
	goto_first(0x0c)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_WAKEUP)
	endlist
};

/**
 * @ailist GAILIST_WAKEUP
 */
u8 func0008_wakeup[] = {
	// Mark target as detected
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_DETECTED)

	// Handle psychosis
	if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_PSYCHOSISED, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_returnlist(CHR_SELF, GAILIST_INIT_PSYCHOSIS)
	set_ailist(CHR_SELF, GAILIST_INIT_PSYCHOSIS)

	// Unset special death animation if no longer idle
	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, FALSE, BANK_1, /*goto*/ 0x13)
	set_chr_special_death_animation(CHR_SELF, SPECIALDIE_NONE)

	// If idle, unset it and stand up or whatever is needed
	label(0x13)
	set_shotlist(GAILIST_ALERTED)
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, FALSE, BANK_1, /*goto*/ 0x06)
	unset_self_flag_bankx(CHRFLAG1_DOINGIDLEANIMATION, BANK_1)
	if_chr_idle_action_eq(IDLEACTION_SITTING_TYPING, /*goto*/ 0x05)
	if_chr_idle_action_eq(IDLEACTION_SITTING_DORMANT, /*goto*/ 0x05)
	goto_next(0x06)

	// About to stand up
	label(0x05)
	yield
	if_chr_deadish(CHR_SELF, /*goto*/ 0x13)
	goto_next(0x15)

	// Dead
	label(0x13)
	set_shotlist(GAILIST_IDLE)
	terminate

	// Stand up
	label(0x15)
	set_chr_maxdamage(CHR_SELF, 40)
	chr_do_animation(ANIM_STAND_UP_FROM_SITTING, 0, -1, CHRANIMFLAG_MOVEWHENINVIS, 16, CHR_SELF, 2)
	object_do_animation(ANIM_025A, 0xff, 0x02, 0xffff)
	goto_next(0x0c)

	// Unreachable - nothing jumps here
	label(0x16)
	object_do_animation(ANIM_027B, 0xff, 0x02, 0xffff)

	// Wait for stand up animation to finish
	label(0x0c)
	yield
	if_chr_stopped(/*goto*/ 0x13)
	goto_first(0x0c)

	label(0x13)
	label(0x06)
	set_ailist(CHR_SELF, GAILIST_ALERTED)
	endlist
};

#define LABEL_TARGETGONE    0x01
#define LABEL_DODGE         0x18
#define LABEL_MAINLOOP      0x1b
#define LABEL_TRACK         0x34
#define LABEL_WAITTIMEOUT   0x3d
#define LABEL_THROWGRENADE  0x48
#define LABEL_GUNJAMMED     0x57
#define LABEL_COVERINVALID  0x5b
#define LABEL_ATTACKING     0x60
#define LABEL_RETREAT       0x6b
#define LABEL_SURRENDER     0x70
#define LABEL_FLANK_LEFT    0x8f
#define LABEL_FLANK_RIGHT   0x90
#define LABEL_WARNFRIENDS   0x95
#define LABEL_RUN_FOR_ALARM 0x9f
#define LABEL_GETGUN_FAILED 0xaf
#define LABEL_SNIPE         0xde
#define LABEL_FLEE_GRENADE  0xe4
#define LABEL_DRAW_PISTOL   0xe7
#define LABEL_TARGETCLOAKED 0xeb

/**
 * @ailist GAILIST_ALERTED
 *
 * This ailist handles the main logic for alerted chrs.
 */
u8 func0007_alerted[] = {
	// Mark target as detected
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_DETECTED)

	// Handle psychosis
	if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_PSYCHOSISED, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_returnlist(CHR_SELF, GAILIST_INIT_PSYCHOSIS)
	set_ailist(CHR_SELF, GAILIST_INIT_PSYCHOSIS)

	// If doing idle animation, turn off special death animation
	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, FALSE, BANK_1, /*goto*/ 0x13)
	set_chr_special_death_animation(CHR_SELF, SPECIALDIE_NONE)

	// If Skedar, just do combat
	label(0x13)
	unset_self_flag_bankx(CHRFLAG1_LOOKINGFORTARGET, BANK_1)
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)

	// Human/Maian
	label(0x16)
	set_shotlist(GAILIST_ALERTED)
	set_aishootingatmelist(GAILIST_DODGE)
	set_darkroomlist(GAILIST_SEARCH_FOR_PLAYER)

	if_chr_deadish(CHR_SELF, /*goto*/ 0x16)
	goto_next(0x13)

	// Dying
	label(0x16)
	set_chr_dodge_rating(2, 0)
	set_shotlist(GAILIST_IDLE)

	restart_timer

	beginloop(0x1c)
		if_timer_gt(15, /*goto*/ 0x16)
	endloop(0x1c)

	label(0x16)
	say_quip(CHR_BOND, QUIP_DIE, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	set_shotlist(GAILIST_IDLE)
	terminate

	// Alive
	label(0x13)
	if_just_injured(CHR_SELF, /*goto*/ 0x16)
	if_has_gun(CHR_SELF, /*goto*/ 0x13)
	goto_next(0x16)

	// Not injured and has gun
	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_DRAW_PISTOL, TRUE, BANK_1, /*goto*/ LABEL_DRAW_PISTOL)
	goto_next(LABEL_MAINLOOP)

	// Injured or no gun
	label(0x16)
	restart_timer
	unset_self_flag_bankx(CHRFLAG1_CAN_RELOAD, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_CAN_DRAW_PISTOL, BANK_1)

	// Say an injury quip if we haven't already, then wait up to 3 seconds for
	// the chr to have finished their injury animation
	beginloop(0x1d)
		if_self_flag_bankx_eq(CHRFLAG0_SAID_INJURY_QUIP, TRUE, BANK_0, /*goto*/ 0x16)
		if_num_times_shot_lt(1, /*goto*/ 0x16)
		if_timer_lt(20, /*goto*/ 0x16)
		say_quip(CHR_BOND, QUIP_INJURED1, 0x28, 0x03, 0x00, BANK_0, 0x00, 0x00)
		say_quip(CHR_BOND, QUIP_INJURED2, 0x28, 0x03, 0x01, BANK_0, 0x00, 0x00)
		set_self_flag_bankx(CHRFLAG0_SAID_INJURY_QUIP, BANK_0)

		label(0x16)
#if VERSION >= VERSION_NTSC_1_0
		if_timer_gt(180, /*goto*/ 0x13)
#endif
		if_chr_stopped(/*goto*/ 0x13)
	endloop(0x1d)

	label(0x13)
#if VERSION >= VERSION_NTSC_1_0
	stop_chr
#endif
	if_self_flag_bankx_eq(CHRFLAG1_CAN_DRAW_PISTOL, TRUE, BANK_1, /*goto*/ LABEL_DRAW_PISTOL)
	if_has_gun(CHR_SELF, /*goto*/ LABEL_MAINLOOP)

	label(0xac)
	set_chr_dodge_rating(2, 0)
	say_quip(CHR_BOND, QUIP_LOSTGUN, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	stop_chr
	restart_timer
	yield

	// Wait for gun to land. If it disappears (eg. falls out of geometry), is
	// picked up by the player, or we've been waiting for 5 seconds then go to
	// GETGUN_FAILED.
	label(0xb0)
	yield
	yield
	yield
	if_gun_unclaimed(-1, TRUE, /*goto*/ 0x13)
	goto_next(LABEL_GETGUN_FAILED)

	label(0x13)
	if_gun_landed(/*goto*/ 0xb1)
	if_timer_gt(300, /*goto*/ LABEL_GETGUN_FAILED)
	goto_first(0xb0)

	label(0xb1)
	restart_timer
	go_to_gun(/*goto*/ 0xad)

	beginloop(0xad)
		if_timer_gt(300, /*goto*/ LABEL_GETGUN_FAILED)
		if_gun_unclaimed(-1, TRUE, /*goto*/ 0x13)
		goto_next(LABEL_GETGUN_FAILED)

		label(0x13)
		if_distance_to_gun_lt(100, /*goto*/ 0x16)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0xad)

	label(0x16)
	restart_timer
	chr_do_animation(ANIM_PICK_UP_GUN, 0, -1, CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)

	beginloop(0xae)
		if_timer_gt(60, /*goto*/ 0x16)
	endloop(0xae)

	label(0x16)
	if_gun_unclaimed(-1, TRUE, /*goto*/ 0x13)
	goto_next(LABEL_GETGUN_FAILED)

	label(0x13)
	chr_recover_gun(CHR_SELF, /*goto*/ 0x16)
	goto_next(LABEL_GETGUN_FAILED)

	label(0x16)
	goto_next(LABEL_MAINLOOP)

	/***************************************************************************
	 * Get gun failed
	 **************************************************************************/

	label(LABEL_GETGUN_FAILED)
	stop_chr
	call_rng
	if_rand_lt(50, /*goto*/ 0xe6)
	if_rand_lt(100, /*goto*/ LABEL_DRAW_PISTOL)
	set_self_flag_bankx(CHRFLAG0_CAN_RETREAT, BANK_0)
	set_self_flag_bankx(CHRFLAG0_CANT_ALERT_GROUP, BANK_0)
	set_self_flag_bankx(CHRFLAG0_CAN_FLEESURRENDER, BANK_0)
	unset_self_flag_bankx(CHRFLAG0_SKIPSAFETYCHECKS, BANK_0)
	unset_self_flag_bankx(CHRFLAG0_FORCESAFETYCHECKS, BANK_0)
	unset_self_flag_bankx(CHRFLAG0_CANLOSEGUN, BANK_0)
	set_self_flag_bankx(CHRFLAG1_LOSTGUN, BANK_1)
	goto_next(LABEL_MAINLOOP)

	label(0xe6)
	set_self_flag_bankx(CHRFLAG1_HANDCOMBATONLY, BANK_1)
	goto_next(LABEL_MAINLOOP)

	/***************************************************************************
	 * Draw pistol
	 **************************************************************************/

	label(LABEL_DRAW_PISTOL)
	call_rng
	if_rand_lt(82, /*goto*/ 0x13)
	if_rand_lt(164, /*goto*/ 0x15)
	chr_do_animation(ANIM_DRAW_PISTOL_0288, 0, -1, CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x13)
	chr_do_animation(ANIM_DRAW_PISTOL_0289, 0, -1, CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x15)
	chr_do_animation(ANIM_DRAW_PISTOL_0245, 0, -1, CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)

	label(0x16)
	if_stage_is_not(STAGE_AIRFORCEONE, /*goto*/ 0x13)
	try_equip_weapon(MODEL_CHRCYCLONE, WEAPON_CYCLONE, 0x00000000, /*goto*/ 0xe5)

	label(0x13)
	try_equip_weapon(MODEL_CHRFALCON2, WEAPON_FALCON2, 0x00000000, /*goto*/ 0xe5)

	beginloop(0xe5)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0xe5)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_DRAW_PISTOL, TRUE, BANK_1, /*goto*/ 0x13)
	unset_self_flag_bankx(CHRFLAG0_CANLOSEGUN, BANK_0)

	label(0x13)
	unset_self_flag_bankx(CHRFLAG1_CAN_DRAW_PISTOL, BANK_1)

	label(0x16)

	/***************************************************************************
	 * Main loop
	 **************************************************************************/

	label(LABEL_MAINLOOP)
	yield

	label(0x16)
	if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_00002000, /*goto*/ 0x13)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_UNTARGETABLE, /*goto*/ LABEL_TARGETGONE)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ LABEL_TRACK)

	// Consider warning others in team
	if_alarm_active(/*goto*/ 0x16)
	if_chr_in_squadron_doing_action(MA_GOTOALARM, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG0_ACTIVATEALARM, TRUE, BANK_0, /*goto*/ LABEL_RUN_FOR_ALARM)

	// No need to warn
	label(0x16)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	goto_next(0x16)

	// Unreachable - nothing jumps here
	label(0x13)
	set_returnlist(CHR_SELF, GAILIST_ALERTED)
	set_alertness(0)
	set_ailist(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x93)
	if_orders_eq(MA_FLANKLEFT, /*goto*/ 0x13)
	if_orders_eq(MA_FLANKRIGHT, /*goto*/ 0x15)
	goto_next(0x93)

	label(0x13)
	say_quip(CHR_BOND, QUIP_FLANK, 0x28, 0x03, 0x01, BANK_0, 0x00, 0x00)
	goto_next(LABEL_FLANK_LEFT)

	label(0x15)
	say_quip(CHR_BOND, QUIP_FLANK, 0x28, 0x03, 0x01, BANK_0, 0x00, 0x00)
	goto_next(LABEL_FLANK_RIGHT)

	/***************************************************************************
	 * Trap (hide and ambush when player gets near)
	 **************************************************************************/

	label(0x93)
	if_self_flag_bankx_eq(CHRFLAG0_CANT_ALERT_GROUP, TRUE, BANK_0, /*goto*/ LABEL_RETREAT)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_TRAP, TRUE, BANK_0, /*goto*/ 0x13)
	if_self_flag_bankx_eq(CHRFLAG0_GP1, FALSE, BANK_0, /*goto*/ 0x16)

	label(0x13)
	if_can_see_attack_target(/*goto*/ 0x16)
	goto_next(0x98)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_SAID_AMBUSH_QUIP, TRUE, BANK_0, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG0_NOACCURACYAFTERJAM, FALSE, BANK_0, /*goto*/ 0x16)

	label(0x13)
	if_can_see_attack_target(/*goto*/ 0x16)
	goto_next(0xa1)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_CAMP, FALSE, BANK_0, /*goto*/ 0x16)

	label(0x13)
	goto_next(0xa4)

	// Not popping
	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_SNIPE, FALSE, BANK_1, /*goto*/ 0x16)

	label(0x13)
	goto_next(LABEL_SNIPE)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_RETREAT, FALSE, BANK_0, /*goto*/ 0x16)
	if_num_times_shot_lt(1, /*goto*/ 0x16)
	if_calculated_safety2_lt(3, /*goto*/ LABEL_RETREAT)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_FORCESAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0x91)
	if_self_flag_bankx_eq(CHRFLAG0_SKIPSAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0x92)

	label(0x91)
	if_calculated_safety2_lt(5, /*goto*/ 0x52)

	label(0x92)
	goto_next(0x68)

	/***************************************************************************
	 * Go to trap pad
	 **************************************************************************/

	label(0x98)
	go_to_target_pad(GOPOSFLAG_JOG)

	beginloop(0x99)
		label(0x16)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0x99)

	label(0x16)

	label(0x9b)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	try_attack_kneel(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x03)

	beginloop(0x03)
		if_can_see_attack_target(/*goto*/ 0x9a)
		if_chr_target_eq(CHR_SELF, CHR_P1P2, FALSE, /*goto*/ 0xee)
		goto_next(0xef)

		label(0xee)
		chr_toggle_p1p2(CHR_SELF)
		if_chr_target_eq(CHR_SELF, CHR_P1P2, FALSE, /*goto*/ 0xee)
		set_target_chr(CHR_P1P2)
		if_can_see_attack_target(/*goto*/ 0x9a)

		label(0xee)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)

		label(0xef)
	endloop(0x03)

	// Aiming only
	label(0x9a)
	try_modify_attack(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0x04)

	beginloop(0x04)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0x04)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_TRAP, TRUE, BANK_0, /*goto*/ 0x16)
	if_distance_to_target_lt(800, /*goto*/ 0x9c)

	label(0x16)
	goto_first(0x9b)

	label(0x9c)
	unset_self_flag_bankx(CHRFLAG0_GP1, BANK_0)
	goto_first(LABEL_MAINLOOP)

	label(0xa1)
	kneel

#if VERSION >= VERSION_NTSC_1_0
	beginloop(0xa2)
		if_can_see_attack_target(/*goto*/ 0xa3)
		if_distance_from_target_to_pad_lt(200, PAD_PRESET, /*goto*/ 0x13)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_enemy_distance_lt_and_los(1200, /*goto*/ 0xee)
	endloop(0xa2)

	label(0xee)
	if_target_is_player(/*goto*/ 0x13)
	set_returnlist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)
#else
	beginloop(0xa2)
		if_can_see_attack_target(/*goto*/ 0xa3)
		if_distance_from_target_to_pad_lt(200, PAD_PRESET, /*goto*/ 0x13)
		if_chr_target_eq(CHR_SELF, CHR_P1P2, FALSE, /*goto*/ 0xee)
		goto_next(0xef)

		label(0xee)
		chr_toggle_p1p2(CHR_SELF)
		if_chr_target_eq(CHR_SELF, CHR_P1P2, FALSE, /*goto*/ 0xee)
		set_target_chr(CHR_P1P2)
		if_can_see_attack_target(0xa3)
		if_distance_from_target_to_pad_lt(200, PAD_PRESET, /*goto*/ 0x13)

		label(0xee)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)

		label(0xef)
	endloop(0xa2)
#endif

	label(0x13)
	set_accuracy(30)
	set_self_flag_bankx(CHRFLAG0_SAID_AMBUSH_QUIP, BANK_0)
	say_quip(CHR_BOND, QUIP_ATTACK2, 0x50, 0x02, 0x01, BANK_0, 0x00, 0x00)
	goto_first(LABEL_MAINLOOP)

	label(0xa3)
#if VERSION >= VERSION_NTSC_1_0
#endif
	unset_self_flag_bankx(CHRFLAG0_NOACCURACYAFTERJAM, BANK_0)
	goto_first(LABEL_MAINLOOP)

	/***************************************************************************
	 * Pop (cover)
	 **************************************************************************/

	label(0xa4)
	if_self_flag_bankx_eq(CHRFLAG0_SKIPSAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0xa8)
	find_cover(COVERCRITERIA_0001 | COVERCRITERIA_DISTTOME | COVERCRITERIA_FORCENEWCOVER | COVERCRITERIA_0020 | COVERCRITERIA_ALLOWNEIGHBOURINGROOMS | COVERCRITERIA_2000 | COVERCRITERIA_ALLOWSOFT, /*goto*/ 0xa5)
	goto_next(0x52)

	label(0xa5)
	set_action(MA_COVERGOTO, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)

	label(0x16)
	go_to_cover(GOPOSFLAG_RUN)

	beginloop(0xa6)
		if_chr_stopped(/*goto*/ 0xa7)
	endloop(0xa6)

	label(0xa7)

	label(0xa8)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	try_attack_stand(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0xa9)

	beginloop(0xa9)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_near_miss(/*goto*/ 0x13)
		if_saw_injury(0x00, /*goto*/ 0x13)
		if_saw_death(0x00, /*goto*/ 0x13)
		if_can_see_attack_target(/*goto*/ 0x13)
	endloop(0xa9)

	label(0x13)
	call_rng
	if_rand_gt(150, /*goto*/ 0x13)
	if_can_see_attack_target(/*goto*/ 0x16)
	goto_next(0x13)

	label(0x16)
	try_modify_attack(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0xaa)

	label(0x13)
	kneel
	restart_timer

	beginloop(0xab)
		set_target_chr(CHR_P1P2)
		if_distance_to_target_gt(300, /*goto*/ 0x13)
		unset_self_flag_bankx(CHRFLAG0_CAMP, BANK_0)
		goto_first(LABEL_MAINLOOP)

		label(0x13)
		call_rng
		if_rand_gt(250, /*goto*/ 0x13)
		if_timer_gt(240, /*goto*/ 0x16)
	endloop(0xab)

	label(0x13)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	try_attack_stand(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x13)
	goto_first(0xa8)

	label(0x13)
	if_can_see_attack_target(/*goto*/ 0x13)
	goto_first(0xa8)

	label(0x13)
	try_modify_attack(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0x13)
	goto_first(0xa8)

	label(0x13)

	beginloop(0xaa)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0xaa)

	label(0x16)
	goto_first(0xa8)

	/***************************************************************************
	 * Snipe
	 **************************************************************************/

	label(LABEL_SNIPE)
	unset_self_flag_bankx(CHRFLAG0_CANLOSEGUN, BANK_0)
	yield
	if_distance_to_target_lt(1000, /*goto*/ 0x16)
	if_distance_to_target_gt(2500, /*goto*/ 0x14)

	// Distance between 1000 and 2500
	if_target_in_fov_left(10, /*goto*/ 0x13)
	if_target_out_of_fov_left(246, /*goto*/ 0x13)
	restart_timer
	stop_chr
	try_face_entity(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0xec)

	beginloop(0xec)
		if_target_in_fov_left(10, /*goto*/ 0x13)
		if_target_out_of_fov_left(246, /*goto*/ 0x13)
		if_timer_gt(60, /*goto*/ 0x13)
	endloop(0xec)

	label(0x13)
	if_can_see_attack_target(/*goto*/ 0x13)
	if_chr_target_eq(CHR_SELF, CHR_P1P2, FALSE, /*goto*/ 0xee)
	goto_next(0xef)

	label(0xee)
	chr_toggle_p1p2(CHR_SELF)
	if_chr_target_eq(CHR_SELF, CHR_P1P2, FALSE, /*goto*/ 0xee)
	set_target_chr(CHR_P1P2)
	if_can_see_attack_target(/*goto*/ 0x13)

	label(0xee)
	chr_toggle_p1p2(CHR_SELF)
	set_target_chr(CHR_P1P2)

	label(0xef)
	goto_first(LABEL_SNIPE)

	// Distance > 2500
	label(0x14)
	try_run_to_target(/*goto*/ 0xe1)

	beginloop(0xe1)
		if_distance_to_target_lt(2500, /*goto*/ 0x15)
	endloop(0xe1)

	label(0x15)
	goto_first(LABEL_SNIPE)

	label(0x13)
	restart_timer
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	try_attack_lie(ATTACKFLAG_AIMFORWARD | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0xdf)

	beginloop(0xdf)
		if_distance_to_target_lt(1000, /*goto*/ 0x16)
		if_timer_gt(300, /*goto*/ 0x13)
	endloop(0xdf)

	label(0x13)
	if_can_see_attack_target(/*goto*/ 0x13)
	set_self_flag_bankx(CHRFLAG0_CANLOSEGUN, BANK_0)
	stop_chr
	goto_first(LABEL_MAINLOOP)

	label(0x13)
	restart_timer
	set_accuracy(100)
	try_attack_lie(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0xe0)

	beginloop(0xe0)
		if_chr_stopped(/*goto*/ 0x13)
	endloop(0xe0)

	label(0x13)
	set_accuracy(10)
	stop_chr
	goto_first(LABEL_SNIPE)

	// Distance < 1000
	label(0x16)
	stop_chr
	unset_self_flag_bankx(CHRFLAG1_CAN_SNIPE, BANK_1)
	set_self_flag_bankx(CHRFLAG0_CANLOSEGUN, BANK_0)
	restart_timer

	beginloop(0xbd)
		if_timer_gt(60, /*goto*/ 0x13)
	endloop(0xbd)

	label(0x13)
	goto_first(LABEL_MAINLOOP)

	/***************************************************************************
	 * Popper/ambush failed
	 **************************************************************************/

	label(0x52)
	if_within_targets_fovx_by_angle(60, /*goto*/ 0x16)
	goto_next(0x68)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_UNDERCOVER, TRUE, BANK_0, /*goto*/ 0x16)
	goto_next(LABEL_COVERINVALID)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_CHUCKNORRIS, TRUE, BANK_0, /*goto*/ 0x13)
	call_rng
	if_rand_gt(100, /*goto*/ 0x13)
	if_within_targets_fovx_by_angle(30, /*goto*/ 0x16)

	label(0x13)
	if_nearly_in_targets_sight(30, /*goto*/ 0x5f)

	label(0x16)
	if_nearly_in_targets_sight(30, /*goto*/ 0x16)
	if_player_using_cmp150_or_ar34(/*goto*/ 0x69)
	if_distance_to_target_gt(2000, /*goto*/ 0x61)
	goto_next(0x16)

	label(0x69)
	if_distance_to_target_gt(4000, /*goto*/ 0x61)

	label(0x16)
	if_can_see_attack_target(/*goto*/ LABEL_COVERINVALID)
	if_nearly_in_targets_sight(30, /*goto*/ LABEL_COVERINVALID)
	if_chr_in_view(/*goto*/ LABEL_COVERINVALID)
	goto_next(0x56)

	label(LABEL_COVERINVALID)
	if_self_flag_bankx_eq(CHRFLAG1_ALLOWSOFTCOVER, TRUE, BANK_1, /*goto*/ 0x15)
	if_self_flag_bankx_eq(CHRFLAG0_COVER_TYPE1, TRUE, BANK_0, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG0_COVER_TYPE2, FALSE, BANK_0, /*goto*/ 0x5c)
	find_cover(COVERCRITERIA_0001 | COVERCRITERIA_DISTTOME | COVERCRITERIA_FORCENEWCOVER | COVERCRITERIA_0020 | COVERCRITERIA_ALLOWNEIGHBOURINGROOMS | COVERCRITERIA_1000, /*goto*/ 0x53)
	goto_next(0x14)

	label(0x5c)
	find_cover(COVERCRITERIA_0001 | COVERCRITERIA_DISTTOME | COVERCRITERIA_FORCENEWCOVER | COVERCRITERIA_0020 | COVERCRITERIA_1000, /*goto*/ 0x53)
	goto_next(0x14)

	label(0x16)
	find_cover(COVERCRITERIA_0001 | COVERCRITERIA_DISTTOME | COVERCRITERIA_FORCENEWCOVER | COVERCRITERIA_0020, /*goto*/ 0x53)
	goto_next(0x14)

	label(0x15)
	if_self_flag_bankx_eq(CHRFLAG0_COVER_TYPE1, TRUE, BANK_0, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG0_COVER_TYPE2, FALSE, BANK_0, /*goto*/ 0x5c)
	find_cover(COVERCRITERIA_0001 | COVERCRITERIA_DISTTOME | COVERCRITERIA_FORCENEWCOVER | COVERCRITERIA_0020 | COVERCRITERIA_ALLOWNEIGHBOURINGROOMS | COVERCRITERIA_1000 | COVERCRITERIA_ALLOWSOFT, /*goto*/ 0x53)
	goto_next(0x14)

	label(0x5c)
	find_cover(COVERCRITERIA_0001 | COVERCRITERIA_DISTTOME | COVERCRITERIA_FORCENEWCOVER | COVERCRITERIA_0020 | COVERCRITERIA_1000 | COVERCRITERIA_ALLOWSOFT, /*goto*/ 0x53)
	goto_next(0x14)

	label(0x16)
	find_cover(COVERCRITERIA_0001 | COVERCRITERIA_DISTTOME | COVERCRITERIA_FORCENEWCOVER | COVERCRITERIA_0020 | COVERCRITERIA_ALLOWSOFT, /*goto*/ 0x53)
	goto_next(0x14)

	label(0x14)
	goto_next(0x54)

	label(0x53)
	set_action(MA_COVERGOTO, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x16)
	set_team_orders(MA_SHOOTING, /*goto*/ 0x16)

	label(0x16)
	say_quip(CHR_BOND, QUIP_GOTOCOVER1, 0xff, 0x02, 0x01, BANK_0, 0x00, 0x00)
	restart_timer
	go_to_cover(GOPOSFLAG_RUN)

	beginloop(0x55)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
		if_self_flag_bankx_eq(CHRFLAG1_NOHANDCOMBAT, TRUE, BANK_1, /*goto*/ 0x13)
		if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x16)
		if_distance_to_target_lt(120, /*goto*/ 0x16)
		call_rng
		if_rand_lt(128, /*goto*/ 0x13)
		if_distance_to_target_gt(250, /*goto*/ 0x13)

		label(0x16)
		if_can_see_attack_target(/*goto*/ 0x16)
		goto_next(0x13)

		label(0x16)
		set_returnlist(CHR_SELF, GAILIST_ALERTED)
		set_ailist(CHR_SELF, GAILIST_HAND_COMBAT)

		label(0x13)
		if_timer_gt(300, /*goto*/ 0x56)
		if_chr_stopped(/*goto*/ 0x56)
	endloop(0x55)

	label(0x56)
	set_self_flag_bankx(CHRFLAG0_UNDERCOVER, BANK_0)

	label(0x58)
	set_chrpreset(CHR_TARGET)
	restart_timer
	set_action(MA_COVERWAIT, TRUE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	try_attack_kneel(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x59)

	beginloop(0x59)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_nearly_in_targets_sight(30, /*goto*/ 0x5f)
		if_timer_gt(240, /*goto*/ 0x61)
		if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x93)
		if_has_orders(/*goto*/ 0x84)

		label(0x93)
		label(0x5a)
	endloop(0x59)

	label(0x61)
	set_target_chr(CHR_PRESET)
	if_self_flag_bankx_eq(CHRFLAG0_00008000, TRUE, BANK_0, /*goto*/ 0x66)

	label(0x16)
	goto_next(0x16)

	label(0x66)
	if_player_using_cmp150_or_ar34(/*goto*/ 0x69)
	if_distance_to_target_gt(2000, /*goto*/ 0x16)
	goto_next(0x6a)

	label(0x69)
	if_distance_to_target_gt(4000, /*goto*/ 0x16)

	label(0x6a)
	if_within_targets_fovx_by_angle(17, /*goto*/ 0x63)

	label(0x16)
	label(0x64)
	set_action(MA_COVERBREAK, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x16)
	set_team_orders(MA_COVERBREAK, /*goto*/ 0x16)

	label(0x16)
	cmd012f
	restart_timer
	try_run_to_target(/*goto*/ 0x62)

	beginloop(0x62)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_can_see_attack_target(/*goto*/ 0x5f)
		if_player_using_cmp150_or_ar34(/*goto*/ 0x69)
		if_distance_to_target_gt(2000, /*goto*/ 0x65)
		goto_next(0x16)

		label(0x69)
		if_distance_to_target_gt(4000, /*goto*/ 0x65)

		label(0x16)
		if_timer_gt(120, /*goto*/ 0x63)

		label(0x65)
	endloop(0x62)

	label(0x63)
	if_self_flag_bankx_eq(CHRFLAG0_00008000, FALSE, BANK_0, /*goto*/ 0x13)
	if_within_targets_fovx_by_angle(17, /*goto*/ 0x16)
	goto_first(0x64)

	label(0x13)
	set_self_flag_bankx(CHRFLAG1_LOOKINGFORTARGET, BANK_1)
	goto_next(LABEL_TRACK)

	label(0x16)
	stop_chr
	yield
	if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
	goto_first(0x52)

	label(0x5f)
	cmd012f
	set_action(MA_COVERSEEN, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x16)
	set_team_orders(MA_COVERSEEN, /*goto*/ 0x16)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x93)
	if_has_orders(/*goto*/ 0x84)

	label(0x93)
	if_can_see_attack_target(/*goto*/ 0x26)
	goto_next(0x87)

	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	if_self_flag_bankx_eq(CHRFLAG1_NOHANDCOMBAT, TRUE, BANK_1, /*goto*/ 0x13)
	if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x16)
	if_distance_to_target_lt(120, /*goto*/ 0x16)
	call_rng
	if_rand_lt(128, /*goto*/ 0x13)
	if_distance_to_target_gt(250, /*goto*/ 0x13)

	label(0x16)
	if_can_see_attack_target(/*goto*/ 0x16)
	goto_next(0x13)

	label(0x16)
	set_returnlist(CHR_SELF, GAILIST_ALERTED)
	set_ailist(CHR_SELF, GAILIST_HAND_COMBAT)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_THROWGRENADEFIRST, FALSE, BANK_1, /*goto*/ 0x13)
	unset_self_flag_bankx(CHRFLAG1_THROWGRENADEFIRST, BANK_1)
	goto_next(LABEL_THROWGRENADE)

	/**
	 * Pseudocode for the below logic:
	 *
	 * rand = random()
	 * if rand < 10
	 *     gun jammed
	 * else if rand <= 64
	 *     if CHRFLAG1_CAN_ATTACKAMOUNTRELOAD is set:
	 *         attackamount then reload
	 *     else if CHRFLAG1_CAN_ATTACKAMOUNT is set:
	 *         attackamount
	 *     else
	 *         goto regular_attack
	 * else
	 *     regular_attack:
	 *     if random() > 50
	 *         stand or kneel attack (50% chance each)
	 *     else
	 *         walk or roll attack (50% chance each)
	 */
	label(0x13)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	call_rng
	if_rand_lt(10, /*goto*/ LABEL_GUNJAMMED)
	if_rand_gt(64, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_ATTACKAMOUNTRELOAD, FALSE, BANK_1, /*goto*/ 0x13)
	try_attack_amount(90, 100)
	set_self_flag_bankx(CHRFLAG1_CAN_RELOAD, BANK_1)
	goto_next(LABEL_ATTACKING)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_ATTACKAMOUNT, FALSE, BANK_1, /*goto*/ 0x13)
	try_attack_amount(20, 30)
	goto_next(LABEL_ATTACKING)

	label(0x13)
	label(0x16)
	call_rng
	if_rand_gt(50, /*goto*/ 0x13)
	call_rng
	if_rand_gt(128, /*goto*/ 0x16)

	label(0xbe)
	try_attack_roll(/*goto*/ LABEL_ATTACKING)

	label(0x16)
	try_attack_walk(/*goto*/ LABEL_ATTACKING)

	label(0x13)
	call_rng
	if_rand_gt(128, /*goto*/ 0x8c)
	try_attack_kneel(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x16)

	label(0x8c)
	try_attack_stand(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x16)

	label(0x16)
	try_modify_attack(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ LABEL_ATTACKING)

	beginloop(LABEL_ATTACKING)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_chr_stopped(/*goto*/ 0x5e)
	endloop(LABEL_ATTACKING)

	label(LABEL_GUNJAMMED)
	say_quip(CHR_BOND, QUIP_GUNJAMMED, 0xfe, 0x03, 0xff, BANK_0, 0x00, 0x00)
	do_preset_animation(PRESETANIM_GUNJAM)
	goto_next(0xe8)

	label(0x5e)
	call_rng
	if_rand_lt(20, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_RELOAD, FALSE, BANK_1, /*goto*/ 0x13)

	label(0x16)
	set_self_flag_bankx(CHRFLAG1_CAN_RELOAD, BANK_1)
	restart_timer
	chr_do_animation(ANIM_RELOAD_0209, 0, -1, CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)

	beginloop(0xe8)
		if_self_flag_bankx_eq(CHRFLAG1_CAN_RELOAD, FALSE, BANK_1, /*goto*/ 0x16)
		if_timer_lt(120, /*goto*/ 0x16)
#if VERSION >= VERSION_NTSC_1_0
		assign_sound(SFX_80F6, CHANNEL_7)
#else
		assign_sound(SFX_01D9, CHANNEL_7)
#endif
		play_sound_from_entity(CHANNEL_7, CHR_SELF, 0x0bb8, 0x1770, 0x01)
		unset_self_flag_bankx(CHRFLAG1_CAN_RELOAD, BANK_1)

		label(0x16)
		if_chr_stopped(/*goto*/ 0x13)
	endloop(0xe8)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG0_NOACCURACYAFTERJAM, FALSE, BANK_0, /*goto*/ 0x13)
	unset_self_flag_bankx(CHRFLAG0_NOACCURACYAFTERJAM, BANK_0)
	set_accuracy(0)

	label(0x13)
	check_cover_out_of_sight(/*goto*/ 0x16)

	label(0x16)
	goto_first(LABEL_MAINLOOP)

	label(0x54)

	label(0x68)
	if_can_see_attack_target(/*goto*/ 0x24)
	goto_next(0x25)

	label(0x25)
	goto_next(0x2f)

	label(0x24)
	label(0x4e)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_RETREAT, FALSE, BANK_0, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG0_CANT_ALERT_GROUP, TRUE, BANK_0, /*goto*/ LABEL_RETREAT)
	if_num_times_shot_lt(1, /*goto*/ 0x16)
	if_calculated_safety2_lt(3, /*goto*/ LABEL_RETREAT)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_FORCESAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0x91)
	if_self_flag_bankx_eq(CHRFLAG0_SKIPSAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0x4d)

	label(0x91)
	if_calculated_safety2_lt(5, /*goto*/ 0x32)
	goto_next(0x4d)

	label(0x4d)
	if_self_flag_bankx_eq(CHRFLAG1_FLANKED, TRUE, BANK_1, /*goto*/ 0x32)
	call_rng
	if_within_targets_fovx_by_angle(30, /*goto*/ 0x16)
	goto_next(0x32)

	/***************************************************************************
	 * Flank
	 **************************************************************************/

	label(0x16)
	if_rand_lt(128, /*goto*/ 0x32)

	label(0x31)

	label(0x16)
	label(0x13)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	cmd0139(25, 0x02, TRUE)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x16)
	set_team_orders(MA_FLANKLEFT, /*goto*/ 0x16)
	set_team_orders(MA_FLANKRIGHT, /*goto*/ 0x16)

	label(0x16)
	goto_next(0x33)

	label(LABEL_FLANK_LEFT)
	set_action(MA_FLANKLEFT, FALSE)
	cmd0139(335, 0x02, FALSE)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x16)
	set_team_orders(MA_FLANKLEFT, /*goto*/ 0x16)

	label(0x16)
	goto_next(0x33)

	label(LABEL_FLANK_RIGHT)
	set_action(MA_FLANKRIGHT, FALSE)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x16)
	set_team_orders(MA_FLANKRIGHT, /*goto*/ 0x16)

	label(0x16)
	cmd0139(25, 0x02, FALSE)

	label(0x33)
	set_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	restart_timer

	beginloop(0x30)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
		if_self_flag_bankx_eq(CHRFLAG1_NOHANDCOMBAT, TRUE, BANK_1, /*goto*/ 0x13)
		if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x16)
		if_distance_to_target_lt(120, /*goto*/ 0x16)
		call_rng
		if_rand_lt(128, /*goto*/ 0x13)
		if_distance_to_target_gt(250, /*goto*/ 0x13)

		label(0x16)
		if_can_see_attack_target(/*goto*/ 0x16)
		goto_next(0x13)

		label(0x16)
		set_returnlist(CHR_SELF, GAILIST_ALERTED)
		set_ailist(CHR_SELF, GAILIST_HAND_COMBAT)

		label(0x13)
		if_chr_stopped(/*goto*/ 0x32)
		if_timer_gt(180, /*goto*/ 0x32)
	endloop(0x30)

	// Unreachable - no label
	// This would have prevented consecutive or multiple dodges, or allowed
	// other parts of the AI list to more finely control when a dodge can occur.
	goto_next(0x2f)
	if_self_flag_bankx_eq(CHRFLAG1_DODGED, TRUE, BANK_1, /*goto*/ 0x2f)
	// End unreachable

	label(0x32)
	if_within_targets_fovx_by_angle(20, /*goto*/ 0x16)
	goto_next(0x2f)

	label(0x16)
	call_rng
	if_rand_gt(100, /*goto*/ 0x2f)
	if_chr_in_view(/*goto*/ LABEL_DODGE)

	label(0x2f)
	if_distance_to_target_gt(1300, /*goto*/ LABEL_TRACK)
	if_can_see_attack_target(/*goto*/ 0x26)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_RETREAT, FALSE, BANK_0, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG0_CANT_ALERT_GROUP, TRUE, BANK_0, /*goto*/ LABEL_RETREAT)
	if_num_times_shot_lt(1, /*goto*/ 0x16)
	if_calculated_safety2_lt(3, /*goto*/ LABEL_RETREAT)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_FORCESAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0x91)
	if_self_flag_bankx_eq(CHRFLAG0_SKIPSAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0x4d)

	label(0x91)
	if_calculated_safety2_lt(5, /*goto*/ 0x51)

	label(0x4d)
	if_self_flag_bankx_eq(CHRFLAG0_00008000, TRUE, BANK_0, /*goto*/ LABEL_THROWGRENADE)
	goto_next(LABEL_TRACK)

	label(0x51)
	if_within_targets_fovx_by_angle(21, /*goto*/ 0x16)
	goto_next(LABEL_TRACK)

	/***************************************************************************
	 * Grenades
	 **************************************************************************/

	label(0x16)
	kneel
	yield
	if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)

	label(LABEL_THROWGRENADE)
	set_grenade_probability_out_of_255(255)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_THROW_GRENADES, TRUE, BANK_0, /*goto*/ 0x16)
	goto_next(0x4c)

	label(0x16)
	if_distance_to_target_lt(500, /*goto*/ 0x4c)
	restart_timer
	if_chr_in_squadron_doing_action(MA_GRENADE, /*goto*/ 0x4c)
	set_action(MA_GRENADE, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	consider_throwing_grenade(0x0200, 0x0000, /*goto*/ 0x46)
	goto_next(0x4c)


	beginloop(0x46)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0x46)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x47)
	set_team_orders(MA_GRENADE, /*goto*/ 0x47)

	beginloop(0x47)
		say_quip(CHR_BOND, QUIP_GRENADE2, 0xff, 0x07, 0x00, BANK_0, 0x00, 0x00)
		if_chr_stopped(/*goto*/ 0x4a)
	endloop(0x47)

	label(0x4a)
	if_timer_lt(60, /*goto*/ 0x49)
	set_action(MA_GRENADEWAIT, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	restart_timer
	try_face_entity(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0x4b)

	beginloop(0x4b)
		if_nearly_in_targets_sight(30, /*goto*/ 0x42)
		if_chr_in_view(/*goto*/ 0x42)
		if_chr_target_eq(CHR_SELF, CHR_P1P2, FALSE, /*goto*/ 0xee)
		goto_next(0xef)

		label(0xee)
		chr_toggle_p1p2(CHR_SELF)
		if_chr_target_eq(CHR_SELF, CHR_P1P2, FALSE, /*goto*/ 0xee)
		set_target_chr(CHR_P1P2)
		if_nearly_in_targets_sight(30, /*goto*/ 0x42)
		if_chr_in_view(/*goto*/ 0x42)

		label(0xee)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)

		label(0xef)
		if_timer_gt(240, /*goto*/ 0x49)
	endloop(0x4b)

	/***************************************************************************
	 * Waiting
	 **************************************************************************/

	label(0x49)
	label(0x4c)
	restart_timer
	set_action(MA_WAITING, TRUE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	try_attack_kneel(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x40)

	beginloop(0x40)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_self_flag_bankx_eq(CHRFLAG0_FORCESAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0x91)
		if_self_flag_bankx_eq(CHRFLAG0_SKIPSAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0x16)

		label(0x91)
		if_calculated_safety2_lt(5, /*goto*/ 0x41)

		label(0x16)
		if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x93)
		if_has_orders(/*goto*/ 0x84)

		label(0x93)
		if_nearly_in_targets_sight(30, /*goto*/ 0x42)
		if_chr_target_eq(CHR_SELF, CHR_P1P2, FALSE, /*goto*/ 0xee)
		goto_next(0xef)

		label(0xee)
		chr_toggle_p1p2(CHR_SELF)
		if_chr_target_eq(CHR_SELF, CHR_P1P2, FALSE, /*goto*/ 0xee)
		set_target_chr(CHR_P1P2)
		if_nearly_in_targets_sight(30, /*goto*/ 0x42)

		label(0xee)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)

		label(0xef)
		if_timer_gt(600, /*goto*/ LABEL_WAITTIMEOUT)
		if_timer_gt(60, /*goto*/ 0x16)

		label(0x16)
		if_within_targets_fovx_by_angle(21, /*goto*/ 0x16)
		goto_next(LABEL_TRACK)

		label(0x16)
		if_distance_to_target_gt(2000, /*goto*/ 0x41)
	endloop(0x40)

	label(0x42)
	set_action(MA_WAITSEEN, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x16)
	set_team_orders(MA_WAITSEEN, /*goto*/ 0x16)

	label(0x16)
	if_can_see_attack_target(/*goto*/ 0x8a)
	restart_timer
	try_run_to_target(/*goto*/ 0x89)

	beginloop(0x89)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_self_flag_bankx_eq(CHRFLAG0_FORCESAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0x91)
		if_self_flag_bankx_eq(CHRFLAG0_SKIPSAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0x16)

		label(0x91)
		if_calculated_safety2_lt(5, /*goto*/ 0x41)

		label(0x16)
		if_can_see_attack_target(/*goto*/ 0x8a)
		if_timer_gt(60, /*goto*/ 0x29)
	endloop(0x89)

	label(0x8a)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	if_self_flag_bankx_eq(CHRFLAG1_NOHANDCOMBAT, TRUE, BANK_1, /*goto*/ 0x13)
	if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x16)
	if_distance_to_target_lt(120, /*goto*/ 0x16)
	call_rng
	if_rand_lt(128, /*goto*/ 0x13)
	if_distance_to_target_gt(250, /*goto*/ 0x13)

	label(0x16)
	if_can_see_attack_target(/*goto*/ 0x16)
	goto_next(0x13)

	label(0x16)
	set_returnlist(CHR_SELF, GAILIST_ALERTED)
	set_ailist(CHR_SELF, GAILIST_HAND_COMBAT)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_THROWGRENADEFIRST, FALSE, BANK_1, /*goto*/ 0x13)
	unset_self_flag_bankx(CHRFLAG1_THROWGRENADEFIRST, BANK_1)
	goto_first(LABEL_THROWGRENADE)

	label(0x13)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	call_rng
	if_rand_lt(10, /*goto*/ LABEL_GUNJAMMED)
	if_rand_gt(64, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_ATTACKAMOUNTRELOAD, FALSE, BANK_1, /*goto*/ 0x13)
	try_attack_amount(90, 100)
	set_self_flag_bankx(CHRFLAG1_CAN_RELOAD, BANK_1)
	goto_next(0x43)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_ATTACKAMOUNT, FALSE, BANK_1, /*goto*/ 0x13)
	try_attack_amount(20, 30)
	goto_next(0x43)

	label(0x13)
	label(0x16)
	call_rng
	if_rand_gt(50, /*goto*/ 0x13)
	call_rng
	if_rand_gt(128, /*goto*/ 0x16)

	label(0xbe)
	try_attack_roll(/*goto*/ 0x43)

	label(0x16)
	try_attack_walk(/*goto*/ 0x43)

	label(0x13)
	call_rng
	if_rand_gt(128, /*goto*/ 0x8c)
	try_attack_stand(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x16)

	label(0x8c)
	try_attack_kneel(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x16)

	label(0x16)
	try_modify_attack(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0x43)

	beginloop(0x43)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_chr_stopped(/*goto*/ 0x29)
	endloop(0x43)

	label(LABEL_GUNJAMMED)
	say_quip(CHR_BOND, QUIP_GUNJAMMED, 0xfe, 0x03, 0xff, BANK_0, 0x00, 0x00)
	do_preset_animation(PRESETANIM_GUNJAM)
	goto_next(0xe9)

	label(0x29)
	if_self_flag_bankx_eq(CHRFLAG0_NOACCURACYAFTERJAM, FALSE, BANK_0, /*goto*/ 0x13)
	unset_self_flag_bankx(CHRFLAG0_NOACCURACYAFTERJAM, BANK_0)
	set_accuracy(0)

	label(0x13)
	if_chr_injured_target(CHR_SELF, /*goto*/ 0x13)
	say_quip(CHR_BOND, QUIP_MISSEDPLAYER1, 0x19, 0x03, 0x00, BANK_0, 0x00, 0x00)
	say_quip(CHR_BOND, QUIP_MISSEDPLAYER2, 0x19, 0x03, 0x01, BANK_0, 0x00, 0x00)
	goto_next(0x16)

	label(0x13)
	say_quip(CHR_BOND, QUIP_HITPLAYER, 0x19, 0x02, 0x00, BANK_0, 0x00, 0x00)
	say_quip(CHR_BOND, QUIP_HITPLAYER, 0x19, 0x03, 0x01, BANK_0, 0x00, 0x00)

	// Reload
	label(0x16)
	call_rng
	if_rand_lt(20, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_RELOAD, FALSE, BANK_1, /*goto*/ 0x13)

	label(0x16)
	set_self_flag_bankx(CHRFLAG1_CAN_RELOAD, BANK_1)
	restart_timer
	chr_do_animation(ANIM_RELOAD_0209, 0, -1, CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)

	beginloop(0xe9)
		if_self_flag_bankx_eq(CHRFLAG1_CAN_RELOAD, FALSE, BANK_1, /*goto*/ 0x16)
		if_timer_lt(120, /*goto*/ 0x16)
#if VERSION >= VERSION_NTSC_1_0
		assign_sound(SFX_80F6, CHANNEL_6)
#else
		assign_sound(SFX_01D9, CHANNEL_6)
#endif
		play_sound_from_entity(CHANNEL_6, CHR_SELF, 0x0bb8, 0x1770, 0x01)
		unset_self_flag_bankx(CHRFLAG1_CAN_RELOAD, BANK_1)

		label(0x16)
		if_chr_stopped(/*goto*/ 0x13)
	endloop(0xe9)

	label(0x13)

	/***************************************************************************
	 * Back off
	 **************************************************************************/

	label(0x8b)
	if_within_targets_fovx_by_angle(20, /*goto*/ 0x13)
	goto_next(0x50)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_BACKOFF, TRUE, BANK_0, /*goto*/ 0x16)
	goto_next(0x50)

	label(0x16)
	set_action(MA_WITHDRAW, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	restart_timer
	retreat(0x02, 0x01)
	cmd012f

	beginloop(0x4f)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_timer_gt(120, /*goto*/ 0x50)
		if_chr_stopped(/*goto*/ 0x50)
	endloop(0x4f)

	label(0x50)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	if_self_flag_bankx_eq(CHRFLAG1_NOHANDCOMBAT, TRUE, BANK_1, /*goto*/ 0x13)
	if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x16)
	if_distance_to_target_lt(120, /*goto*/ 0x16)
	call_rng
	if_rand_lt(128, /*goto*/ 0x13)
	if_distance_to_target_gt(250, /*goto*/ 0x13)

	label(0x16)
	if_can_see_attack_target(/*goto*/ 0x16)
	goto_next(0x13)

	label(0x16)
	set_returnlist(CHR_SELF, GAILIST_ALERTED)
	set_ailist(CHR_SELF, GAILIST_HAND_COMBAT)

	label(0x13)
	if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	goto_next(0x16)

	label(0x13)
	set_returnlist(CHR_SELF, GAILIST_ALERTED)
	set_alertness(0)
	set_ailist(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER)

	if_self_flag_bankx_eq(CHRFLAG0_HOLD_POSITION, TRUE, BANK_0, /*goto*/ 0x16)
	call_rng
	if_rand_gt(196, /*goto*/ 0x16)
	goto_next(LABEL_TRACK)

	label(0x16)
	goto_first(LABEL_MAINLOOP)

	/***************************************************************************
	 * Wait timeout
	 **************************************************************************/

	label(LABEL_WAITTIMEOUT)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_TRAP, FALSE, BANK_0, /*goto*/ 0x13)
	goto_first(LABEL_MAINLOOP)

	label(0x13)
	set_action(MA_WAITTIMEOUT, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x16)
	set_team_orders(MA_WAITTIMEOUT, /*goto*/ 0x16)

	label(0x16)
	goto_next(0x3f)

	// Unreachable - no label
	try_run_sideways(/*goto*/ 0x3e)

	beginloop(0x3e)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_chr_stopped(/*goto*/ 0x3f)
	endloop(0x3e)

	label(0x3f)
	goto_first(LABEL_MAINLOOP)

	label(0x41)
	goto_first(LABEL_MAINLOOP)

	/***************************************************************************
	 * Track (go towards target)
	 **************************************************************************/

	label(LABEL_TRACK)
	set_chrpreset(CHR_TARGET)
	set_action(MA_TRACKING, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	restart_timer

	label(0xf7)
	try_run_to_target(/*goto*/ 0x35)

	beginloop(0x35)
		if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_00002000, /*goto*/ 0x13)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_UNTARGETABLE, /*goto*/ LABEL_TARGETGONE)

		label(0x13)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
		if_self_flag_bankx_eq(CHRFLAG1_NOHANDCOMBAT, TRUE, BANK_1, /*goto*/ 0x13)
		if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x16)
		if_distance_to_target_lt(120, /*goto*/ 0x16)
		call_rng
		if_rand_lt(128, /*goto*/ 0x13)
		if_distance_to_target_gt(250, /*goto*/ 0x13)

		label(0x16)
		if_can_see_attack_target(/*goto*/ 0x16)
		goto_next(0x13)

		label(0x16)
		set_returnlist(CHR_SELF, GAILIST_ALERTED)
		set_ailist(CHR_SELF, GAILIST_HAND_COMBAT)

		label(0x13)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_distance_to_target_gt(1300, /*goto*/ 0x13)
		if_self_flag_bankx_eq(CHRFLAG1_LOOKINGFORTARGET, TRUE, BANK_1, /*goto*/ 0x16)
		if_self_flag_bankx_eq(CHRFLAG1_STOPTRACKINGIFLOOKEDAT, FALSE, BANK_1, /*goto*/ 0x16)
		if_within_targets_fovx_by_angle(21, /*goto*/ 0x3b)
		goto_next(0x16)

		// Unreachable - nothing jumps to here
		label(0xc8)
		unset_self_flag_bankx(CHRFLAG1_STOPTRACKINGIFLOOKEDAT, BANK_1)
		goto_first(0x31)

		label(0x16)
		if_can_see_attack_target(/*goto*/ 0x26)

		label(0x13)
		if_self_flag_bankx_eq(CHRFLAG0_AIVSAI, FALSE, BANK_0, /*goto*/ 0x13)
		if_enemy_distance_lt_and_los(2540, /*goto*/ 0x16)
		goto_next(0x13)

		label(0x16)
#if VERSION >= VERSION_NTSC_1_0
		if_target_is_player(/*goto*/ 0x13)
#endif
		set_returnlist(CHR_SELF, GAILIST_UNALERTED)
		set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)

		label(0x13)
		if_distance_to_target_gt(1300, /*goto*/ 0x3c)
		if_self_flag_bankx_eq(CHRFLAG0_CAN_RETREAT, FALSE, BANK_0, /*goto*/ 0x16)
		if_self_flag_bankx_eq(CHRFLAG0_CANT_ALERT_GROUP, TRUE, BANK_0, /*goto*/ LABEL_RETREAT)
		if_num_times_shot_lt(1, /*goto*/ 0x16)
		if_calculated_safety2_lt(3, /*goto*/ LABEL_RETREAT)

		label(0x16)
		if_self_flag_bankx_eq(CHRFLAG0_FORCESAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0x91)
		if_self_flag_bankx_eq(CHRFLAG0_SKIPSAFETYCHECKS, TRUE, BANK_0, /*goto*/ 0x4d)

		label(0x91)
		if_calculated_safety2_lt(5, /*goto*/ 0x39)

		label(0x4d)
		if_self_flag_bankx_eq(CHRFLAG0_00008000, TRUE, BANK_0, /*goto*/ 0x3a)
		goto_next(0x3c)

		label(0x39)
		if_self_flag_bankx_eq(CHRFLAG1_LOOKINGFORTARGET, TRUE, BANK_1, /*goto*/ 0x3c)
		if_within_targets_fovx_by_angle(21, /*goto*/ 0x3b)
		if_distance_to_target_lt(1300, /*goto*/ 0x3b)
		goto_next(0x17)

		label(0x3a)
		if_self_flag_bankx_eq(CHRFLAG1_LOOKINGFORTARGET, TRUE, BANK_1, /*goto*/ 0x3c)
		if_within_targets_fovx_by_angle(21, /*goto*/ 0x3b)
		goto_next(0x3c)

		label(0x3c)

		label(0x17)
		if_chr_stopped(/*goto*/ 0x13)
		if_timer_gt(600, /*goto*/ 0x36)
	endloop(0x35)

	label(0x13)
	set_target_chr(CHR_PRESET)
	goto_first(0xf7)

	label(0x36)
	set_target_chr(CHR_PRESET)
	set_alertness(0)
	set_returnlist(CHR_SELF, GAILIST_ALERTED)
	set_ailist(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER)
	goto_first(LABEL_MAINLOOP)

	label(0x3b)
	goto_first(LABEL_MAINLOOP)

	label(0x37)
	goto_first(LABEL_MAINLOOP)

	/***************************************************************************
	 * Sync shoot
	 **************************************************************************/

	label(0x87)
	cmd012f
	set_action(MA_SHOOTING, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	restart_timer
	try_run_to_target(/*goto*/ 0x85)

	beginloop(0x85)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
		if_self_flag_bankx_eq(CHRFLAG1_NOHANDCOMBAT, TRUE, BANK_1, /*goto*/ 0x13)
		if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x16)
		if_distance_to_target_lt(120, /*goto*/ 0x16)
		call_rng
		if_rand_lt(128, /*goto*/ 0x13)
		if_distance_to_target_gt(250, /*goto*/ 0x13)

		label(0x16)
		if_can_see_attack_target(/*goto*/ 0x16)
		goto_next(0x13)

		label(0x16)
		set_returnlist(CHR_SELF, GAILIST_ALERTED)
		set_ailist(CHR_SELF, GAILIST_HAND_COMBAT)

		label(0x13)
		if_can_see_attack_target(/*goto*/ 0x26)
		if_timer_gt(120, /*goto*/ 0x86)
	endloop(0x85)

	label(0x86)
	goto_first(LABEL_MAINLOOP)

	label(0x26)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	if_self_flag_bankx_eq(CHRFLAG1_NOHANDCOMBAT, TRUE, BANK_1, /*goto*/ 0x13)
	if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x16)
	if_distance_to_target_lt(120, /*goto*/ 0x16)
	call_rng
	if_rand_lt(128, /*goto*/ 0x13)
	if_distance_to_target_gt(250, /*goto*/ 0x13)

	label(0x16)
	if_can_see_attack_target(/*goto*/ 0x16)
	goto_next(0x13)

	label(0x16)
	set_returnlist(CHR_SELF, GAILIST_ALERTED)
	set_ailist(CHR_SELF, GAILIST_HAND_COMBAT)

	label(0x13)
	restart_timer
	stop_chr
	set_action(MA_SHOOTING, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	if_self_flag_bankx_eq(CHRFLAG1_THROWGRENADEFIRST, FALSE, BANK_1, /*goto*/ 0x13)
	unset_self_flag_bankx(CHRFLAG1_THROWGRENADEFIRST, BANK_1)
	goto_first(LABEL_THROWGRENADE)

	label(0x13)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ LABEL_TARGETCLOAKED)
	call_rng
	if_rand_lt(10, /*goto*/ LABEL_GUNJAMMED)
	if_rand_gt(64, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_ATTACKAMOUNTRELOAD, FALSE, BANK_1, /*goto*/ 0x13)
	try_attack_amount(90, 100)
	set_self_flag_bankx(CHRFLAG1_CAN_RELOAD, BANK_1)
	goto_next(0x44)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_ATTACKAMOUNT, FALSE, BANK_1, /*goto*/ 0x13)
	try_attack_amount(20, 30)
	goto_next(0x44)

	label(0x13)
	label(0x16)
	call_rng
	if_rand_gt(50, /*goto*/ 0x13)
	call_rng
	if_rand_gt(128, /*goto*/ 0x16)

	label(0xbe)
	try_attack_roll(/*goto*/ 0x44)

	label(0x16)
	try_attack_walk(/*goto*/ 0x44)

	label(0x13)
	call_rng
	if_rand_gt(128, /*goto*/ 0x8c)
	try_attack_kneel(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x28)

	label(0x8c)
	try_attack_stand(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x28)
	yield
	if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
	goto_first(0x8b)

	label(0x28)
	try_modify_attack(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0x44)
	goto_next(0x67)

	label(0x44)
	say_quip(CHR_BOND, QUIP_ATTACK1, 0x19, 0x02, 0x00, BANK_0, 0x00, 0x00)
	say_quip(CHR_BOND, QUIP_ATTACK2, 0x19, 0x02, 0x01, BANK_0, 0x00, 0x00)

	beginloop(0x45)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_chr_stopped(/*goto*/ 0x2a)
	endloop(0x45)

	label(LABEL_GUNJAMMED)
	say_quip(CHR_BOND, QUIP_GUNJAMMED, 0xfe, 0x03, 0xff, BANK_0, 0x00, 0x00)
	do_preset_animation(PRESETANIM_GUNJAM)
	goto_next(0xea)

	label(0x2a)
	if_self_flag_bankx_eq(CHRFLAG0_NOACCURACYAFTERJAM, FALSE, BANK_0, /*goto*/ 0x13)
	unset_self_flag_bankx(CHRFLAG0_NOACCURACYAFTERJAM, BANK_0)
	set_accuracy(0)

	label(0x13)
	if_chr_injured_target(CHR_SELF, /*goto*/ 0x13)
	say_quip(CHR_BOND, QUIP_MISSEDPLAYER1, 0x19, 0x03, 0x00, BANK_0, 0x00, 0x00)
	say_quip(CHR_BOND, QUIP_MISSEDPLAYER2, 0x19, 0x03, 0x01, BANK_0, 0x00, 0x00)
	goto_next(0x16)

	label(0x13)
	say_quip(CHR_BOND, QUIP_HITPLAYER, 0x19, 0x02, 0x00, BANK_0, 0x00, 0x00)
	say_quip(CHR_BOND, QUIP_HITPLAYER, 0x19, 0x03, 0x01, BANK_0, 0x00, 0x00)

	label(0x16)
	call_rng
	if_rand_lt(20, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_RELOAD, FALSE, BANK_1, /*goto*/ 0x13)

	label(0x16)
	set_self_flag_bankx(CHRFLAG1_CAN_RELOAD, BANK_1)
	restart_timer
	chr_do_animation(ANIM_RELOAD_0209, 0, -1, CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)

	beginloop(0xea)
		if_self_flag_bankx_eq(CHRFLAG1_CAN_RELOAD, FALSE, BANK_1, /*goto*/ 0x16)
		if_timer_lt(120, /*goto*/ 0x16)
#if VERSION >= VERSION_NTSC_1_0
		assign_sound(SFX_80F6, CHANNEL_6)
#else
		assign_sound(SFX_01D9, CHANNEL_6)
#endif
		play_sound_from_entity(CHANNEL_6, CHR_SELF, 0x0bb8, 0x1770, 0x01)
		unset_self_flag_bankx(CHRFLAG1_CAN_RELOAD, BANK_1)

		label(0x16)
		if_chr_stopped(/*goto*/ 0x13)
	endloop(0xea)

	label(0x13)
	goto_first(0x8b)

	label(0x67)

	label(0x2b)
	goto_first(0x8b)

	/***************************************************************************
	 * Dodge
	 **************************************************************************/

	label(LABEL_DODGE)
	set_action(MA_DODGE, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	set_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLANK, FALSE, BANK_0, /*goto*/ 0x16)
	set_team_orders(MA_DODGE, /*goto*/ 0x16)

	label(0x16)
	call_rng
	if_rand_lt(128, /*goto*/ 0x2d)
	try_jumpout(/*goto*/ 0x13)
	goto_next(0x2e)

	label(0x2d)
	try_sidestep(/*goto*/ 0x13)
	goto_next(0x2e)

	label(0x13)
	set_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)

	beginloop(0x2c)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_chr_stopped(/*goto*/ 0x2e)
	endloop(0x2c)

	label(0x2e)
	call_rng
	if_rand_lt(128, /*goto*/ 0x16)
	goto_first(0x87)

	label(0x16)
	goto_first(LABEL_MAINLOOP)

	/***************************************************************************
	 * Retreat
	 **************************************************************************/

	label(LABEL_RETREAT)
	set_chr_dodge_rating(0, 0)
	set_chr_dodge_rating(1, 0)
	set_action(MA_RETREAT, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	say_quip(CHR_BOND, QUIP_RETREAT1, 0x19, 0x02, 0x01, BANK_0, 0x00, 0x00)
	try_set_chrpreset_to_unalerted_teammate(0, /*goto*/ LABEL_WARNFRIENDS)
	if_self_flag_bankx_eq(CHRFLAG1_LOSTGUN, TRUE, BANK_1, /*goto*/ 0x16)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_FLEESURRENDER, TRUE, BANK_0, /*goto*/ 0x16)
	unset_self_flag_bankx(CHRFLAG0_CAN_RETREAT, BANK_0)
	unset_self_flag_bankx(CHRFLAG0_CANT_ALERT_GROUP, BANK_0)
	goto_first(LABEL_MAINLOOP)

	label(0x16)
	set_target_chr(CHR_P1P2)
	retreat(2, 1)

	beginloop(0x6c)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_distance_to_target_gt(300, /*goto*/ 0x16)
		if_chr_in_view(/*goto*/ LABEL_SURRENDER)

		label(0x16)
		if_distance_to_target_gt(3000, /*goto*/ 0x6d)
	endloop(0x6c)

	label(0x6d)
	stop_chr

	beginloop(0x6e)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_distance_to_target_lt(3000, /*goto*/ 0x6f)
	endloop(0x6e)

	label(0x6f)
	goto_first(LABEL_RETREAT)

	/***************************************************************************
	 * Surrender
	 **************************************************************************/

	label(LABEL_SURRENDER)
	if_chr_dead(CHR_SELF, /*goto*/ 0x88)
	set_action(MA_SURRENDER, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	say_quip(CHR_BOND, QUIP_SURRENDER, 0xfe, 0x0a, 0xff, BANK_0, 0x00, 0x00)
	restart_timer
	surrender

	beginloop(0x71)
		if_chr_dead(CHR_SELF, /*goto*/ 0x88)
		if_timer_gt(300, /*goto*/ 0x16)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_chr_in_view(/*goto*/ 0x72)
		if_onscreen(/*goto*/ 0x13)
	endloop(0x71)

	// Out of view for 5 seconds - remove
	label(0x16)
	drop_concealed_items(CHR_SELF)
	yield
	remove_chr(CHR_SELF)

	// Still on screen
	label(0x13)
	restart_timer
	goto_first(0x71)

	label(0x72)
	say_quip(CHR_BOND, QUIP_SURRENDER, 0x23, 0x0a, 0x00, BANK_0, 0x00, 0x00)
	restart_timer
	goto_first(0x71)

	/***************************************************************************
	 * Warn friends
	 **************************************************************************/

	label(LABEL_WARNFRIENDS)
	unset_self_flag_bankx(CHRFLAG0_CAN_RETREAT, BANK_0)
	unset_self_flag_bankx(CHRFLAG0_CANT_ALERT_GROUP, BANK_0)
	try_run_to_chr(CHR_PRESET, /*goto*/ 0x96)

	beginloop(0x96)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_chr_death_animation_finished(CHR_PRESET, /*goto*/ 0x84)
		if_chr_knockedout(CHR_PRESET, /*goto*/ 0x84)
		if_detected_chr(CHR_PRESET, /*goto*/ 0x97)
		if_chr_stopped(/*goto*/ 0x97)
	endloop(0x96)

	label(0x97)
	say_quip(CHR_BOND, QUIP_WARNFRIENDS, 0xff, 0x02, 0xff, BANK_0, 0x00, 0x00)
	increase_squadron_alertness(100)

	label(0x84)
	goto_first(LABEL_MAINLOOP)

	label(0x88)
	terminate

	/***************************************************************************
	 * Run for alarm
	 **************************************************************************/

	label(LABEL_RUN_FOR_ALARM)
	set_action(MA_GOTOALARM, FALSE)
	unset_self_flag_bankx(CHRFLAG1_FLANKED, BANK_1)
	unset_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	say_quip(CHR_BOND, QUIP_GOFORALARM, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	go_to_target_pad(GOPOSFLAG_JOG)

	beginloop(0x9d)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0x9d)

	label(0x16)
	chr_do_animation(ANIM_PUSH_BUTTON, 0, 193, CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	restart_timer

	beginloop(0xed)
		if_timer_gt(70, /*goto*/ 0x16)
	endloop(0xed)

	label(0x16)
	activate_alarm

	beginloop(0x9e)
		if_dangerous_object_nearby(3, /*goto*/ LABEL_FLEE_GRENADE)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0x9e)

	label(0x16)
	goto_first(LABEL_MAINLOOP)

	/***************************************************************************
	 * Flee grenade
	 **************************************************************************/

	label(LABEL_FLEE_GRENADE)
	set_returnlist(CHR_SELF, GAILIST_ALERTED)
	set_ailist(CHR_SELF, GAILIST_FLEE_FROM_GRENADE)

	/***************************************************************************
	 * Target is cloaked
	 **************************************************************************/

	label(LABEL_TARGETCLOAKED)
	set_alertness(0)
	set_returnlist(CHR_SELF, GAILIST_ALERTED)
	set_ailist(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER)

	/***************************************************************************
	 * Target is gone (disguised on Rescue or just warped)
	 **************************************************************************/

	label(LABEL_TARGETGONE)
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_00002000)
	stop_chr
	if_chr_deadish(CHR_SELF, /*goto*/ 0x0a)
	if_can_see_target(/*goto*/ 0x13)
	set_alertness(0)
	set_self_flag_bankx(CHRFLAG1_SEARCHSAMEROOM, BANK_1)
	set_returnlist(CHR_SELF, GAILIST_UNALERTED)
	set_ailist(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_ALERTED)

	// Unreachable - nothing jumps to here
	label(0x0a)
	terminate
	endlist
};

/**
 * @ailist GAILIST_BUSY
 *
 * Do one animation of operating equipment, typing etc.
 */
u8 func000a_do_busy_animation[] = {
	set_chr_chrflag(CHR_SELF, CHRCFLAG_CANFACEWALL)
	if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_TRIGGER_BUDDY_WARP, /*goto*/ 0x13)
	if_chr_idle_action_eq(IDLEACTION_SITTING_TYPING, /*goto*/ 0x16)
	if_chr_idle_action_eq(IDLEACTION_SITTING_DORMANT, /*goto*/ 0x16)
	goto_next(0x13)

	// Sitting
	label(0x16)
	set_chr_maxdamage(CHR_SELF, 1)
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_TRIGGER_BUDDY_WARP)

	label(0x13)
	set_shotlist(GAILIST_UNALERTED)

	label(0x00)
	if_chr_idle_action_eq(IDLEACTION_STANDING, /*goto*/ 0x03)
	if_chr_idle_action_eq(IDLEACTION_SITTING_TYPING, /*goto*/ 0x04)
	if_chr_idle_action_eq(IDLEACTION_SITTING_DORMANT, /*goto*/ 0x05)
	if_chr_idle_action_eq(IDLEACTION_OPERATING, /*goto*/ 0x06)
	if_chr_idle_action_eq(IDLEACTION_OPERATING_PAD, /*goto*/ 0x07)
	stop_chr
	return
	terminate

	// Standing and operating something
	label(0x03)
	chr_do_animation(ANIM_OPERATE_0204, 0, -1, CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x0c)

	// Sitting and typing
	label(0x04)
	set_chr_special_death_animation(CHR_SELF, SPECIALDIE_ONCHAIR)
	chr_do_animation(ANIM_SITTING_TYPING, 0, -1, CHRANIMFLAG_PAUSEATEND | CHRANIMFLAG_SLOWUPDATE, 0, CHR_SELF, 2)
	goto_next(0x0c)

	// Sitting dormantly
	label(0x05)
	set_chr_special_death_animation(CHR_SELF, SPECIALDIE_ONCHAIR)
	chr_do_animation(ANIM_SITTING_DORMANT, 0, -1, CHRANIMFLAG_PAUSEATEND | CHRANIMFLAG_SLOWUPDATE, 0, CHR_SELF, 2)
	goto_next(0x0c)

	// Operating something with a pad
	label(0x07)
	restart_timer
	try_face_entity(ATTACKFLAG_AIMATPAD, PAD_PRESET, /*goto*/ 0xfa)

	beginloop(0xfa)
		if_timer_gt(60, /*goto*/ 0x06)
	endloop(0xfa)

	// Operating something without a pad
	label(0x06)
	call_rng
	if_rand_gt(85, /*goto*/ 0x28)
	if_rand_gt(170, /*goto*/ 0x29)
	chr_do_animation(ANIM_OPERATE_0221, 0, -1, CHRANIMFLAG_SLOWUPDATE, 0, CHR_SELF, 2)
	goto_next(0x0c)

	label(0x28)
	chr_do_animation(ANIM_OPERATE_0222, 0, -1, CHRANIMFLAG_SLOWUPDATE, 0, CHR_SELF, 2)
	goto_next(0x0c)

	label(0x29)
	chr_do_animation(ANIM_OPERATE_0223, 0, -1, CHRANIMFLAG_SLOWUPDATE, 0, CHR_SELF, 2)
	goto_next(0x0c)

	label(0x0c)
	return

	// Nothing jumps to here or below
	beginloop(0x15)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_target_in_sight(/*goto*/ 0x13)
		if_self_flag_bankx_eq(CHRFLAG0_AIVSAI, FALSE, BANK_0, /*goto*/ 0x16)
		if_enemy_distance_lt_and_los(2540, /*goto*/ 0x13)
		goto_next(0x16)

		label(0x13)
		goto_next(0x12)

		label(0x16)
	endloop(0x15)

	// Stand up
	label(0x13)
	chr_do_animation(ANIM_STAND_UP_FROM_SITTING, 0, -1, CHRANIMFLAG_MOVEWHENINVIS, 16, CHR_SELF, 2)
	object_do_animation(ANIM_025A, 0xff, 0x02, 0xffff)

	beginloop(0x06)
		if_chr_stopped(/*goto*/ 0x13)
	endloop(0x06)

	label(0x13)
	return

	label(0x12)
	set_returnlist(CHR_SELF, GAILIST_UNALERTED)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)
	endlist
};

/**
 * @ailist GAILIST_CHOOSE_TARGET
 *
 * Attempts to set the target chr and engage in combat with them.
 *
 * - If current chr has been shot, say something, raise group alertness, then
 * engage combat with the target (if in range and line of sight).
 * - Otherwise, attempt to find the target using a one of two methods,
 * determined by a bitflag on the chr:
 *
 * 1) Find chr by group (details unknown)
 * 2) Distance and line of sight check on all chrs
 *
 * When one fails the other is attempted. Presumably both of these are quite
 * expensive, so the flag exists to determine which one is more likely to pass
 * on a per chr basis.
 *
 * If both fail, the function yields and continues checking.
 *
 * This is used only for spawned guards in Escape in the first spawn group.
 */
u8 func000b_choose_target_chr[] = {
	set_shotlist(GAILIST_CHOOSE_TARGET)
	if_num_times_shot_lt(1, /*goto*/ 0xd3)

	// Has been shot
	say_quip(CHR_BOND, QUIP_SHOTUNALERT, 0x78, 0x03, 0x00, BANK_0, 0x00, 0x00)

	beginloop(0x03)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0x03)

	label(0x16)
	say_quip(CHR_BOND, QUIP_SHOTUNALERT, 0xff, 0x03, 0x00, BANK_0, 0x00, 0x00)
	increase_squadron_alertness(100)
	if_enemy_distance_lt_and_los(2540, /*goto*/ 0x13)
	goto_next(0xd3)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)

	// Hasn't been shot, or can't engage target chr
	label(0xd3)
		set_action(MA_NORMAL, FALSE)
		yield

		if_self_flag_bankx_eq(CHRFLAG0_NOHEAR, TRUE, BANK_0, /*goto*/ 0x13)
		set_target_to_enemy_on_same_floor(/*goto*/ 0x16)
		goto_next(0x04)

		// No hear - only see
		label(0x13)
		if_enemy_distance_lt_and_los(2540, /*goto*/ 0x16)

		// Scan failed
		label(0x04)
		if_saw_injury(0x00, /*goto*/ 0x13)
		if_alertness(99, OPERATOR_GREATER_THAN, /*goto*/ 0x13)
	goto_first(0xd3)

	// Found
	label(0x16)
	increase_squadron_alertness(100)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)

	// Unreachable
	set_returnlist(CHR_SELF, GAILIST_CHOOSE_TARGET)

	label(0x13)
	if_enemy_distance_lt_and_los(2540, /*goto*/ 0x13)
	if_self_flag_bankx_eq(CHRFLAG0_NOHEAR, TRUE, BANK_0, /*goto*/ 0x16)
	set_target_to_enemy_on_same_floor(/*goto*/ 0x13)

	label(0x16)
	goto_first(0xd3)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)
	endlist
};

/**
 * @ailist GAILIST_INIT_COMBAT
 */
u8 func000d_init_combat[] = {
	set_shotlist(GAILIST_COMBAT_WITH_TARGET)

	// If not idle, turn off special death animation
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, FALSE, BANK_1, /*goto*/ 0x13)
	set_chr_special_death_animation(CHR_SELF, SPECIALDIE_NONE)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, FALSE, BANK_1, /*goto*/ 0x16)
	unset_self_flag_bankx(CHRFLAG1_DOINGIDLEANIMATION, BANK_1)
	if_chr_idle_action_eq(IDLEACTION_SITTING_TYPING, /*goto*/ 0x05)
	if_chr_idle_action_eq(IDLEACTION_SITTING_DORMANT, /*goto*/ 0x05)
	goto_next(0x16)

	// Stand up
	label(0x05)
	set_chr_maxdamage(CHR_SELF, 40)
	chr_do_animation(ANIM_STAND_UP_FROM_SITTING, 0, -1, CHRANIMFLAG_MOVEWHENINVIS, 16, CHR_SELF, 2)
	object_do_animation(ANIM_025A, 0xff, 0x02, 0xffff)

	beginloop(0x0c)
		if_chr_stopped(/*goto*/ 0x13)
	endloop(0x0c)

	label(0x13)
	label(0x16)
	set_shotlist(GAILIST_COMBAT_WITH_TARGET)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)
	endlist
};

/**
 * @ailist GAILIST_COMBAT_WITH_TARGET
 */
u8 func000c_combat_with_target_chr[] = {
	label(0xbb)
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0x16)
	if_num_human_players_lt(2, /*goto*/ 0x16)

	label(0x13)
	if_target_is_player(/*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_WAR, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_DEEPSEA, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_DEFENSE, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_WAR, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_ESCAPE, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_ATTACKSHIP, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_CRASHSITE, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	// Solo mode, or co-op/counter-op on any other stage
	label(0x13)
	if_target_is_player(/*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_ALERTED)

	// Co-op with friend or counter-op on any of the above stages
	// Or follow through if chr still has target
	label(0x16)
	set_self_chrflag(CHRCFLAG_00000040)
	if_chr_deadish(CHR_SELF, /*goto*/ 0x16)
	goto_next(0x13)

	// Dying
	label(0x16)
	set_shotlist(GAILIST_IDLE)
	restart_timer

	beginloop(0x1c)
		if_timer_gt(15, /*goto*/ 0x16)
	endloop(0x1c)

	label(0x16)
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0x16)
	say_quip(CHR_BOND, QUIP_DIE, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	goto_next(0x17)

	label(0x16)
	say_quip(CHR_BOND, QUIP_GOTOCOVER1, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)

	label(0x17)
	set_shotlist(GAILIST_IDLE)
	terminate

	// Alive
	label(0x13)
	if_num_times_shot_lt(1, /*goto*/ 0xb2)
	if_just_injured(CHR_SELF, /*goto*/ 0x16)
	if_has_gun(CHR_SELF, /*goto*/ 0xb2)
	set_self_flag_bankx(CHRFLAG1_HANDCOMBATONLY, BANK_1)
	goto_next(0xb2)

	// Injured
	label(0x16)
	restart_timer

	beginloop(0x1d)
		if_self_flag_bankx_eq(CHRFLAG0_SAID_INJURY_QUIP, TRUE, BANK_0, /*goto*/ 0x16)
		if_num_times_shot_lt(1, /*goto*/ 0x16)
		if_timer_lt(20, /*goto*/ 0x16)
		say_quip(CHR_BOND, QUIP_INJURED1, 0x28, 0x03, 0xff, BANK_0, 0x00, 0x00)
		say_quip(CHR_BOND, QUIP_INJURED2, 0x28, 0x03, 0x01, BANK_0, 0x00, 0x00)
		set_self_flag_bankx(CHRFLAG0_SAID_INJURY_QUIP, BANK_0)

		label(0x16)
		if_chr_stopped(/*goto*/ 0x13)
		if_timer_gt(180, /*goto*/ 0x13)
	endloop(0x1d)

	label(0x13)

	// Chrs can have CHRHFLAG_DONTSHOOTME, which I think is used for coop.
	// If an enemy has this flag, the AI buddy will not shoot the chr but will
	// engage in hand combat. It's mainly used on guards where killing them
	// fails an objective but knocking them out is OK. It's also used on the G5
	// guard between the two cloak rooms, because having the buddy fire shots in
	// this area will alert the guards later in the stage which is undesirable.
	label(0xb2)
	yield
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DONTSHOOTME, /*goto*/ 0x13)
	unset_self_flag_bankx(CHRFLAG1_HANDCOMBATONLY, BANK_1)
	goto_next(0x16)

	label(0x13)
	set_self_flag_bankx(CHRFLAG1_HANDCOMBATONLY, BANK_1)

	label(0x16)
	if_alertness(100, OPERATOR_LESS_THAN, /*goto*/ 0xb3)
	if_enemy_distance_lt_and_los(2540, /*goto*/ 0xb3)
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0x16)
	if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_PSYCHOSISED, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_chrpresets_target_is_different(/*goto*/ 0xb3)

	label(0x16)
	set_target_chr(-1)
	return

	label(0xb3)
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0x16)

	label(0x13)
	if_target_is_player(/*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_WAR, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_DEEPSEA, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_DEFENSE, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_WAR, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_ESCAPE, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_ATTACKSHIP, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	if_stage_is_not(STAGE_CRASHSITE, /*goto*/ 0x13)
	if_num_human_players_lt(2, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_ALERTED)

	label(0x16)
	restart_timer
	if_chr_deadish(CHR_TARGET, /*goto*/ 0xba)
	if_chrpresets_target_is_different(/*goto*/ 0x13)
	goto_next(0x16)

	// Target's field 0x17e is different
	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, FALSE, BANK_1, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG1_NOHANDCOMBAT, TRUE, BANK_1, /*goto*/ 0x16)
	call_rng
	if_rand_lt(128, /*goto*/ 0x13)
	if_distance_to_target_gt(250, /*goto*/ 0x16)
	if_can_see_attack_target(/*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_self_flag_bankx(CHRFLAG1_PUNCH_THEN_GENERAL_COMBAT, BANK_1)
	set_ailist(CHR_SELF, GAILIST_HAND_COMBAT)

	label(0x16)
	try_run_to_target(/*goto*/ 0xb4)

	beginloop(0xb4)
		if_timer_gt(600, /*goto*/ 0xba)
		if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, FALSE, BANK_1, /*goto*/ 0x13)
		if_timer_gt(20, /*goto*/ 0x09)

		label(0x13)
		if_chr_deadish(CHR_TARGET, /*goto*/ 0xba)
		if_chrpresets_target_is_different(/*goto*/ 0x13)
		goto_next(0xba)

		label(0x13)
		if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x16)
		if_self_flag_bankx_eq(CHRFLAG1_NOHANDCOMBAT, TRUE, BANK_1, /*goto*/ 0x13)
		if_distance_to_target_lt(120, /*goto*/ 0x16)
		call_rng
		if_rand_lt(128, /*goto*/ 0x13)
		if_distance_to_target_gt(250, /*goto*/ 0x13)

		label(0x16)
		if_can_see_attack_target(/*goto*/ 0x16)
		goto_next(0x13)

		label(0x16)
		set_self_flag_bankx(CHRFLAG1_PUNCH_THEN_GENERAL_COMBAT, BANK_1)
		set_ailist(CHR_SELF, GAILIST_HAND_COMBAT)

		label(0x13)
		label(0x13)
		if_enemy_distance_lt_and_los(1200, /*goto*/ 0xb5)
		if_can_see_attack_target(/*goto*/ 0xb6)
	endloop(0xb4)

	label(0x09)
	goto_first(0xb3)

	label(0xb7)
	label(0xb9)
	if_can_see_attack_target(/*goto*/ 0xb6)
	if_chr_deadish(CHR_TARGET, /*goto*/ 0xba)
	if_chrpresets_target_is_different(/*goto*/ 0x13)
	goto_next(0xba)

	label(0x13)
	if_chr_in_view(/*goto*/ 0xbb)
	goto_first(0xb4)

	label(0xbb)
	goto_first(0xbb)

	label(0xb5)
	restart_timer
	try_run_to_target(/*goto*/ 0xbc)

	beginloop(0xbc)
		if_chr_deadish(CHR_TARGET, /*goto*/ 0xc0)
		if_chrpresets_target_is_different(/*goto*/ 0x13)
		goto_next(0xc0)

		label(0x13)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ 0x13)
		if_can_see_attack_target(/*goto*/ 0xbf)

		label(0x13)
		if_timer_gt(120, /*goto*/ 0xc0)
	endloop(0xbc)

	label(0xbf)
	goto_next(0xb6)

	label(0xc0)
	goto_first(0xb2)

	label(0xb6)
	set_grenade_probability_out_of_255(255)
	set_self_chrflag(CHRCFLAG_00000040)
	if_chr_deadish(CHR_TARGET, /*goto*/ 0xc1)
	if_chrpresets_target_is_different(/*goto*/ 0x13)
	goto_next(0xc1)

	label(0x13)
	goto_next(0xc2)

	label(0x16)
	if_distance_to_target_lt(800, /*goto*/ 0xc2)

	// @bug: Guards will consider throwing grenades if you're moving quickly
	// rather than slowly. The below statement skips the grenade logic if slow.
	if_target_moving_slowly(0, /*goto*/ 0xc2)
	restart_timer
	if_chr_in_squadron_doing_action(MA_GRENADE, /*goto*/ 0xc2)
	set_action(MA_GRENADE, FALSE)
	consider_throwing_grenade(0x0200, 0x0000, /*goto*/ 0x16)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG1_NOHANDCOMBAT, TRUE, BANK_1, /*goto*/ 0x13)
	if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x16)
	if_distance_to_target_lt(120, /*goto*/ 0x16)
	call_rng
	if_rand_lt(128, /*goto*/ 0x13)
	if_distance_to_target_gt(250, /*goto*/ 0x13)

	label(0x16)
	if_can_see_attack_target(/*goto*/ 0x16)
	goto_next(0x13)

	label(0x16)
	set_self_flag_bankx(CHRFLAG1_PUNCH_THEN_GENERAL_COMBAT, BANK_1)
	set_ailist(CHR_SELF, GAILIST_HAND_COMBAT)

	label(0x13)
	yield

	label(0xc2)
	if_self_flag_bankx_eq(CHRFLAG1_NOHANDCOMBAT, TRUE, BANK_1, /*goto*/ 0x13)
	if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x16)
	if_distance_to_target_lt(120, /*goto*/ 0x16)
	call_rng
	if_rand_lt(128, /*goto*/ 0x13)
	if_distance_to_target_gt(250, /*goto*/ 0x13)

	label(0x16)
	if_can_see_attack_target(/*goto*/ 0x16)
	goto_next(0x13)

	label(0x16)
	set_self_flag_bankx(CHRFLAG1_PUNCH_THEN_GENERAL_COMBAT, BANK_1)
	set_ailist(CHR_SELF, GAILIST_HAND_COMBAT)

	label(0x13)
	label(0x13)
	call_rng
	if_rand_gt(64, /*goto*/ 0x16)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_ATTACKAMOUNTRELOAD, FALSE, BANK_1, /*goto*/ 0x13)
	try_attack_amount(90, 100)
	set_self_flag_bankx(CHRFLAG1_CAN_RELOAD, BANK_1)
	goto_next(0xc3)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_CAN_ATTACKAMOUNT, FALSE, BANK_1, /*goto*/ 0x13)
	try_attack_amount(20, 30)
	goto_next(0xc3)

	label(0x13)
	label(0x16)
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0x17)
	goto_next(0x16)

	label(0x17)
	say_quip(CHR_BOND, QUIP_ATTACK2, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)

	label(0x16)
	call_rng
	if_rand_gt(50, /*goto*/ 0x13)
	call_rng
	if_rand_gt(128, /*goto*/ 0x16)

	label(0xbe)
	try_attack_roll(/*goto*/ 0xc3)

	label(0x16)
	try_attack_walk(/*goto*/ 0xc3)

	label(0x13)
	call_rng
	if_rand_gt(128, /*goto*/ 0x8c)
	try_attack_kneel(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x16)

	label(0x8c)
	try_attack_stand(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x16)

	label(0x16)
	try_modify_attack(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0xc3)

	label(0xc3)
	say_quip(CHR_BOND, QUIP_ATTACK3, 0x19, 0x02, 0xff, BANK_0, 0x00, 0x00)
	say_quip(CHR_BOND, QUIP_ATTACK4, 0x19, 0x02, 0x01, BANK_0, 0x00, 0x00)

	beginloop(0xe2)
		if_chr_deadish(CHR_TARGET, /*goto*/ 0xc1)
		if_chrpresets_target_is_different(/*goto*/ 0x16)
		goto_next(0xc1)

		label(0x16)
		if_chr_stopped(/*goto*/ 0xc1)
	endloop(0xe2)

	label(0x13)
	restart_timer

	beginloop(0x03)
		if_timer_gt(60, /*goto*/ 0x16)
	endloop(0x03)

	label(0x16)
	goto_first(0xc3)

	label(0xba)

	label(0xc1)
	if_chr_injured_target(CHR_SELF, /*goto*/ 0x13)
	say_quip(CHR_BOND, QUIP_MISSEDPLAYER1, 0x19, 0x03, 0xff, BANK_0, 0x00, 0x00)
	say_quip(CHR_BOND, QUIP_MISSEDPLAYER2, 0x19, 0x03, 0x01, BANK_0, 0x00, 0x00)
	goto_next(0x16)

	label(0x13)
	say_quip(CHR_BOND, QUIP_HITPLAYER, 0x19, 0x02, 0xff, BANK_0, 0x00, 0x00)
	say_quip(CHR_BOND, QUIP_HITPLAYER, 0x19, 0x03, 0x01, BANK_0, 0x00, 0x00)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG1_PUNCHAIEXTRADAMAGE, FALSE, BANK_1, /*goto*/ 0x13)
	if_target_is_player(/*goto*/ 0x13)
	damage_chr(CHR_TARGET, WEAPON_SUPERDRAGON)
	unset_self_flag_bankx(CHRFLAG1_PUNCHAIEXTRADAMAGE, BANK_1)

	label(0x13)
	if_chr_deadish(CHR_TARGET, /*goto*/ 0xc5)
	if_chrpresets_target_is_different(/*goto*/ 0x13)
	goto_next(0xc5)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_IGNORECOVER, TRUE, BANK_1, /*goto*/ 0xc5)

	label(0xb8)
	label(0xc6)
	if_self_flag_bankx_eq(CHRFLAG0_COVER_TYPE2, FALSE, BANK_0, /*goto*/ 0x13)
	goto_first(0xb3)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG1_ALLOWSOFTCOVER, TRUE, BANK_1, /*goto*/ 0x15)
	find_cover(COVERCRITERIA_0001 | COVERCRITERIA_DISTTOME | COVERCRITERIA_FORCENEWCOVER | COVERCRITERIA_0020 | COVERCRITERIA_1000, /*goto*/ 0x53)
	find_cover(COVERCRITERIA_0001 | COVERCRITERIA_DISTTOME | COVERCRITERIA_FORCENEWCOVER | COVERCRITERIA_ALLOWNEIGHBOURINGROOMS | COVERCRITERIA_1000, /*goto*/ 0x53)
	goto_next(0x16)

	label(0x15)
	find_cover(COVERCRITERIA_0001 | COVERCRITERIA_DISTTOME | COVERCRITERIA_FORCENEWCOVER | COVERCRITERIA_0020 | COVERCRITERIA_1000 | COVERCRITERIA_ALLOWSOFT, /*goto*/ 0x53)
	find_cover(COVERCRITERIA_0001 | COVERCRITERIA_DISTTOME | COVERCRITERIA_FORCENEWCOVER | COVERCRITERIA_ALLOWNEIGHBOURINGROOMS | COVERCRITERIA_1000 | COVERCRITERIA_ALLOWSOFT, /*goto*/ 0x53)

	label(0x16)
	goto_next(0x54)

	label(0x53)
	say_quip(CHR_BOND, QUIP_GOTOCOVER1, 0xff, 0x02, 0x01, BANK_0, 0x00, 0x00)
	go_to_cover(GOPOSFLAG_RUN)
	restart_timer

	beginloop(0xc7)
		if_chr_deadish(CHR_TARGET, /*goto*/ 0xc5)
		if_enemy_distance_lt_and_los(400, /*goto*/ 0x13)
		if_chr_stopped(/*goto*/ 0x16)
		if_timer_gt(360, /*goto*/ 0x16)
	endloop(0xc7)

	label(0x13)
	goto_first(0xb5)

	label(0x16)

	label(0xc8)
	restart_timer
	try_attack_kneel(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0xc9)

	beginloop(0xc9)
		if_chr_deadish(CHR_TARGET, /*goto*/ 0xc5)
		if_chrpresets_target_is_different(/*goto*/ 0x13)
		goto_next(0xc5)

		label(0x13)
		if_enemy_distance_lt_and_los(400, /*goto*/ 0x15)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_CLOAKED, /*goto*/ 0x13)
		if_can_see_attack_target(/*goto*/ 0xca)

		label(0x13)
		if_timer_gt(200, /*goto*/ 0x16)
	endloop(0xc9)

	label(0x16)
	goto_first(0xb3)

	label(0x15)
	goto_first(0xb5)

	label(0xa3)
	goto_first(0xc1)

	label(0xca)
	restart_timer
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0x16)
	goto_next(0x17)

	label(0x16)
	say_quip(CHR_BOND, QUIP_ATTACK2, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)

	label(0x17)
	try_modify_attack(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0xcb)
	goto_next(0x16)

	label(0xcb)
	say_quip(CHR_BOND, QUIP_ATTACK3, 0x19, 0x02, 0xff, BANK_0, 0x00, 0x00)
	say_quip(CHR_BOND, QUIP_ATTACK4, 0x19, 0x02, 0x01, BANK_0, 0x00, 0x00)

	beginloop(0xcc)
		if_chr_deadish(CHR_TARGET, /*goto*/ 0xc5)
		if_chrpresets_target_is_different(/*goto*/ 0x13)
		goto_next(0xc5)

		label(0x13)
		if_timer_gt(300, /*goto*/ 0x16)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0xcc)

	label(0x54)
	goto_first(0xb3)

	restart_timer
	try_run_sideways(/*goto*/ 0xcd)

	beginloop(0xcd)
		if_timer_gt(300, /*goto*/ 0xce)
		if_chr_stopped(/*goto*/ 0xce)
	endloop(0xcd)

	label(0xce)
	goto_first(0xb3)

	label(0xc5)
	if_self_flag_bankx_eq(CHRFLAG1_AIVSAI_ADVANTAGED, FALSE, BANK_1, /*goto*/ 0x13)
	return

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG0_NOHEAR, TRUE, BANK_0, /*goto*/ 0x13)

	label(0x13)
	goto_next(0xcf)

	label(0x16)
	restart_timer
	call_rng
	if_self_flag_bankx_eq(CHRFLAG1_AIVSAI_ADVANTAGED, FALSE, BANK_1, /*goto*/ 0x13)
	return

	label(0x13)
	goto_next(0xd0)

	// Unreachable - no label
	if_rand_gt(128, /*goto*/ 0xd1)
	try_jog_to_target(/*goto*/ 0xd0)

	beginloop(0xd0)
		if_timer_gt(120, /*goto*/ 0xd2)
		if_chr_stopped(/*goto*/ 0xd2)
	endloop(0xd0)

	label(0xd2)
	goto_next(0xc4)

	// Begin unreachable
	kneel

	beginloop(0xd4)
		if_timer_gt(60, /*goto*/ 0xc4)
	endloop(0xd4)

	label(0xd1)
	try_run_sideways(/*goto*/ 0xd5)

	beginloop(0xd5)
		if_timer_gt(180, /*goto*/ 0xc4)
		if_chr_stopped(/*goto*/ 0xc4)
	endloop(0xd5)
	// End unreachable

	label(0xc4)
	if_self_flag_bankx_eq(CHRFLAG0_CAMP, FALSE, BANK_0, /*goto*/ 0x13)
	return

	label(0x13)
	goto_first(0xb3)

	label(0xcf)
	if_self_flag_bankx_eq(CHRFLAG0_CAMP, FALSE, BANK_0, /*goto*/ 0x13)
	if_target_is_player(/*goto*/ 0x16)
	goto_next(0x13)

	label(0x16)
	return

	label(0x13)
	try_walk_to_target(/*goto*/ 0xd6)

	beginloop(0xd6)
		if_enemy_distance_lt_and_los(1200, /*goto*/ 0x13)
		if_chr_stopped(/*goto*/ 0xd7)
	endloop(0xd6)

	label(0x13)
	goto_first(0xb3)

	label(0xd7)
	say_quip(CHR_BOND, QUIP_KILLEDPLAYER1, 0x28, 0x00, 0x01, BANK_0, 0x00, 0x00)
	try_face_entity(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0x16)

	label(0x16)
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0xd9)
	chr_do_animation(ANIM_YAWN, 0, 193, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)

	beginloop(0xd8)
		if_chr_stopped(/*goto*/ 0xd9)
	endloop(0xd8)

	label(0xd9)
	return
	endlist
};

/**
 * @ailist GAILIST_IDLE_0009
 *
 * This is the initial function for Defense blondes and some chr in
 * Attack Ship.
 */
u8 func0000_idle_0009[] = {
	set_action(MA_NORMAL, FALSE)
	set_returnlist(CHR_SELF, GAILIST_IDLE_0009)
	stop_chr
	terminate
	endlist
};

/**
 * @ailist GAILIST_SHOW_OBJ_FAILED_MSG
 *
 * Waits for an objective to fail, then shows the objectives failed messages
 * periodically.
 */
u8 func0016_show_objective_failed_msg[] = {
	// Wait until an objective has failed
	beginloop(0x0c)
		if_any_objective_failed(/*goto*/ 0x16)
	endloop(0x0c)

	// Wait 30 seconds
	label(0x16)
	restart_timer

	beginloop(0x0d)
		if_timer_gt(1800, /*goto*/ 0x13)
	endloop(0x0d)

	// Check objective is still failed
	label(0x13)
	if_any_objective_failed(/*goto*/ 0x13)
	terminate

	// Show message first time
	label(0x13)
	show_hudmsg(CHR_BOND, L_MISC_069) // "OBJECTIVES FAILED - abort mission."

	// Wait 60 seconds
	restart_timer

	beginloop(0x03)
		if_timer_gt(3600, /*goto*/ 0x16)
	endloop(0x03)

	// Check objective is still failed
	label(0x16)
	if_any_objective_failed(/*goto*/ 0x13)
	terminate

	// Show message second time
	label(0x13)
	show_hudmsg(CHR_BOND, L_MISC_069) // "OBJECTIVES FAILED - abort mission."
	restart_timer

	// Wait 2 minutes
	beginloop(0x04)
		if_timer_gt(7200, /*goto*/ 0x06)
	endloop(0x04)

	// Check objective is still failed
	label(0x06)
	if_any_objective_failed(/*goto*/ 0x13)
	terminate

	// Show message third time
	label(0x13)
	show_hudmsg(CHR_BOND, L_MISC_069) // "OBJECTIVES FAILED - abort mission."
	restart_timer

	// Wait 5 minutes
	beginloop(0x05)
		if_timer_gt(18000, /*goto*/ 0x16)
	endloop(0x05)

	// Loop back to the last message, so it will display every 5 minutes
	label(0x16)
	goto_first(0x06)

	endlist
};

/**
 * @ailist GAILIST_REBUILD_GROUPS
 */
u8 func0017_rebuild_groups[] = {
	rebuild_teams
	rebuild_squadrons
	terminate
	endlist
};

/**
 * @ailist GAILIST_DO_BORED_ANIMATION
 */
u8 func0018_do_bored_animation[] = {
	call_rng
	if_rand_gt(50, /*goto*/ 0x13)
	chr_do_animation(ANIM_YAWN, 0, 193, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x13)
	if_rand_gt(100, /*goto*/ 0x13)
	chr_do_animation(ANIM_SCRATCH_HEAD, 0, 294, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x13)
	if_rand_gt(150, /*goto*/ 0x13)
	chr_do_animation(ANIM_ROLL_HEAD, 0, 183, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x13)
	if_rand_gt(200, /*goto*/ 0x13)
	chr_do_animation(ANIM_GRAB_CROTCH, 0, 123, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x13)
	if_rand_gt(250, /*goto*/ 0x13)
	chr_do_animation(ANIM_GRAB_BUTT, 0, 56, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x13)
	label(0x16)
	return
	endlist
};

/**
 * @ailist GAILIST_LOOK_AROUND
 */
u8 func001e_look_around[] = {
	call_rng
	if_rand_gt(50, /*goto*/ 0x13)
	chr_do_animation(ANIM_LOOK_AROUND_025B, 0, 193, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x13)
	if_rand_gt(100, /*goto*/ 0x13)
	chr_do_animation(ANIM_LOOK_AROUND_025C, 0, 294, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x13)
	if_rand_gt(150, /*goto*/ 0x13)
	chr_do_animation(ANIM_LOOK_AROUND_025D, 0, 183, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	// @bug: Shows grab crotch animation when wanting to look around instead.
	// Probably a copy/paste error from previous function.
	label(0x13)
	if_rand_gt(200, /*goto*/ 0x13)
	chr_do_animation(ANIM_GRAB_CROTCH, 0, 123, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x13)
	if_rand_gt(250, /*goto*/ 0x13)
	chr_do_animation(ANIM_LOOK_AROUND_025E, 0, 56, CHRANIMFLAG_COMPLETED | CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x13)
	label(0x16)
	return
	endlist
};

/**
 * @ailist GAILIST_DO_SITTING_ANIMATION
 */
u8 func0019_do_sitting_animation[] = {
	call_rng
	if_rand_gt(128, /*goto*/ 0x13)
	chr_do_animation(ANIM_SITTING_TYPING, 0, -1, CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x13)
	chr_do_animation(ANIM_SITTING_DORMANT, 0, -1, CHRANIMFLAG_SLOWUPDATE, 16, CHR_SELF, 2)
	goto_next(0x16)

	label(0x13)
	label(0x16)
	return
	endlist
};

/**
 * @ailist GAILIST_PATROLLER_DIS_TALKING
 *
 * This function is assigned to patrollers when they are talking to a disguised
 * player. It puts them into combat mode or makes them resume their patrol after
 * their talk has finished.
 */
u8 func001a_patroller_dis_talking[] = {
	restart_timer

	beginloop(0x00)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DISGUISED, /*goto*/ 0x14)
		goto_next(0x13)

		// Target is disguised
		label(0x14)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_ALMOSTUNCOVERED, /*goto*/ 0xe8)
		goto_next(0x13)

		// Target is disguised and almost uncovered
		label(0xe8)
		if_chr_weapon_equipped(CHR_TARGET, WEAPON_NONE, /*goto*/ 0x13)
		if_chr_weapon_equipped(CHR_TARGET, WEAPON_UNARMED, /*goto*/ 0x13)
		if_chr_weapon_equipped(CHR_TARGET, WEAPON_COMBATBOOST, /*goto*/ 0x13)
		if_chr_weapon_equipped(CHR_TARGET, WEAPON_HORIZONSCANNER, /*goto*/ 0x13)
		if_chr_weapon_equipped(CHR_TARGET, WEAPON_SUITCASE, /*goto*/ 0x13)

		// Target is disguised, almost uncovered and has weapon equipped...
		// While it appears the patroller goes unalert here, it's likely that
		// something in the unalert ailist makes him alert immediately.
		set_ailist(CHR_SELF, GAILIST_UNALERTED)

		// Not disguised, or disguised good enough

		// @bug: I'd say the intention here is to wait 1 second, then wait for
		// the talk and anim to finish, then resume the patrol. But the idle
		// check is inverted, and the chr is surely not idle on the first check,
		// so they'll resume patrolling immediately.
		label(0x13)
		if_timer_lt(60, /*goto*/ 0x15)
		if_chr_not_talking(CHR_SELF, /*goto*/ 0x28)
		goto_next(0x15)

		// Not talking
		label(0x28)
		unset_self_flag_bankx(CHRFLAG1_TALKINGTODISGUISE, BANK_1)
		stop_chr

		// Not talking (follow through from above) or less than 1 second
		// Wait until no longer idle (@bug?) then resume patrol
		label(0x15)
		if_chr_idle(/*goto*/ 0x13)
		goto_next(0x16)

		label(0x13)
	endloop(0x00)

	// Resume patrol
	label(0x16)
	start_patrol
	set_ailist(CHR_SELF, GAILIST_UNALERTED)
	endlist
};

#define LABEL_FACE_TARGET 0x16
#define LABEL_PUNCH       0x16

/**
 * @ailist GAILIST_HAND_COMBAT
 */
u8 func000f_hand_combat[] = {
#if VERSION >= VERSION_NTSC_1_0
	if_chr_deadish(CHR_SELF, /*goto*/ 0x13)
	goto_next(0x16)

	// Dying
	label(0x13)
	set_shotlist(GAILIST_IDLE)
	terminate

	label(0x16)
#endif

	set_action(MA_UNARMEDATTACK, FALSE)
	restart_timer

	// If current chr doesn't have this flag and isn't idle, stop them
	if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x0c)
#if VERSION >= VERSION_NTSC_1_0
	if_chr_idle(/*goto*/ 0x0c)
#endif
	stop_chr

	// Begin loop
	beginloop(0x0c)
		if_chr_deadish(CHR_TARGET, /*goto*/ 0xfa)

		// This if-statement is effectively a no op
		if_chrpresets_target_is_different(/*goto*/ 0x13)


		label(0x13)
		if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x13)
		if_chr_is_skedar(CHR_SELF, /*goto*/ 0x0f)
		if_distance_to_target_gt(120, /*goto*/ 0x13)
		goto_next(0x10)

		label(0x0f)
		if_distance_to_target_gt(150, /*goto*/ 0x13)

		label(0x10)
		if_can_see_attack_target(/*goto*/ 0x13)
		if_timer_gt(120, /*goto*/ 0x0b)
	endloop(0x0c)

	label(0x13)
	if_target_is_player(/*goto*/ 0x0a)
	if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x13)
	if_distance_to_target_gt(400, /*goto*/ 0x0b)

	label(0x13)
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0x0f)
	if_distance_to_target_lt(120, /*goto*/ LABEL_FACE_TARGET)
	goto_next(0x10)

	label(0x0f)
	if_distance_to_target_lt(150, /*goto*/ LABEL_FACE_TARGET)

	label(0x10)
	label(0xe3)
	restart_timer
	try_run_to_target(/*goto*/ 0x03)

	beginloop(0x03)
		if_chr_deadish(CHR_TARGET, /*goto*/ 0xfa)

		label(0x13)
		if_timer_gt(90, /*goto*/ 0x0b)
		if_target_is_player(/*goto*/ 0x09)
		if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x13)
		if_distance_to_target_gt(400, /*goto*/ 0x0b)

		label(0x13)
		if_chr_is_skedar(CHR_SELF, /*goto*/ 0x0f)
		if_distance_to_target_lt(120, /*goto*/ LABEL_FACE_TARGET)
		goto_next(0x10)

		label(0x0f)
		if_distance_to_target_lt(150, /*goto*/ LABEL_FACE_TARGET)

		label(0x10)
	endloop(0x03)

	label(0x09)
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0x0f)
	if_distance_to_target_lt(120, /*goto*/ LABEL_FACE_TARGET)
	goto_next(0x10)

	label(0x0f)
	if_distance_to_target_lt(150, /*goto*/ LABEL_FACE_TARGET)

	label(0x10)
	if_self_flag_bankx_eq(CHRFLAG1_HANDCOMBATONLY, TRUE, BANK_1, /*goto*/ 0x13)
	if_distance_to_target_gt(400, /*goto*/ 0x0b)

	label(0x13)
	goto_first(0x03)

	label(0x0a)
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0x0f)
	if_distance_to_target_lt(120, /*goto*/ LABEL_FACE_TARGET)
	goto_next(0x10)

	label(0x0f)
	if_distance_to_target_lt(150, /*goto*/ LABEL_FACE_TARGET)

	label(0x10)
	if_distance_to_target_gt(400, /*goto*/ 0x0b)
	goto_first(0xe3)

	label(0x13)
	restart_timer
	goto_first(0x0c)

	label(LABEL_FACE_TARGET)
	restart_timer
	if_target_in_fov_left(10, /*goto*/ LABEL_PUNCH)
	if_target_out_of_fov_left(246, /*goto*/ LABEL_PUNCH)
	stop_chr
	try_face_entity(ATTACKFLAG_AIMATTARGET, 1, /*goto*/ 0x04)

	beginloop(0x04)
		if_chr_deadish(CHR_TARGET, /*goto*/ 0xfa)

		label(0x13)
		if_distance_to_target_gt(400, /*goto*/ 0x0b)
		if_target_in_fov_left(15, /*goto*/ LABEL_PUNCH)
		if_target_out_of_fov_left(240, /*goto*/ LABEL_PUNCH)
		if_timer_gt(60, /*goto*/ LABEL_PUNCH)
	endloop(0x04)

	label(0x13)
	restart_timer
	goto_first(0x0c)

	label(LABEL_PUNCH)
	try_punch_or_kick(/*goto*/ 0x13)

	// Unable to punch or kick for some reason - return to main loop
	restart_timer
	goto_first(0x0c)

	label(0x13)
	add_morale(1)
	if_chr_is_skedar(CHR_SELF, /*goto*/ 0x17)
	goto_next(0x05)

	label(0x17)
	restart_timer
	say_quip(CHR_BOND, QUIP_ATTACK2, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)

	beginloop(0x05)
		if_timer_gt(180, /*goto*/ 0x16)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0x05)

	label(0x16)
	if_chr_injured_target(CHR_SELF, /*goto*/ 0x13)
	say_quip(CHR_BOND, QUIP_MISSEDPLAYER1, 0x19, 0x03, 0x00, BANK_0, 0x00, 0x00)
	say_quip(CHR_BOND, QUIP_MISSEDPLAYER2, 0x19, 0x03, 0x01, BANK_0, 0x00, 0x00)
	goto_next(0x16)

	label(0x13)
	say_quip(CHR_BOND, QUIP_HITPLAYER, 0x19, 0x02, 0x00, BANK_0, 0x00, 0x00)
	say_quip(CHR_BOND, QUIP_HITPLAYER, 0x19, 0x03, 0x01, BANK_0, 0x00, 0x00)

	label(0x16)
	goto_next(0x0b)

	label(0xfa)

	// Finished attack
	label(0x0b)

	label(0x15)
	if_self_flag_bankx_eq(CHRFLAG1_PUNCH_THEN_GENERAL_COMBAT, TRUE, BANK_1, /*goto*/ 0x13)

	label(0x17)
	return

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)
	endlist
};

/**
 * @ailist GAILIST_CIVILIAN_SAY_COMMENT
 *
 * Used in Chicago, Air Base, and some other global ailists.
 *
 * There is special logic for Chicago, where they don't care if you have a gun
 * and say different things.
 *
 * The chr will make one comment, then it returns to the previous function.
 */
u8 func0010_civilian_say_comment[] = {
	set_self_chrflag(CHRCFLAG_00040000)
	set_shotlist(GAILIST_SURPRISED)

	label(0x0c)
	yield

	label(0x16)
	restart_timer
	try_face_entity(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0x04)

	// Wait until facing target, or a second has passed, or something else
	// happens such as hearing gunfire or seeing someone die.
	beginloop(0x04)
		if_saw_injury(0x01, /*goto*/ 0x83)
		if_saw_death(0x01, /*goto*/ 0x83)
		if_target_in_fov_left(10, /*goto*/ 0x13)
		if_target_out_of_fov_left(246, /*goto*/ 0x13)
		if_timer_gt(60, /*goto*/ 0x13)
	endloop(0x04)

	// Facing target
	label(0x13)
	if_stage_is_not(STAGE_CHICAGO, /*goto*/ 0x16)
	goto_next(0x0e)

	// Not Chicago
	label(0x16)
	label(0x16)
	set_self_flag_bankx(CHRFLAG1_TALKINGTODISGUISE, BANK_1)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_NONE, /*goto*/ 0x07)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_UNARMED, /*goto*/ 0x07)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_COMBATBOOST, /*goto*/ 0x07)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_HORIZONSCANNER, /*goto*/ 0x07)
	if_chr_weapon_equipped(CHR_TARGET, WEAPON_SUITCASE, /*goto*/ 0x07)
	goto_next(0x05)

	// Player is unarmed
	label(0x07)
	restart_timer
	if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_ASKEDABOUTGUN, /*goto*/ 0x13)
	do_preset_animation(PRESETANIM_TALKRANDOM)
	say_quip(CHR_BOND, QUIP_GREETING, 0xff, 0x02, 0xff, BANK_0, 0x00, 0x00)
	yield

	// Set a do-once flag, so chr is only stopped the first time
	if_self_flag_bankx_eq(CHRFLAG1_TALKINGTODISGUISE, TRUE, BANK_1, /*goto*/ 0x13)
	stop_chr
	set_self_flag_bankx(CHRFLAG1_TALKINGTODISGUISE, BANK_1)

	label(0x13)
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_ASKEDABOUTGUN)
	return

	// Player is armed
	label(0x05)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_ASKEDABOUTGUN, /*goto*/ 0x06)
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_ASKEDABOUTGUN)
	say_quip(CHR_BOND, QUIP_ASKWEAPON1, 0xff, 0x02, 0xff, BANK_0, 0x00, 0x00)
	do_preset_animation(PRESETANIM_TALK6)
	restart_timer

	beginloop(0x08)
		if_saw_injury(0x01, /*goto*/ 0x83)
		if_saw_death(0x01, /*goto*/ 0x83)
		if_timer_gt(180, /*goto*/ 0x16)
	endloop(0x08)

	// Unreachable
	return

	// Has chrflag2_01000000
	label(0x06)
	if_chr_in_view(/*goto*/ 0x13)
	return

	label(0x13)
	set_chr_hiddenflag(CHR_TARGET, CHRHFLAG_ALMOSTUNCOVERED)
	say_quip(CHR_BOND, QUIP_ASKWEAPON2, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	do_preset_animation(PRESETANIM_TALKRANDOM)
	restart_timer

	// Wait 3 seconds, or for see/hear gunfire
	beginloop(0x09)
		if_saw_injury(0x01, /*goto*/ 0x83)
		if_saw_death(0x01, /*goto*/ 0x83)
		if_timer_gt(180, /*goto*/ 0x16)
	endloop(0x09)

	// Timer expired
	label(0x16)
	return

	label(0x15)

	// Special Chicago logic
	label(0x0e)
	call_rng
	if_rand_gt(128, /*goto*/ 0x16)
	if_stage_is_not(STAGE_CHICAGO, /*goto*/ 0x15) // will never pass

	// 50% chance of this happening
	say_quip(CHR_BOND, 0x16, 0xff, 0x00, 0xff, 0x81, 0x00, 0x00)
	goto_next(0x13)

	// 50% chance of this happening
	label(0x16)
	say_quip(CHR_BOND, 0x17, 0xff, 0x00, 0xff, 0x81, 0x00, 0x00)

	label(0x13)
	do_preset_animation(PRESETANIM_TALK3)
	return

	// Not Chicago
	label(0x15)
	say_quip(CHR_BOND, 0x22, 0xff, 0x00, 0xff, 0x81, 0x00, 0x00)
	goto_next(0x13)

	label(0x16)
	say_quip(CHR_BOND, 0x23, 0xff, 0x00, 0xff, 0x81, 0x00, 0x00)

	label(0x13)
	do_preset_animation(PRESETANIM_TALK3)
	return

	label(0x83)
	set_alertness(100)
	set_ailist(CHR_SELF, GAILIST_UNALERTED)
	endlist
};

/**
 * @ailist GAILIST_SURPRISED
 *
 * This function is only used by civilians,
 * and only when they get shot while talking.
 */
u8 func001c_surprised[] = {
	set_shotlist(GAILIST_ALERTED)
	increase_squadron_alertness(100)
	say_quip(CHR_BOND, QUIP_SHOTUNALERT, 0xff, 0x03, 0x00, BANK_0, 0x00, 0x00)
	restart_timer
	chr_do_animation(ANIM_SURPRISED_0202, 0, -1, 0, 16, CHR_SELF, 2)

	// Wait 1 second
	beginloop(0x03)
		if_timer_gt(60, /*goto*/ 0x77)
	endloop(0x03)

	label(0x77)
	set_ailist(CHR_SELF, GAILIST_ALERTED)
	set_returnlist(CHR_SELF, GAILIST_ALERTED)
	endlist
};

/**
 * @ailist GAILIST_FLEE_FROM_GRENADE
 */
u8 func0011_flee_from_grenade[] = {
	set_action(MA_RUNFROMGRENADE, FALSE)
	run_from_grenade
	say_quip(CHR_BOND, QUIP_GRENADE1, 0xff, 0x00, 0xff, BANK_0, 0x00, 0x00)
	restart_timer

	beginloop(0x0c)
		if_dangerous_object_nearby(3, /*goto*/ 0x13)
		goto_next(0x16)

		label(0x13)
		if_timer_gt(180, /*goto*/ 0x16)
	endloop(0x0c)

	label(0x16)
	return
	endlist
};

/**
 * @ailist GAILIST_OBSERVE_CAMSPY
 */
u8 func001b_observe_camspy[] = {
	stop_chr
	set_shotlist(GAILIST_ALERTED)
	unset_self_flag_bankx(CHRFLAG1_DOINGIDLEANIMATION, BANK_1)
	set_chr_special_death_animation(CHR_SELF, SPECIALDIE_NONE)
	set_chrpreset(CHR_TARGET)

	if_just_injured(CHR_SELF, /*goto*/ 0x09)

	label(0x00)
	set_target_chr(CHR_PRESET)
	if_distance_to_target_lt(300, /*goto*/ 0x13)
	try_jog_to_target(/*goto*/ 0x0c)
	restart_timer

	beginloop(0x0c)
		if_timer_gt(60, /*goto*/ 0x0b)
		if_distance_to_target_lt(300, /*goto*/ 0x13)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_target_outside_my_yvisang(/*goto*/ 0x16)
		if_target_in_sight(/*goto*/ 0x0b)

		label(0x16)
		set_target_chr(CHR_PRESET)
	endloop(0x0c)

	// At camspy
	label(0x13)
	set_target_chr(CHR_PRESET)
	restart_timer
	try_face_entity(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0x03)

	// Wait 5 seconds. During this time:
	// - If the camspy moves away, exit the loop and follow it
	// - If the player comes into sight, return from the function
	beginloop(0x03)
		if_timer_gt(300, /*goto*/ 0x04)
		if_distance_to_target_gt(400, /*goto*/ 0x13)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_target_outside_my_yvisang(/*goto*/ 0x16)
		if_target_in_sight(/*goto*/ 0x0b)

		label(0x16)
		set_target_chr(CHR_PRESET)
	endloop(0x03)

	// Camspy moved away - follow it
	label(0x13)
	goto_first(0x00)

	// Timer expired
	label(0x04)
	do_preset_animation(PRESETANIM_TALKRANDOM)
	say_quip(CHR_BOND, QUIP_SEEEYESPY, 0xff, 0x00, 0xff, BANK_0, 0x00, 0x00)

	// Wait another 5 seconds with same logic as previous timer
	beginloop(0x05)
		if_chr_idle(/*goto*/ 0x13)
		if_timer_gt(300, /*goto*/ 0x06)

		label(0x13)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_target_outside_my_yvisang(/*goto*/ 0x16)
		if_target_in_sight(/*goto*/ 0x0b)

		label(0x16)
		set_target_chr(CHR_PRESET)
	endloop(0x05)

	// Timer expired
	label(0x06)
	if_self_flag_bankx_eq(CHRFLAG1_LONG_CAMSPY_OBSERVATION, FALSE, BANK_1, /*goto*/ 0xe8)
	unset_self_flag_bankx(CHRFLAG1_LONG_CAMSPY_OBSERVATION, BANK_1)
	goto_first(0x00)

	// Player in sight while observing camspy
	label(0x0b)
	return

	// Timer expired second time
	label(0xe8)
	say_quip(CHR_BOND, QUIP_UNCOVEREDDISGUISE2, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)

	// Shoot camspy
	label(0x09)
	set_self_chrflag(CHRCFLAG_00000040)
	try_attack_stand(ATTACKFLAG_AIMATTARGET | ATTACKFLAG_AIMONLY, 0, /*goto*/ 0x16)

	label(0x16)
	try_modify_attack(ATTACKFLAG_AIMATTARGET, 0, /*goto*/ 0xc3)

	// Wait until shooting animation done
	beginloop(0xc3)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0xc3)

	label(0x16)
	if_chr_deadish(CHR_TARGET, /*goto*/ 0x13)

	// Camspy still alive
	if_can_see_target(/*goto*/ 0x16)
	goto_next(0x13)

	// Unreachable - no label
	if_chrpresets_target_is_different(/*goto*/ 0x16)
	goto_next(0x13)

	label(0x16)
	if_target_is_player(/*goto*/ 0x13)
	goto_first(0x09)

	label(0x13)
	set_target_chr(CHR_BOND)
	set_self_flag_bankx(CHRFLAG1_SEARCHSAMEROOM, BANK_1)
	set_ailist(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER)
	endlist
};

/**
 * @ailist GAILIST_SEARCH_FOR_PLAYER
 */
u8 func001d_search_for_player[] = {
	if_chr_deadish(CHR_SELF, /*goto*/ 0x13)
	goto_next(0x16)

	// Dying
	label(0x13)
	set_shotlist(GAILIST_IDLE)
	terminate

	// Alive
	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG1_DOINGIDLEANIMATION, FALSE, BANK_1, /*goto*/ 0x13)
	set_chr_special_death_animation(CHR_SELF, SPECIALDIE_NONE)

	label(0x13)
	set_chr_dodge_rating(2, 0)

	label(0x00)
	set_target_chr(CHR_PRESET)
	set_self_flag_bankx(CHRFLAG1_LOOKINGFORTARGET, BANK_1)
	if_self_flag_bankx_eq(CHRFLAG1_SEARCHSAMEROOM, TRUE, BANK_1, /*goto*/ 0x16)
	set_chr_roomtosearch
	if_distance_to_target_gt(1000, /*goto*/ 0x13)
	try_jog_to_target(/*goto*/ 0x03)

	label(0x13)
	try_run_to_target(/*goto*/ 0x03)

	beginloop(0x03)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_target_in_sight(/*goto*/ 0x12)
		if_self_flag_bankx_eq(CHRFLAG0_AIVSAI, FALSE, BANK_0, /*goto*/ 0x13)
		if_enemy_distance_lt_and_los(2540, /*goto*/ 0xc3)

		label(0x13)
		if_chr_in_room(CHR_SELF, 0x01, 0x0001, /*goto*/ 0x16)
	endloop(0x03)

	// Player is in the same room as self
	label(0x16)
	restart_timer

	beginloop(0x0a)
		if_self_flag_bankx_eq(CHRFLAG0_AIVSAI, FALSE, BANK_0, /*goto*/ 0x13)
		if_self_flag_bankx_eq(CHRFLAG1_INDARKROOM, TRUE, BANK_1, /*goto*/ 0x15)
		if_enemy_distance_lt_and_los(2540, /*goto*/ 0xc3)
		goto_next(0x13)

		label(0x15)
		if_enemy_distance_lt_and_los(100, /*goto*/ 0xc3)

		label(0x13)
		if_self_flag_bankx_eq(CHRFLAG1_INDARKROOM, FALSE, BANK_1, /*goto*/ 0x13)
		if_distance_to_target_lt(500, /*goto*/ 0x13)
		goto_next(0x05)

		label(0x13)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_target_outside_my_yvisang(/*goto*/ 0x15)
		if_target_in_sight(/*goto*/ 0x12)

		label(0x15)
		if_timer_gt(30, /*goto*/ 0x05)
	endloop(0x0a)

	label(0x05)
	restart_timer

	label(0x06)
	if_distance_to_target_lt(1000, /*goto*/ 0x13)
	goto_first(0x00)

	label(0x13)
	stop_chr
	if_self_flag_bankx_eq(CHRFLAG1_DONE_SEARCH_ANIM, TRUE, BANK_1, /*goto*/ 0x13)
	call_rng
	if_rand_gt(128, /*goto*/ 0x13)
	call_rng
	if_rand_lt(64, /*goto*/ 0x28)
	if_rand_lt(128, /*goto*/ 0x29)
	if_rand_lt(196, /*goto*/ 0x2a)
	set_self_flag_bankx(CHRFLAG1_DONE_SEARCH_ANIM, BANK_1)
	chr_do_animation(ANIM_LOOK_AROUND_025B, 0, -1, CHRANIMFLAG_SLOWUPDATE, 10, CHR_SELF, 2)
	goto_next(0x04)

	label(0x28)
	chr_do_animation(ANIM_LOOK_AROUND_025C, 0, -1, CHRANIMFLAG_SLOWUPDATE, 10, CHR_SELF, 2)
	goto_next(0x04)

	label(0x29)
	chr_do_animation(ANIM_LOOK_AROUND_025D, 0, -1, CHRANIMFLAG_SLOWUPDATE, 10, CHR_SELF, 2)
	goto_next(0x04)

	label(0x2a)
	chr_do_animation(ANIM_LOOK_AROUND_025E, 0, -1, CHRANIMFLAG_SLOWUPDATE, 10, CHR_SELF, 2)
	goto_next(0x04)

	label(0x13)
	unset_self_flag_bankx(CHRFLAG1_DONE_SEARCH_ANIM, BANK_1)
	call_rng
	if_rand_lt(64, /*goto*/ 0x28)
	if_rand_lt(128, /*goto*/ 0x29)
	if_rand_lt(196, /*goto*/ 0x2a)
	try_set_target_pad_to_something(0x08, /*goto*/ 0x13)

	label(0x28)
	try_set_target_pad_to_something(0x02, /*goto*/ 0x13)

	label(0x29)
	try_set_target_pad_to_something(0x04, /*goto*/ 0x13)

	label(0x2a)
	try_set_target_pad_to_something(0x08, /*goto*/ 0x13)
	goto_next(0x04)

	label(0x13)
	go_to_target_pad(GOPOSFLAG_WALK)

	beginloop(0x04)
		if_self_flag_bankx_eq(CHRFLAG0_AIVSAI, FALSE, BANK_0, /*goto*/ 0x13)
		if_enemy_distance_lt_and_los(2540, /*goto*/ 0xc3)

		label(0x13)
		if_self_flag_bankx_eq(CHRFLAG1_INDARKROOM, FALSE, BANK_1, /*goto*/ 0x13)
		if_distance_to_target_lt(500, /*goto*/ 0x13)

		label(0x13)
		chr_toggle_p1p2(CHR_SELF)
		set_target_chr(CHR_P1P2)
		if_timer_gt(600, /*goto*/ 0x0b)
		if_chr_stopped(/*goto*/ 0x16)
		if_hears_target(/*goto*/ 0x12)
		if_alertness(99, OPERATOR_GREATER_THAN, /*goto*/ 0x12)
		if_near_miss(/*goto*/ 0x12)
		if_self_flag_bankx_eq(CHRFLAG1_INDARKROOM, FALSE, BANK_1, /*goto*/ 0x13)
		if_distance_to_target_gt(500, /*goto*/ 0x15)

		label(0x13)
		if_target_outside_my_yvisang(/*goto*/ 0x15)
		if_target_in_sight(/*goto*/ 0x12)

		label(0x15)
		if_saw_death(0x00, /*goto*/ 0x28)
	endloop(0x04)

	label(0x16)
	goto_first(0x06)

	// Saw death?
	label(0x28)

	label(0x12)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DISGUISED, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_alertness(0)
	set_returnlist(CHR_SELF, GAILIST_UNALERTED)
	set_ailist(CHR_SELF, GAILIST_UNALERTED)

	label(0x16)
	set_alertness(100)
	if_self_flag_bankx_eq(CHRFLAG1_INDARKROOM, FALSE, BANK_1, /*goto*/ 0x13)
	say_quip(CHR_BOND, QUIP_SEARCHSUCCESS, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_ALERTED)

	label(0x0b)
	say_quip(CHR_BOND, QUIP_SEARCHFAIL, 0x32, 0x03, 0xff, BANK_0, 0x00, 0x00)
	goto_first(0x05)

	label(0xc3)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DISGUISED, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	set_alertness(0)
	set_returnlist(CHR_SELF, GAILIST_UNALERTED)
	set_ailist(CHR_SELF, GAILIST_UNALERTED)

	label(0x16)
	set_returnlist(CHR_SELF, GAILIST_UNALERTED)
	set_shotlist(GAILIST_COMBAT_WITH_TARGET)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)
	endlist
};

/**
 * @ailist GAILIST_RELATED_TO_SPAWNING
 */
u8 func001f_related_to_spawning[] = {
	label(0x23)
	if_chr_deadish(CHR_SELF, /*goto*/ 0x13)
	goto_next(0x16)

	// Dying
	label(0x13)
	set_shotlist(GAILIST_IDLE)
	terminate

	// Alive
	label(0x16)
	set_returnlist(CHR_SELF, GAILIST_RELATED_TO_SPAWNING)
	set_target_chr(CHR_P1P2)
	set_shotlist(GAILIST_RELATED_TO_SPAWNING)
	if_num_times_shot_lt(1, /*goto*/ 0x1a) // pointless check

	label(0x1a)
	set_action(MA_NORMAL, TRUE)
	yield
	chr_toggle_p1p2(CHR_SELF)
	set_target_chr(CHR_P1P2)
	if_alertness(100, OPERATOR_LESS_THAN, /*goto*/ 0x16)
	goto_next(0x78)

	// Not alert
	label(0x16)
	if_saw_death(0x01, /*goto*/ 0x1e)
	if_saw_injury(0x01, /*goto*/ 0x1e)
	if_target_outside_my_yvisang(/*goto*/ 0x16)
	if_target_in_sight(/*goto*/ 0x1e)

	label(0x16)
	if_self_flag_bankx_eq(CHRFLAG0_AIVSAI, FALSE, BANK_0, /*goto*/ 0x16)
	if_enemy_distance_lt_and_los(2540, /*goto*/ 0x13)
	goto_next(0x16)

	label(0x13)
	goto_next(0x12)

	label(0x16)
	if_near_miss(/*goto*/ 0x1e)
	if_num_times_shot_gt(0, /*goto*/ 0x1e)
	if_self_flag_bankx_eq(CHRFLAG0_NOHEAR, FALSE, BANK_0, /*goto*/ 0x16)
	if_can_see_target(/*goto*/ 0x16)
	goto_next(0x13)

	label(0x16)
	if_heard_target_recently(/*goto*/ 0x1f)

	label(0x13)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_HEAR_ALARMS, FALSE, BANK_0, /*goto*/ 0x16)
	if_alarm_active(/*goto*/ 0x23)

	label(0x16)
	goto_first(0x1a)

	label(0x23)
	goto_first(0x23)

	label(0x1e)
	say_quip(CHR_BOND, QUIP_SEEPLAYER, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)

	label(0x9f)
	increase_squadron_alertness(100)
	set_alertness(255)
	set_returnlist(CHR_SELF, GAILIST_RELATED_TO_SPAWNING)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)

	label(0x1f)
	yield
	say_quip(CHR_BOND, QUIP_HEARNOISE, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_HEARSPAWN, TRUE, BANK_0, /*goto*/ 0xda)
	increase_squadron_alertness(100)
	set_returnlist(CHR_SELF, GAILIST_RELATED_TO_SPAWNING)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)

	label(0xda)
	if_never_been_onscreen(/*goto*/ 0x0e)
	set_returnlist(CHR_SELF, GAILIST_RELATED_TO_SPAWNING)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)

	label(0x0e)
	if_chr_death_animation_finished(CHR_CLONE, /*goto*/ 0x0e)
	if_chr_knockedout(CHR_CLONE, /*goto*/ 0x0e)
	set_returnlist(CHR_SELF, GAILIST_RELATED_TO_SPAWNING)
	set_ailist(CHR_SELF, GAILIST_RELATED_TO_SPAWNING)

	label(0x0e)
	try_spawn_clone2(CHR_SELF, GAILIST_SEARCH_FOR_PLAYER, 0, /*goto*/ 0x13)
	set_returnlist(CHR_SELF, GAILIST_RELATED_TO_SPAWNING)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)

	label(0x13)
	set_ailist(CHR_SELF, GAILIST_RELATED_TO_SPAWNING)

	// Alert
	label(0x78)
	if_self_flag_bankx_eq(CHRFLAG0_CAN_HEARSPAWN, FALSE, BANK_0, /*goto*/ 0x13)
	set_alertness(0)
	goto_first(0xda)

	label(0x13)
	set_returnlist(CHR_SELF, GAILIST_RELATED_TO_SPAWNING)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)

	label(0x12)
	if_self_flag_bankx_eq(CHRFLAG1_AIVSAI_ADVANTAGED, TRUE, BANK_1, /*goto*/ 0x13)
	set_returnlist(CHR_SELF, GAILIST_RELATED_TO_SPAWNING)

	label(0x13)
	set_returnlist(CHR_SELF, GAILIST_RELATED_TO_SPAWNING)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)
	endlist
};

/**
 * @ailist GAILIST_INIT_DEFAULT_BUDDY
 */
u8 func0012_init_default_buddy[] = {
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_PASSIVE)
	set_self_chrflag(CHRCFLAG_00040000)
	set_chr_team(CHR_SELF, TEAM_ALLY)
	set_self_chrflag(CHRCFLAG_PUSHABLE)
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_DETECTED)
	set_alertness(100)
	set_chrpreset(CHR_BOND)
	yield
	rebuild_teams
	rebuild_squadrons
	yield
	yield
	yield
	yield
	yield
	yield
	yield
	yield

	// Wait for intro to finish
	beginloop(0x03)
		if_camera_animating(/*goto*/ 0x13)
		goto_next(0x16)

		label(0x13)
	endloop(0x03)

	label(0x16)
	rebuild_teams
	rebuild_squadrons

#if VERSION >= VERSION_NTSC_1_0
	set_target_chr(-1)

	// Set stage flag 00000080 for Air Force One and Skedar Ruins
	if_stage_is_not(STAGE_AIRFORCEONE, /*goto*/ 0x15)
	set_stage_flag(0x00000080)

	label(0x15)
	if_stage_is_not(STAGE_SKEDARRUINS, /*goto*/ 0x15)
	set_stage_flag(0x00000080)

	label(0x15)
#endif

	// Uncloak
	set_chr_cloaked(CHR_SELF, FALSE, TRUE)

	// Wait 2 seconds
	restart_timer

	beginloop(0x04)
		if_timer_gt(120, /*goto*/ 0x16)
	endloop(0x04)

	label(0x16)
	set_target_chr(-1)

	// Set the flags again
	if_stage_is_not(STAGE_AIRFORCEONE, /*goto*/ 0x15)
	set_stage_flag(0x00000080)

	label(0x15)
	if_stage_is_not(STAGE_SKEDARRUINS, /*goto*/ 0x15)
	set_stage_flag(0x00000080)

	label(0x15)
	set_ailist(CHR_SELF, GAILIST_BUDDY_MAIN)
	endlist
};

/**
 * @ailist GAILIST_INIT_PUGILIST_BUDDY
 *
 * Used to initialise the Pugilist buddy on all stages except MBR.
 */
u8 func0013_init_pugilist_buddy[] = {
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_PASSIVE)
	set_chr_team(CHR_SELF, TEAM_ALLY)
	set_self_chrflag(CHRCFLAG_00040000)
	set_self_chrflag(CHRCFLAG_PUSHABLE)
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_DETECTED)
	set_alertness(200)
	yield
	yield
	yield
	yield
	yield
	yield
	yield
	yield
	yield
	set_self_chrflag(CHRCFLAG_RUNFASTER)

	// Wait for intro to finish
	beginloop(0x03)
		if_camera_animating(/*goto*/ 0x13)
		goto_next(0x16)

		label(0x13)
	endloop(0x03)

	label(0x16)
	rebuild_teams
	rebuild_squadrons
	set_chr_cloaked(CHR_SELF, FALSE, TRUE)

	// Wait 2 seconds
	restart_timer

	beginloop(0x04)
		if_timer_gt(120, /*goto*/ 0x16)
	endloop(0x04)

	label(0x16)
	set_target_chr(-1)

	// Set stage flag 00000080 for Air Force One and Skedar Ruins
	if_stage_is_not(STAGE_AIRFORCEONE, /*goto*/ 0x15)
	set_stage_flag(0x00000080)

	label(0x15)
	if_stage_is_not(STAGE_SKEDARRUINS, /*goto*/ 0x15)
	set_stage_flag(0x00000080)

	label(0x15)
	set_ailist(CHR_SELF, GAILIST_BUDDY_MAIN)
	endlist
};

#define LABEL_PLACE_BUDDY 0x0a

/**
 * @ailist GAILIST_BUDDY_MAIN
 *
 * Main loop for coop buddies.
 */
u8 func0014_buddy_main[] = {
	set_returnlist(CHR_SELF, GAILIST_BUDDY_MAIN)
	set_shotlist(GAILIST_BUDDY_MAIN)
	unset_self_chrflag(CHRCFLAG_00040000)
	set_self_flag_bankx(CHRFLAG1_AIVSAI_ADVANTAGED, BANK_1)
	set_morale(0)
	set_shotlist(GAILIST_BUDDY_MAIN)
	set_target_chr(CHR_BOND)
	if_chr_deadish(CHR_SELF, /*goto*/ 0x13)
	goto_next(0x16)

	// Dying
	label(0x13)
	terminate

	// Alive
	label(0x16)
	if_just_injured(CHR_SELF, /*goto*/ 0x13)
	goto_next(0x16)

	// Injured
	label(0x13)
	say_quip(CHR_BOND, QUIP_INJURED2, 0xff, 0x14, 0xff, BANK_0, 0x00, 0x00)

	beginloop(0xf2)
		if_chr_has_hiddenflag(CHR_BOND, CHRHFLAG_TRIGGER_BUDDY_WARP, /*goto*/ 0x13)
		unset_chr_hiddenflag(CHR_SELF, CHRHFLAG_UNTARGETABLE)
		goto_next(0x15)

		label(0x13)
		if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_UNTARGETABLE, /*goto*/ 0x15)
		goto_next(LABEL_PLACE_BUDDY)

		label(0x15)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0xf2)

	// Healthy
	label(0x16)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DONTSHOOTME, /*goto*/ 0x13)
	unset_self_flag_bankx(CHRFLAG1_HANDCOMBATONLY, BANK_1)
	goto_next(0x16)

	label(0x13)
	set_self_flag_bankx(CHRFLAG1_HANDCOMBATONLY, BANK_1)

	label(0x16)
	if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_PASSIVE, /*goto*/ 0xdc)
	if_enemy_distance_lt_and_los(2540, /*goto*/ 0x03)

	label(0xdc)
	set_target_chr(CHR_BOND)
	if_distance_to_target_lt(200, /*goto*/ 0x16)

	// Start outer loop
	label(0x00)
	set_target_chr(CHR_BOND)
	restart_timer
	if_distance_to_target_gt(300, /*goto*/ 0x16)
	try_jog_to_target(/*goto*/ 0x0c)

	label(0x16)
	try_run_to_target(/*goto*/ 0x0c)

	beginloop(0x0c)
		if_chr_has_hiddenflag(CHR_BOND, CHRHFLAG_TRIGGER_BUDDY_WARP, /*goto*/ 0x13)
		unset_chr_hiddenflag(CHR_SELF, CHRHFLAG_UNTARGETABLE)
		goto_next(0x15)

		label(0x13)
		if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_UNTARGETABLE, /*goto*/ 0x15)
		goto_next(LABEL_PLACE_BUDDY)

		label(0x15)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DISGUISED, /*goto*/ 0x06)
		goto_next(0x07)

		label(0x06)
		set_chr_hiddenflag(CHR_SELF, CHRHFLAG_DISGUISED)

		label(0x07)
		set_target_chr(CHR_BOND)
		if_can_see_target(/*goto*/ 0xdd)
		if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_PASSIVE, /*goto*/ 0xdc)

		label(0xdd)
		if_enemy_distance_lt_and_los(2540, /*goto*/ 0x03)

		label(0xdc)
		set_target_chr(CHR_BOND)
		if_distance_to_target_lt(200, /*goto*/ 0x16)
		if_timer_gt(60, /*goto*/ 0x13)
	endloop(0x0c)

	label(0x13)
	goto_first(0x00)

	// Standing next to player
	label(0x16)
	stop_chr

	beginloop(0x04)
		if_chr_has_hiddenflag(CHR_BOND, CHRHFLAG_TRIGGER_BUDDY_WARP, /*goto*/ 0x13)
		unset_chr_hiddenflag(CHR_SELF, CHRHFLAG_UNTARGETABLE)
		goto_next(0x15)

		label(0x13)
		if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_UNTARGETABLE, /*goto*/ 0x15)
		goto_next(LABEL_PLACE_BUDDY)

		label(0x15)
		label(0x13)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DISGUISED, /*goto*/ 0x06)
		goto_next(0x07)

		label(0x06)
		set_chr_hiddenflag(CHR_SELF, CHRHFLAG_DISGUISED)

		label(0x07)
		if_chr_has_hiddenflag(CHR_SELF, CHRHFLAG_PASSIVE, /*goto*/ 0xdc)
		if_stage_is_not(STAGE_G5BUILDING, /*goto*/ 0x15)
		goto_next(0x13)

		label(0x15)
		if_stage_is_not(STAGE_DEEPSEA, /*goto*/ 0x15)
		goto_next(0x13)

		// Not G5 Building or Deep Sea
		label(0x15)
		set_target_to_enemy_on_same_floor(/*goto*/ 0x0b)

		// All stages
		label(0x13)
		label(0xdc)
		if_enemy_distance_lt_and_los(2540, /*goto*/ 0x03)
		set_target_chr(CHR_BOND)
		if_distance_to_target_gt(300, /*goto*/ 0x16)
	endloop(0x04)

	label(0x16)
	goto_first(0x00)

	label(0x03)
	call_rng
	if_rand_lt(85, /*goto*/ 0xfa)
	if_rand_lt(170, /*goto*/ 0xf3)
	say_quip(CHR_BOND, QUIP_ATTACK1, 0xff, 0x14, 0xff, BANK_0, 0x00, 0x00)
	goto_next(0x13)

	label(0xfa)
	say_quip(CHR_BOND, QUIP_SEEPLAYER, 0xff, 0x14, 0xff, BANK_0, 0x00, 0x00)
	goto_next(0x13)

	label(0xf3)
	say_quip(CHR_BOND, QUIP_HITPLAYER, 0xff, 0x14, 0xff, BANK_0, 0x00, 0x00)

	label(0x13)

	// Found chr in group from non G5 or Deep Sea stage
	label(0x0b)
	if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DONTSHOOTME, /*goto*/ 0x13)
	unset_self_flag_bankx(CHRFLAG1_HANDCOMBATONLY, BANK_1)
	goto_next(0x16)

	label(0x13)
	set_self_flag_bankx(CHRFLAG1_HANDCOMBATONLY, BANK_1)

	label(0x16)
	set_returnlist(CHR_SELF, GAILIST_BUDDY_MAIN)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)

	label(LABEL_PLACE_BUDDY)
	set_ailist(CHR_SELF, GAILIST_BUDDY_WARP)
	endlist
};

/**
 * @ailist GAILIST_DODGE
 *
 * Do a sideways dodge, then assign GAILIST_ALERTED.
 */
u8 func0023_dodge[] = {
	if_chr_deadish(CHR_SELF, /*goto*/ 0x16)
	goto_next(0x13)

	// Dying
	label(0x16)
	set_aishootingatmelist(GAILIST_IDLE)
	terminate

	label(0x13)
	set_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)

	// 50% chance of hop vs jump
	call_rng
	if_rand_lt(128, /*goto*/ 0x2d)
	try_jumpout(/*goto*/ 0x13)
	goto_next(0x2e)

	label(0x2d)
	try_sidestep(/*goto*/ 0x13)
	goto_next(0x2c)

	// Unreachable
	label(0xbb)
	cmd0139(25, 0x01, TRUE)
	goto_next(0x2c)

	// Was able to start dodge
	label(0x13)

	beginloop(0x2c)
		if_chr_stopped(/*goto*/ 0x2e)
	endloop(0x2c)

	// Unable to hop sideways, or follow-through from above
	label(0x2e)
	set_self_flag_bankx(CHRFLAG1_DODGED, BANK_1)
	set_ailist(CHR_SELF, GAILIST_ALERTED)
	endlist
};

#if VERSION >= VERSION_NTSC_1_0
/**
 * @ailist GAILIST_BUDDY_STEALTH
 *
 * In co-op mode with an AI buddy, this ailist is applied to the buddy when you
 * give them the Stealth command.
 */
u8 func0015_buddy_stealth[] = {
	stop_chr
	set_chr_cloaked(CHR_SELF, TRUE, TRUE)

	// Wait 1 second
	restart_timer

	beginloop(0x03)
		if_timer_gt(60, /*goto*/ 0x16)
	endloop(0x03)

	// Hide chr
	label(0x16)
	remove_references_to_chr
	set_self_chrflag(CHRCFLAG_INVINCIBLE)
	set_self_chrflag((CHRCFLAG_HIDDEN | CHRCFLAG_PERIMDISABLEDTMP | CHRCFLAG_00040000))

	// Wait 3 seconds
	restart_timer

	beginloop(0x04)
		if_timer_gt(180, /*goto*/ 0x16)
	endloop(0x04)

	label(0x16)

	// Wait for intro to finish and for 1 second to have passed
	// (in case intro was cut by player)
	#define wait_intro(loopid) \
		restart_timer \
		beginloop(loopid) \
			if_camera_animating(/*goto*/ 0x13) \
			if_timer_gt(60, /*goto*/ 0x16) \
			label(0x13) \
		endloop(loopid) \
		label(0x16)

	wait_intro(0x05)
	cmd01b4_if_something(/*goto*/ 0x17)

	wait_intro(0x06)
	cmd01b4_if_something(/*goto*/ 0x17)

	wait_intro(0x07)
	cmd01b4_if_something(/*goto*/ 0x17)

	wait_intro(0x08)
	cmd01b4_if_something(/*goto*/ 0x17)

	wait_intro(0x09)
	cmd01b4_if_something(/*goto*/ 0x17)

	wait_intro(0x0a)
	cmd01b4_if_something(/*goto*/ 0x17)

	label(0x19)
	wait_intro(0x0b)

	// This will execute every second after the initial 7ish seconds and while
	// cutscene is not running. The chr is being moved back to the player
	// repeatedly.
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_00000200 | CHRHFLAG_00100000)
	chr_move_to_pad(CHR_SELF, CHR_BOND, 88, /*goto*/ 0x17)
	goto_first(0x19)

	label(0x17)
	unset_self_chrflag(CHRCFLAG_HIDDEN | CHRCFLAG_PERIMDISABLEDTMP | CHRCFLAG_00040000)
	unset_self_chrflag(CHRCFLAG_INVINCIBLE)
	unset_chr_hiddenflag(CHR_SELF, CHRHFLAG_00000200 | CHRHFLAG_00100000)
	set_chr_cloaked(CHR_SELF, FALSE, TRUE)
	set_returnlist(CHR_SELF, GAILIST_BUDDY_MAIN)
	set_shotlist(GAILIST_BUDDY_MAIN)
	set_ailist(CHR_SELF, GAILIST_BUDDY_MAIN)
	endlist
};
#endif

/**
 * @ailist GAILIST_INIT_PSYCHOSIS
 *
 * Handle a chr being shot with the psychosis gun.
 */
u8 func0026_init_psychosis[] = {
	set_shotlist(GAILIST_INIT_PSYCHOSIS)
	set_chr_team(CHR_SELF, TEAM_NONCOMBAT)
	set_self_flag_bankx(CHRFLAG1_PUNCHHARDER, BANK_1)
	set_self_chrflag(CHRCFLAG_00040000)
	set_self_chrflag(CHRCFLAG_PUSHABLE)
	set_self_flag_bankx(CHRFLAG1_AIVSAI_ADVANTAGED, BANK_1)
	set_self_flag_bankx(CHRFLAG1_IGNORECOVER, BANK_1)
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_PSYCHOSISED)
	set_alertness(100)
	set_chrpreset(CHR_BOND)
	yield
	rebuild_teams
	rebuild_squadrons
	yield
	yield
	yield
	yield
	yield
	yield
	yield
	yield
	chr_do_animation(ANIM_BIG_SNEEZE, -1, -1, CHRANIMFLAG_SLOWUPDATE, 20, CHR_SELF, 2)

	// Wait for animation to finish
	beginloop(0x03)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0x03)

	// Become ally
	label(0x16)
	set_chr_team(CHR_SELF, TEAM_ALLY)
	rebuild_teams
	rebuild_squadrons

	label(0x16)
	set_target_chr(-1)
	set_ailist(CHR_SELF, GAILIST_PSYCHOSISED)
	endlist
};

/**
 * @ailist GAILIST_PSYCHOSISED
 *
 * Main loop for a chr who is psychosised.
 */
u8 func0027_psychosised[] = {
	set_morale(0)
	set_shotlist(GAILIST_PSYCHOSISED)
	set_target_chr(CHR_PRESET)
	if_chr_deadish(CHR_SELF, /*goto*/ 0x13)
	goto_next(0x16)

	// Dying
	label(0x13)
	terminate

	// Alive
	label(0x16)
	if_just_injured(CHR_SELF, /*goto*/ 0x13)
	goto_next(0x16)

	// Injured
	label(0x13)
	say_quip(CHR_BOND, QUIP_INJURED2, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)

	beginloop(0xf2)
		if_chr_stopped(/*goto*/ 0x16)
	endloop(0xf2)

	// Healthy
	label(0x16)
	set_target_to_enemy_on_same_floor(/*goto*/ 0x03)
	if_enemy_distance_lt_and_los(2540, /*goto*/ 0x03)
	set_target_chr(CHR_PRESET)
	if_distance_to_target_lt(200, /*goto*/ 0x16)

	// Following Joanna
	label(0x00)
	set_target_chr(CHR_PRESET)
	restart_timer
	if_distance_to_target_gt(300, /*goto*/ 0x16)
	try_jog_to_target(/*goto*/ 0x0c)

	label(0x16)
	if_distance_to_target_lt(200, /*goto*/ 0x16)
	try_run_to_target(/*goto*/ 0x0c)

	beginloop(0x0c)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DISGUISED, /*goto*/ 0x06)
		goto_next(0x07)

		label(0x06)
		set_chr_hiddenflag(CHR_SELF, CHRHFLAG_DISGUISED)

		label(0x07)
		if_enemy_distance_lt_and_los(2540, /*goto*/ 0x03)
		set_target_chr(CHR_PRESET)
		if_distance_to_target_lt(200, /*goto*/ 0x16)
		if_timer_gt(60, /*goto*/ 0x13)
	endloop(0x0c)

	label(0x13)
	goto_first(0x00)

	// At Joanna
	label(0x16)
	stop_chr

	beginloop(0x04)
		if_chr_has_hiddenflag(CHR_TARGET, CHRHFLAG_DISGUISED, /*goto*/ 0x06)
		goto_next(0x07)

		label(0x06)
		set_chr_hiddenflag(CHR_SELF, CHRHFLAG_DISGUISED)

		label(0x07)
		set_target_to_enemy_on_same_floor(/*goto*/ 0x03)
		if_enemy_distance_lt_and_los(2540, /*goto*/ 0x03)
		set_target_chr(CHR_PRESET)
		if_distance_to_target_gt(300, /*goto*/ 0x16)
	endloop(0x04)

	label(0x16)
	goto_first(0x00)

	// Detected enemy
	label(0x03)
	call_rng
	if_rand_lt(85, /*goto*/ 0xfa)
	if_rand_lt(170, /*goto*/ 0xf3)

	say_quip(CHR_BOND, QUIP_ATTACK1, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	goto_next(0x13)

	label(0xfa)
	say_quip(CHR_BOND, QUIP_SEEPLAYER, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)
	goto_next(0x13)

	label(0xf3)
	say_quip(CHR_BOND, QUIP_HITPLAYER, 0xff, 0x03, 0xff, BANK_0, 0x00, 0x00)

	label(0x13)
	set_returnlist(CHR_SELF, GAILIST_PSYCHOSISED)
	set_ailist(CHR_SELF, GAILIST_COMBAT_WITH_TARGET)
	endlist
};

/**
 * @ailist GAILIST_INVINCIBLE_AND_IDLE
 *
 * Default ailist for some cutscene chrs.
 */
u8 func002d_invincible_and_idle[] = {
	set_self_chrflag(CHRCFLAG_INVINCIBLE)
	set_self_chrflag(CHRCFLAG_UNEXPLODABLE)
	terminate
	endlist
};

/**
 * @ailist GAILIST_BUDDY_WARP
 *
 * Warp the coop buddy to be near the player again.
 * Used when using teleports in Deep Sea, probably Attack Ship lifts and more.
 */
u8 func0020_buddy_warp[] = {
	set_self_chrflag(CHRCFLAG_INVINCIBLE)
	set_self_chrflag(CHRCFLAG_00040000)
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_CLOAKED)
	stop_chr
	set_target_chr(CHR_BOND)
	yield
	if_stage_is_not(STAGE_DEEPSEA, /*goto*/ 0x15)
	goto_next(0x16)

	// Not Deep Sea
	label(0x15)
	if_stage_is_not(STAGE_AIRFORCEONE, /*goto*/ 0x15)

	// Deep Sea or Air Force One
	label(0x16)
	yield
	yield
	yield
	yield
	yield

	// Try to move chr to one of four pads until it works
	beginloop(0x03)
		try_set_padpreset_to_target_quadrant(QUADRANT_BACK, /*goto*/ 0xfa)
		goto_next(0xf3)

		label(0xfa)
		chr_move_to_pad(CHR_SELF, PAD_PRESET, 0x01, /*goto*/ 0x13)

		label(0xf3)
		try_set_padpreset_to_target_quadrant(QUADRANT_SIDE1, /*goto*/ 0xfa)
		goto_next(0xf3)

		label(0xfa)
		chr_move_to_pad(CHR_SELF, PAD_PRESET, 0x01, /*goto*/ 0x13)

		label(0xf3)
		try_set_padpreset_to_target_quadrant(QUADRANT_SIDE2, /*goto*/ 0xfa)
		goto_next(0xf3)

		label(0xfa)
		chr_move_to_pad(CHR_SELF, PAD_PRESET, 0x01, /*goto*/ 0x13)

		label(0xf3)
		try_set_padpreset_to_target_quadrant(QUADRANT_FRONT, /*goto*/ 0xfa)
		goto_next(0xf3)

		label(0xfa)
		chr_move_to_pad(CHR_SELF, PAD_PRESET, 0x01, /*goto*/ 0x13)

		label(0xf3)
	endloop(0x03)

	// Move to pad worked
	label(0x13)
	unset_self_chrflag(CHRCFLAG_HIDDEN)
	unset_self_chrflag(CHRCFLAG_INVINCIBLE)
	stop_chr
	set_chr_cloaked(CHR_SELF, FALSE, TRUE)
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_UNTARGETABLE)
	set_returnlist(CHR_SELF, GAILIST_BUDDY_MAIN)
	set_ailist(CHR_SELF, GAILIST_BUDDY_MAIN)

	// Not Deep Sea nor Air Force One
	label(0x15)
	yield
	yield
	yield
	yield
	yield
	stop_chr
	if_stage_is_not(STAGE_ATTACKSHIP, /*goto*/ 0x04)

	// Attack Ship
	beginloop(0x05)
		chr_move_to_pad(CHR_SELF, 0x011f, 0x01, /*goto*/ 0x13)
		chr_move_to_pad(CHR_SELF, 0x0120, 0x01, /*goto*/ 0x13)
		chr_move_to_pad(CHR_SELF, 0x0121, 0x01, /*goto*/ 0x13)
		chr_move_to_pad(CHR_SELF, 0x0122, 0x01, /*goto*/ 0x13)
	endloop(0x05)

	// Not Air Force One, Deep Sea or Attack Ship
	beginloop(0x04)
		chr_move_to_pad(CHR_SELF, 0x0012, 0x01, /*goto*/ 0x13)
		chr_move_to_pad(CHR_SELF, 0x001a, 0x01, /*goto*/ 0x13)
		chr_move_to_pad(CHR_SELF, 0x0013, 0x01, /*goto*/ 0x13)
		chr_move_to_pad(CHR_SELF, 0x0019, 0x01, /*goto*/ 0x13)
	endloop(0x04)

	// Move to pad worked
	label(0x13)
	stop_chr
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_00020000)
	set_chr_hiddenflag(CHR_SELF, CHRHFLAG_UNTARGETABLE)

	// Wait until flag unset (timer is not checked)
	restart_timer

	beginloop(0xef)
		if_chr_has_hiddenflag(CHR_BOND, CHRHFLAG_TRIGGER_BUDDY_WARP, /*goto*/ 0x13)
		goto_next(0x16)

		label(0x13)
	endloop(0xef)

	// Unhide
	label(0x16)
	unset_self_chrflag(CHRCFLAG_HIDDEN)
	unset_self_chrflag(CHRCFLAG_INVINCIBLE)
	stop_chr
	set_chr_cloaked(CHR_SELF, FALSE, TRUE)
	set_returnlist(CHR_SELF, GAILIST_BUDDY_MAIN)
	set_ailist(CHR_SELF, GAILIST_BUDDY_MAIN)
	endlist
};

/**
 * @ailist GAILIST_STOP_AND_IDLE
 */
u8 func0021_stop_and_idle[] = {
	set_shotlist(GAILIST_STOP_AND_IDLE)
	stop_chr
	terminate
	endlist
};

/**
 * @ailist GAILIST_AIBOT_INIT
 *
 * Assigned at start of match, then switches to GAILIST_AIBOT_MAIN.
 *
 * Also assigned the moment the aibot dies, but this jumps out immediately to
 * GAILIST_AIBOT_DEAD while their death animation is happening.
 *
 * Also assigned upon respawn.
 */
u8 func0029_aibot_init[] = {
	set_chr_maxdamage(CHR_SELF, 80)
	set_reaction_speed(100)
	set_returnlist(CHR_SELF, GAILIST_AIBOT_MAIN)

	beginloop(0x19)
		if_chr_death_animation_finished(CHR_SELF, /*goto*/ 0x13)
		goto_next(0x16)

		label(0x13)
	endloop(0x19)

	label(0x16)
	set_ailist(CHR_SELF, GAILIST_AIBOT_MAIN)
	endlist
};

/**
 * @ailist GAILIST_AIBOT_DEAD
 *
 * Assigned while the aibot is doing their death animation. Stops after fadeout.
 */
u8 func0028_aibot_dead[] = {

	beginloop(0x0d)
		cmd0176_if_something(/*goto*/ 0x16)
	endloop(0x0d)

	label(0x16)
	set_ailist(CHR_SELF, GAILIST_AIBOT_INIT)
	endlist
};

/**
 * @ailist GAILIST_AIBOT_MAIN
 *
 * Assigned while the aibot is alive.
 */
u8 func002a_aibot_main[] = {
	beginloop(0xf8)
		if_chr_dead(CHR_SELF, /*goto*/ 0xf9)
	endloop(0xf8)

	label(0xf9)
	set_ailist(CHR_SELF, GAILIST_AIBOT_DEAD)
	endlist
};

struct ailist g_GlobalAilists[] = {
	{ func0000_idle,                      GAILIST_IDLE                   },
	{ func0005_end_cinema,                GAILIST_END_CINEMA             },
	{ func0006_unalerted,                 GAILIST_UNALERTED              },
	{ func0007_alerted,                   GAILIST_ALERTED                },
	{ func0008_wakeup,                    GAILIST_WAKEUP                 },
	{ func000a_do_busy_animation,         GAILIST_BUSY                   },
	{ func000b_choose_target_chr,         GAILIST_CHOOSE_TARGET          },
	{ func000c_combat_with_target_chr,    GAILIST_COMBAT_WITH_TARGET     },
	{ func000d_init_combat,               GAILIST_INIT_COMBAT            },
	{ func0016_show_objective_failed_msg, GAILIST_SHOW_OBJ_FAILED_MSG    },
	{ func0017_rebuild_groups,            GAILIST_REBUILD_GROUPS         },
	{ func0018_do_bored_animation,        GAILIST_DO_BORED_ANIMATION     },
	{ func0019_do_sitting_animation,      GAILIST_DO_SITTING_ANIMATION   },
	{ func0000_idle_0009,                 GAILIST_IDLE_0009              },
	{ func001a_patroller_dis_talking,     GAILIST_PATROLLER_DIS_TALKING  },
	{ func000f_hand_combat,               GAILIST_HAND_COMBAT            },
	{ func0010_civilian_say_comment,      GAILIST_CIVILIAN_SAY_COMMENT   },
	{ func0011_flee_from_grenade,         GAILIST_FLEE_FROM_GRENADE      },
	{ func001b_observe_camspy,            GAILIST_OBSERVE_CAMSPY         },
	{ func0020_buddy_warp,                GAILIST_BUDDY_WARP             },
	{ func0012_init_default_buddy,        GAILIST_INIT_DEFAULT_BUDDY     },
	{ func0013_init_pugilist_buddy,       GAILIST_INIT_PUGILIST_BUDDY    },
	{ func0014_buddy_main,                GAILIST_BUDDY_MAIN             },
	{ func001d_search_for_player,         GAILIST_SEARCH_FOR_PLAYER      },
	{ func001e_look_around,               GAILIST_LOOK_AROUND            },
	{ func001f_related_to_spawning,       GAILIST_RELATED_TO_SPAWNING    },
	{ func001c_surprised,                 GAILIST_SURPRISED              },
	{ func0023_dodge,                     GAILIST_DODGE                  },
	{ func0026_init_psychosis,            GAILIST_INIT_PSYCHOSIS         },
	{ func0027_psychosised,               GAILIST_PSYCHOSISED            },
	{ func002d_invincible_and_idle,       GAILIST_INVINCIBLE_AND_IDLE    },
	{ func0021_stop_and_idle,             GAILIST_STOP_AND_IDLE          },
#if VERSION >= VERSION_NTSC_1_0
	{ func0015_buddy_stealth,             GAILIST_BUDDY_STEALTH          },
#endif
	{ func0028_aibot_dead,                GAILIST_AIBOT_DEAD             },
	{ func0029_aibot_init,                GAILIST_AIBOT_INIT             },
	{ func002a_aibot_main,                GAILIST_AIBOT_MAIN             },
	{ NULL },
};

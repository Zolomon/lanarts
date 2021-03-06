/**
 * stats.h:
 *  Represents the various kinds of attacks.
 *  Note that EffectiveStats is first created from CoreStats with all static
 *  information, ie  without attack choice info, and then EffectiveAttackStats
 *  is finally created once an attack is chosen.
 */

#ifndef STATS_H_
#define STATS_H_

#include <cmath>
#include <cstring>

#include "../lanarts_defines.h"

#include "AllowedActions.h"

struct AttackStats;
struct ClassType;
class MTwist;

/* Core combat stats*/
struct CoreStats {
	int hp, max_hp;
	int mp, max_mp;
	int strength, defence, magic, willpower;
	float hpregen, mpregen;

	CoreStats() :
			hp(0), max_hp(0), mp(0), max_mp(0), strength(0), defence(0), magic(
					0), willpower(0), hpregen(0), mpregen(0), hp_regened(0), mp_regened(
					0) {
	}

	void step();

	bool hurt(int dmg);
	void heal_fully();
	void heal_hp(float hpgain);
	void heal_mp(float mpgain);
private:
	float hp_regened, mp_regened;
};

/* Stat multiplier, weighted sum*/
struct CoreStatMultiplier {
	Range base;
	float strength, defence, magic, willpower;
	CoreStatMultiplier(int base = 0, float strength = 0, float defence = 0,
			float magic = 0, float willpower = 0) :
			base(base, base), strength(strength), defence(defence), magic(
					magic), willpower(willpower) {
	}
	Range calculate_range(const CoreStats& stats) const;
	float calculate(MTwist& mt, const CoreStats& stats) const;
};

/* Derived combat stats, power & damage represent properties
 * They are not fully determined until EffectiveAttackStats is created */
struct DerivedStats {
	float power, resistance;
	int damage, reduction;
	DerivedStats(int power = 0, int resistance = 0, int damage = 0,
			int reduction = 0) :
			power(power), resistance(resistance), damage(damage), reduction(
					reduction) {
	}

	bool operator==(const DerivedStats& derived) const;
};

/* Stats related to a chosen attack */
struct EffectiveAttackStats {
	int damage, power, cooldown;
	float magic_percentage;
	float resist_modifier;
	EffectiveAttackStats() :
			damage(0), power(0), cooldown(0), magic_percentage(0.0f), resist_modifier(
					1.0f) {
	}
	float physical_percentage() const {
		return 1.0f - magic_percentage;
	}
};

/* Which actions are allowed ? */
struct ActionsAllowed {
	bool can_use_spell, can_use_item, can_use_weapon;
	ActionsAllowed() :
			can_use_spell(true), can_use_item(true), can_use_weapon(true) {
	}
};

/* Core & derived stats after stat & item properties */
struct EffectiveStats {
	CoreStats core;
	DerivedStats physical, magic;
	float cooldown_mult, movespeed;
	AllowedActions allowed_actions;
	EffectiveStats() :
			cooldown_mult(1.0f), movespeed(0.0f) {
	}

	EffectiveAttackStats with_attack(MTwist& mt,
			const AttackStats& attack) const;
};

/* Cooldown, eg count before a certain action can be done again*/
struct CooldownStats {
	int action_cooldown;
	int pickup_cooldown;
	int rest_cooldown;
	int hurt_cooldown;
	int stopaction_timeout;

	CooldownStats() :
			action_cooldown(0), pickup_cooldown(0), rest_cooldown(0), hurt_cooldown(
					0), stopaction_timeout(0) {
	}

	void step();

	bool can_rest() {
		return rest_cooldown <= 0;
	}
	bool can_pickup() {
		return pickup_cooldown <= 0;
	}
	bool can_doaction() {
		return action_cooldown <= 0;
	}
	bool is_hurting() {
		return hurt_cooldown > 0;
	}
	bool can_do_stopaction() {
		return stopaction_timeout <= 0;
	}

	void reset_action_cooldown(int cooldown);
	void reset_pickup_cooldown(int cooldown);
	void reset_rest_cooldown(int cooldown);
	void reset_hurt_cooldown(int cooldown);
	void reset_stopaction_timeout(int cooldown);
};

/* Represents class related stats */
struct ClassStats {
	class_id classid;
	int xp, xpneeded, xplevel;
	ClassStats(class_id classtype = -1, int xplevel = 0, int xp = 0,
			int xpneeded = 0) :
			classid(classtype), xp(xp), xpneeded(xpneeded), xplevel(xplevel) {
	}
	bool has_class() {
		return classid >= 0;
	}
	ClassType& class_type() const;
};

#endif /* STATS_H_ */

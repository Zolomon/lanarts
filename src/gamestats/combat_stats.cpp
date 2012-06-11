/*
 * combat_stats.cpp:
 *  All the stats used by a combat entity.
 *  TODO: either rename this or 'stats.h' ?
 */


#include "../data/class_data.h"
#include "../data/weapon_data.h"

#include "combat_stats.h"
#include "items.h"
#include "stat_formulas.h"

void CombatStats::step() {
	core.step();
	cooldowns.step();
	effects.step();
}

bool CombatStats::has_died() {
	return core.hp <= 0;
}

EffectiveStats CombatStats::effective_stats(GameState* gs) const {
	return ::effective_stats(gs, *this);
}

void CombatStats::gain_level() {
	ClassType& ct = game_class_data[class_stats.classtype];

	core.hp += ct.hp_perlevel;
	core.max_hp += ct.hp_perlevel;

	core.mp += ct.mp_perlevel;
	core.max_mp += ct.mp_perlevel;

	core.defence += ct.def_perlevel;
	core.strength += ct.str_perlevel;
	core.magic += ct.mag_perlevel;

	core.hpregen += ct.hpregen_perlevel;
	core.mpregen += ct.mpregen_perlevel;

	class_stats.xplevel++;
}

int CombatStats::gain_xp(int amnt) {
	int levels_gained = 0;
	class_stats.xp += amnt;
	while (class_stats.xp >= class_stats.xpneeded) {
		gain_level();
		levels_gained++;
		class_stats.xp -= class_stats.xpneeded;
		class_stats.xpneeded = experience_needed_formula(class_stats.xplevel);
	}
	return levels_gained;
}

bool AttackStats::is_ranged() const {
    return projectile.valid_projectile() || weapon.weapon_entry().uses_projectile;
}

WeaponEntry& AttackStats::weapon_entry() const {
	return weapon.weapon_entry();
}

ProjectileEntry& AttackStats::projectile_entry() const {
    LANARTS_ASSERT(is_ranged());
    if(projectile.valid_projectile())
        return projectile.projectile_entry();

    int created = weapon.weapon_entry().created_projectile;
    LANARTS_ASSERT(created > -1);
    return game_projectile_data.at(created);
}





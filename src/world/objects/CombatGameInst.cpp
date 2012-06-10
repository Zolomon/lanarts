/*
 * CombatGameInst.cpp:
 *  Represents an instance that is affected by combat, ie enemies and players
 */

#include "../../data/sprite_data.h"
#include "../../data/effect_data.h"
#include "../../data/weapon_data.h"

#include "../../gamestats/stat_formulas.h"

#include "../../util/math_util.h"

#include "../GameState.h"

#include "../utility_objects/AnimatedInst.h"
#include "CombatGameInst.h"
#include "ProjectileInst.h"

bool CombatGameInst::damage(GameState* gs, int dmg) {
	if (core_stats().hurt(dmg)) {
		die(gs);
		return true;
	}
	return false;
}

void CombatGameInst::update_field_of_view() {
}

void CombatGameInst::step(GameState* gs) {
	estats = stats().effective_stats(gs);
	stats().step();
}

/* Getters */
CombatStats& CombatGameInst::stats() {
	return base_stats;
}

EffectiveStats& CombatGameInst::effective_stats() {
	return estats;
}

const int HURT_COOLDOWN = 30;

static float hurt_alpha_value(int hurt_cooldown) {
	if (hurt_cooldown < HURT_COOLDOWN / 2)
		return float(hurt_cooldown) / HURT_COOLDOWN / 2 * 0.7f + 0.3f;
	else
		return (HURT_COOLDOWN - hurt_cooldown) / 10 * 0.7f + 0.3f;
}

void CombatGameInst::draw(GameState *gs) {
	GameView& view = gs->window_view();
	SpriteEntry& spr = game_sprite_data[spriteid];
	Colour draw_colour(255, 255, 255);

	if (core_stats().hp < core_stats().max_hp)
		gl_draw_statbar(view, x - 10, y - 20, 20, 5, core_stats().hp,
				core_stats().max_hp);

	int haste_effect = get_effect_by_name("Haste");
	if (effects().get(haste_effect)) {
		Effect* e = effects().get(haste_effect);
		float s = e->t_remaining / 200.0;
		if (s > 1)
			s = 1;
		draw_colour = Colour(255 * (1 - s), 255 * (1 - s), 255);
	} else if (cooldowns().is_hurting()) {
		float s = 1 - hurt_alpha_value(cooldowns().hurt_cooldown);
		draw_colour = Colour(255, 255 * s, 255 * s);
	}
	gl_draw_sprite_entry(view, spr, x - spr.width() / 2, y - spr.height() / 2,
			vx, vy, gs->frame(), draw_colour);

	if (is_resting) {
		GLimage& restimg =
				game_sprite_data[get_sprite_by_name("resting")].img();
		gl_draw_image(view, restimg, x - spr.width() / 2, y - spr.height() / 2);
	}
}

bool CombatGameInst::attack(GameState* gs, CombatGameInst* inst,
		const AttackStats& attack) {
	bool isdead = false;
	if (!cooldowns().can_doaction())
		return false;
	if (attack.is_ranged()) {
		ProjectileEntry& pentry = attack.projectile_entry();

		Pos p(inst->x, inst->y);
		p.x += gs->rng().rand(-12, +13);
		p.y += gs->rng().rand(-12, +13);
		if (gs->tile_radius_test(p.x, p.y, 10)) {
			p.x = inst->x;
			p.y = inst->y;
		}
		//	ProjectileInst(sprite_id sprite, obj_id originator, float speed, int range,
		//			int damage, int x, int y, int tx, int ty, bool bounce = false,
		//			int hits = 1, obj_id target = NONE);
		GameInst* bullet = new ProjectileInst(pentry.attack_sprite, id,
				pentry.speed, pentry.range, 1, x, y, p.x, p.y);
		gs->add_instance(bullet);
		cooldowns().reset_action_cooldown(pentry.cooldown);
		cooldowns().action_cooldown += gs->rng().rand(-4, 5);
	} else {
		WeaponEntry& wentry = attack.weapon_entry();

		int damage = physical_damage_formula(effective_stats(),
				inst->effective_stats());
		if (!gs->game_settings().invincible) {
			isdead = inst->damage(gs, damage);
		}

		char dmgstr[32];
		snprintf(dmgstr, 32, "%d", damage);
		float rx, ry;
		direction_towards(Pos(x, y), Pos(inst->x, inst->y), rx, ry, 0.5);
		gs->add_instance(
				new AnimatedInst(inst->x - 5 + rx * 5, inst->y + ry * 5, -1, 25,
						rx, ry, dmgstr, Colour(255, 148, 120)));

		cooldowns().reset_action_cooldown(wentry.cooldown);
		cooldowns().action_cooldown += gs->rng().rand(-4, 5);

		if (wentry.name != "none") {
			gs->add_instance(
					new AnimatedInst(inst->x, inst->y, wentry.attack_sprite,
							25));
		}
	}
	return isdead;
}

void CombatGameInst::init(GameState* gs) {
	estats = stats().effective_stats(gs);
}

team_id& CombatGameInst::team() {
	return teamid;
}

ClassStats& CombatGameInst::class_stats() {
	return stats().class_stats;
}

CooldownStats& CombatGameInst::cooldowns() {
	return stats().cooldowns;
}

EffectStats& CombatGameInst::effects() {
	return stats().effects;
}

CoreStats& CombatGameInst::core_stats() {
	return stats().core;
}

_Inventory& CombatGameInst::inventory() {
	return stats().equipment.inventory;
}

_Equipment& CombatGameInst::equipment() {
	return stats().equipment;
}
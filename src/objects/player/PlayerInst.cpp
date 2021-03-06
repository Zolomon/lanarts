/*
 * PlayerInst.cpp
 *  Implements the standard GameInst actions such as step, draw, etc for PlayerInst
 */

#include <typeinfo>

#include "../AnimatedInst.h"

#include "../../gamestate/GameState.h"
#include "../../display/sprite_data.h"
#include "../../display/tile_data.h"
#include "../../stats/class_data.h"
#include "../../stats/item_data.h"
#include "../../stats/weapon_data.h"
#include "../../display/display.h"

#include "../../serialize/SerializeBuffer.h"

#include "../../util/math_util.h"

#include "../enemy/EnemyInst.h"
#include "../ItemInst.h"
#include "PlayerInst.h"
#include "../ProjectileInst.h"

PlayerInst::PlayerInst(const CombatStats& stats, sprite_id sprite, int x, int y,
		bool local) :
		CombatGameInst(stats, sprite, NONE, -1, x, y, RADIUS, true, DEPTH), actions_set_for_turn(
				false), fieldofview(LINEOFSIGHT), local(local), moving(0), autouse_mana_potion_try_count(
				0), lives(0), deaths(0), previous_spellselect(0), spellselect(
				-1) {
}

void PlayerInst::init(GameState* gs) {
	CombatGameInst::init(gs);
	teamid = gs->teams().default_player_team();
	_path_to_player.calculate_path(gs, x, y, PLAYER_PATHING_RADIUS);
	collision_simulation_id() = gs->collision_avoidance().add_player_object(
			this);
}

PlayerInst::~PlayerInst() {
}

void PlayerInst::update_field_of_view(GameState* gs) {
	int sx = last_x / TILE_SIZE;
	int sy = last_y / TILE_SIZE;
	field_of_view().calculate(gs, sx, sy);
}

bool PlayerInst::within_field_of_view(const Pos& pos) {
	return fieldofview.within_fov(pos.x / TILE_SIZE, pos.y / TILE_SIZE);
}

void PlayerInst::die(GameState* gs) {
	//Let step event handle death
}

void PlayerInst::deinit(GameState* gs) {
	CombatGameInst::deinit(gs);
	current_target = NONE;
	gs->collision_avoidance().remove_object(collision_simulation_id());
}

static Pos seen_square_in_area(MTwist& mt, GameTiles& tiles) {
	Pos p, ret;
	do {
		p.x = mt.rand(tiles.tile_width());
		p.y = mt.rand(tiles.tile_height());
	} while (!tiles.is_seen(p.x, p.y) || tiles.is_solid(p.x, p.y));
	return centered_multiple(p, TILE_SIZE);
}
static void spawn_in_lower_level(GameState* gs, PlayerInst* player) {
	int levelid1 = gs->game_world().get_current_level_id();
	int levelid2 = (levelid1 > 0) ? levelid1 - 1 : 0;
	GameLevelState* level2 = gs->game_world().get_level(levelid2);
	Pos sqr = seen_square_in_area(gs->rng(), level2->tiles());

	for (int i = 0; i < gs->player_data().all_players().size(); i++) {
		PlayerDataEntry& pde = gs->player_data().all_players()[i];
		if (pde.player_inst.get_instance() == player) {
			pde.action_queue.clear_actions();
		}
	}

	if (player->is_local_player()) {
		gs->game_chat().add_message("You have respawned!",
				Colour(100, 150, 150));
	} else {
		gs->game_chat().add_message("Your ally has respawned!",
				Colour(100, 150, 150));
	}
	if (levelid1 != levelid2) {
		gs->game_world().level_move(player->id, sqr.x, sqr.y, levelid1,
				levelid2);
	} else {
		player->update_position(sqr.x, sqr.y);
	}
}

//Either finds new or shifts target
void PlayerInst::shift_autotarget(GameState* gs) {
	const std::vector<obj_id>& mids = gs->monster_controller().monster_ids();
	int i = 0, j = 0;
	bool has_target = current_target != NONE;
	if (has_target) {
		for (i = 0; i < mids.size(); i++) {
			if (mids[i] == current_target) {
				break;
			}
		}
		j = i + 1;
	}

	while (true) {
		if (j >= mids.size()) {
			// Exit when we end the list if we didn't have a target
			if (!has_target) {
				return;
			}
			j -= mids.size();
		}
		if (j == i && has_target) {
			// Exit when we wrap around if we have a target
			return;
		}
		EnemyInst* e = (EnemyInst*) gs->get_instance(mids[j]);
		bool isvisible = e != NULL && gs->object_visible_test(e, this, false);
		if (isvisible) {
			current_target = e->id;
			return;
		}
		j++;

	}

}

void PlayerInst::step(GameState* gs) {
	_path_to_player.calculate_path(gs, x, y, PLAYER_PATHING_RADIUS);

	GameInst* target_inst = gs->get_instance(current_target);
	bool visible = target_inst != NULL
			&& gs->object_visible_test(target_inst, this, false);
	if (!visible) {
		current_target = NONE;
	}
	if (current_target == NONE) {
		shift_autotarget(gs);
	}

	if (!actions_set_for_turn) {
		printf("No actions for turn player id %d\n", id);
		return;
	}

	vx = 0, vy = 0;

	CombatGameInst::step(gs);
	GameView& view = gs->view();

	enqueue_network_actions(gs);

	//Stats/effect step
	if (cooldowns().is_hurting())
		reset_rest_cooldown();

	if (stats().has_died()) {
		deaths++;
//		if (is_local_focus())
		queued_actions.clear();
		actions_set_for_turn = false;
		if (gs->game_settings().regen_on_death) {
			stats().core.heal_fully();
			spawn_in_lower_level(gs, this);
		} else {
			gs->game_world().reset(0);
		}
//		else
//			gs->remove_instance(this);
		return;
	}

	is_resting = false;
	perform_queued_actions(gs);

	if (!gs->key_down_state(SDLK_x) && is_local_player())
		view.center_on(last_x, last_y);

	update_position(rx + vx, ry + vy);
}

void PlayerInst::draw(GameState* gs) {
	CombatGameInst::draw(gs);
}

void PlayerInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(PlayerInst*) inst = *this;
}

void PlayerInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	CombatGameInst::serialize(gs, serializer);
	serializer.write(actions_set_for_turn);
//	serializer.write_container(queued_actions);

	SERIALIZE_POD_REGION(serializer, this, local, spellselect);
}

void PlayerInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	CombatGameInst::deserialize(gs, serializer);
	serializer.read(actions_set_for_turn);
//	serializer.read_container(queued_actions);
	queued_actions.clear();
	_path_to_player.calculate_path(gs, x, y, PLAYER_PATHING_RADIUS);
	update_field_of_view(gs);
	DESERIALIZE_POD_REGION(serializer, this, local, spellselect);

	CollisionAvoidance& coll_avoid = gs->collision_avoidance();
	collision_simulation_id() = coll_avoid.add_player_object(this);
}

PlayerInst *PlayerInst::clone() const {
	return new PlayerInst(*this);
}


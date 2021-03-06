/*
 * EquipmentContent.cpp:
 *  Represents an interactive view of equipped items for the side bar
 */

#include "../../stats/item_data.h"
#include "../../stats/weapon_data.h"

#include "../../display/sprite_data.h"

#include "../../display/display.h"

#include "../../display/colour_constants.h"

#include "../console_description_draw.h"

#include "../../gamestate/GameState.h"

#include "../../objects/player/PlayerInst.h"

#include "EquipmentContent.h"

EquipmentContent::~EquipmentContent() {
}

static BBox weapon_slot_bbox(GameState* gs, const BBox& bbox) {
	BBox entry_box(bbox.x1, bbox.y1, bbox.x2, bbox.y1 + TILE_SIZE);
	if (gs->local_player()->equipment().has_projectile()) {
		entry_box.y2 += TILE_SIZE;
	}
	return entry_box;
}
static BBox armour_slot_bbox(GameState* gs, const BBox& bbox) {
	BBox weapon_bbox = weapon_slot_bbox(gs, bbox);
	return BBox(weapon_bbox.x1, weapon_bbox.y2, weapon_bbox.x2,
			weapon_bbox.y2 + TILE_SIZE);
}

static void draw_weapon(GameState* gs, Equipment& eqp, const BBox& bbox) {
	Colour bbox_col = COL_FILLED_OUTLINE;
	if (bbox.contains(gs->mouse_pos())) {
		bbox_col = COL_WHITE;
		draw_console_item_description(gs, eqp.weapon.as_item());
	}

	draw_item_icon_and_name(gs, eqp.weapon.item_entry(), COL_WHITE, bbox.x1,
			bbox.y1);
	if (eqp.has_projectile()) {
		draw_item_icon_and_name(gs, eqp.projectile.item_entry(), COL_WHITE,
				bbox.x1, bbox.y1 + TILE_SIZE);
		gl_draw_line(bbox.x1, bbox.center_y(), bbox.x2, bbox.center_y(),
				COL_UNFILLED_OUTLINE);
	}
	gl_draw_rectangle_outline(bbox, bbox_col);
}

static void draw_armour(GameState* gs, Equipment& eqp, BBox bbox) {
	Colour bbox_col = COL_UNFILLED_OUTLINE;
	if (eqp.has_armour()) {
		bbox_col = COL_FILLED_OUTLINE;
		if (bbox.contains(gs->mouse_pos())) {
			bbox_col = COL_WHITE;
			draw_console_item_description(gs, eqp.armour.as_item());
		}
	}

	draw_item_icon_and_name(gs, eqp.armour.item_entry(), COL_WHITE, bbox.x1,
			bbox.y1);
	gl_draw_rectangle_outline(bbox, bbox_col);
}

void EquipmentContent::draw(GameState* gs) const {
	PlayerInst* p = gs->local_player();

	Equipment& eqp = p->equipment();

	gl_draw_rectangle_outline(bbox, COL_UNFILLED_OUTLINE);
	draw_weapon(gs, eqp, weapon_slot_bbox(gs, bbox));
	draw_armour(gs, eqp, armour_slot_bbox(gs, bbox));
}

bool EquipmentContent::handle_io(GameState* gs, ActionQueue& queued_actions) {
	PlayerInst* p = gs->local_player();
	BBox weapon_bbox(weapon_slot_bbox(gs, bbox));
	// Check if we are de-equipping our weapon
	Pos mouse = gs->mouse_pos();
	if (gs->mouse_right_click() && weapon_bbox.contains(mouse)) {
		queued_actions.push_back(
				game_action(gs, p, GameAction::DEEQUIP_ITEM,
						ItemEntry::WEAPON));
	}

	// Check if we are de-equipping our armour
	BBox armour_bbox(armour_slot_bbox(gs, bbox));
	if (gs->mouse_right_click() && armour_bbox.contains(mouse)) {
		queued_actions.push_back(
				game_action(gs, p, GameAction::DEEQUIP_ITEM,
						ItemEntry::ARMOUR));
	}

	return false;
}


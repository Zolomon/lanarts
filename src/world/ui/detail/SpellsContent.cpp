/*
 * SpellsContent.cpp:
 *  Draws known spells in a grid, for the side bar
 */

#include "SpellsContent.h"

#include "../../../data/spell_data.h"
#include "../../../data/sprite_data.h"

#include "../../../display/display.h"

#include "../../../gamestats/SpellsKnown.h"

#include "../../../util/colour_constants.h"

#include "../../GameState.h"

#include "../../objects/PlayerInst.h"

static void draw_spell_icon(spell_id spell, int x, int y) {
	SpellEntry& spl_entry = game_spell_data.at(spell);
	gl_draw_sprite(spl_entry.sprite, x, y);
}

static void draw_icon_and_name(GameState* gs, SpellEntry& spl_entry, Colour col,
		int x, int y) {
	gl_draw_sprite(spl_entry.sprite, x, y);
	/* Draw spell name */
	gl_printf(gs->primary_font(), col, x + TILE_SIZE + 5, y + TILE_SIZE / 2 - 4,
			"%s", spl_entry.name.c_str());
}

static void draw_stat_text(GameState* gs, int x, int y, const char* prefix,
		Colour prefix_col, int stat, Colour stat_col) {

	Pos p = gl_printf_y_centered(gs->primary_font(), prefix_col, x, y, "%s", prefix);
	gl_printf_y_centered(gs->primary_font(), stat_col, p.x + x, y, "%d", stat);
}

static void draw_console_spell_description(GameState* gs,
		SpellEntry& spl_entry) {
	GameTextConsole& console = gs->game_console();

	if (console.has_content_already()) {
		return;
	}

	console.draw_box(gs);
	BBox bbox(console.bounding_box());
	draw_icon_and_name(gs, spl_entry, Colour(PALE_YELLOW), bbox.x1, bbox.y1);
	draw_stat_text(gs, bbox.center_x(), bbox.y1 + TILE_SIZE/2, "MP cost: ", Colour(),
			spl_entry.mp_cost, Colour());

	const int MAX_WIDTH = bbox.width() - TILE_SIZE;

	gl_printf_bounded(gs->primary_font(), LIGHT_GRAY, bbox.x1 + TILE_SIZE + 5,
			bbox.y1 + TILE_SIZE + 5, MAX_WIDTH, "%s",
			spl_entry.description.c_str());
}

static void draw_spells_known(GameState* gs, const BBox& bbox,
		SpellsKnown& spells, int ind_low, int ind_high) {
	const int spell_n = spells.amount();
	int mx = gs->mouse_x(), my = gs->mouse_y();
	int spellidx = ind_low;

	gl_draw_rectangle_outline(bbox, COL_UNFILLED_OUTLINE);

	int x = bbox.x1, ex = bbox.x2;
	for (int y = bbox.y1; y < bbox.y2; y += TILE_SIZE) {
		if (spellidx >= spell_n)
			break;

		spell_id spell = spells.get(spellidx);
		SpellEntry& spl_entry = game_spell_data.at(spell);
		draw_icon_and_name(gs, spl_entry, Colour(), x, y);

		BBox entry_box(x, y, ex - 2, y + TILE_SIZE);
		if (entry_box.contains(mx, my)) {
			gl_draw_rectangle_outline(entry_box, COL_FILLED_OUTLINE);
			draw_console_spell_description(gs, spl_entry);
		}
		spellidx++;
	}

//	for (int y = bbox.y1; y < bbox.y2; y += TILE_SIZE) {
//		for (int x = bbox.x1; x < bbox.x2; x += TILE_SIZE) {
//			if (spellidx >= ind_high)
//				break;
//
//			bool filledslot = spellidx < spell_n;
//			Colour outline = filledslot ? COL_FILLED_OUTLINE : COL_UNFILLED_OUTLINE;
//
//			gl_draw_rectangle_outline(x, y, TILE_SIZE, TILE_SIZE, outline);
//
//			if (filledslot) {
//				draw_spell_icon(spells.get(spellidx), x, y);
//			}
//
//			spellidx++;
//		}
//	}
}

const int SPELLS_PER_PAGE = 40;

void SpellsContent::draw(GameState* gs) const {
	PlayerInst* p = gs->local_player();

	int min_spell = SPELLS_PER_PAGE * page_number, max_spell = min_spell
			+ SPELLS_PER_PAGE;
	gl_printf(gs->primary_font(), COL_FILLED_OUTLINE, bbox.center_x() - 15,
			bbox.y2 + 3, "Spells");
	draw_spells_known(gs, bbox, p->spells_known(), min_spell, max_spell);
}

int SpellsContent::amount_of_pages(GameState* gs) {
	PlayerInst* p = gs->local_player();

	SpellsKnown& spells = p->spells_known();
	int spells_n = spells.amount();
	/* Add SPELLS_PER_PAGE - 1 so that 0 spells need 0 pages, 1 spell needs 1 page, etc*/
	int spell_pages = (spells_n + SPELLS_PER_PAGE - 1) / SPELLS_PER_PAGE;

	return spell_pages;
}


#include "PlayerInst.h"
#include "BulletInst.h"
#include "EnemyInst.h"
#include "ItemInst.h"
#include "AnimatedInst.h"

#include "../../util/draw_util.h"
#include "../GameState.h"
#include "../../data/sprite_data.h"
#include "../../data/tile_data.h"
#include "../../display/display.h"
#include "../../data/item_data.h"
#include "TestInst.h"

PlayerInst::~PlayerInst() {
}

void PlayerInst::init(GameState* gs) {
	PlayerController& pc = gs->player_controller();
	pc.register_player(this->id);
	portal = NULL;
}

void PlayerInst::step(GameState* gs) {

	GameView& view = gs->window_view();

	bool mouse_within = gs->mouse_x() < gs->window_view().width;

	//Stats/effect step
	stats().step();
	effects.step();

	if (stats().hp <= 0) {
		gs->branch_level() = 1;
		gs->set_generate_flag();
		return;
	}

	if (stats().hurt_cooldown > 0)
		canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
	if (--canrestcooldown < 0)
		canrestcooldown = 0;

	perform_io_action(gs);

	if (gs->key_down_state(SDLK_x)) {
		int nx = gs->mouse_x() + view.x, ny = gs->mouse_y() + view.y;
		view.center_on(nx, ny);
	 } else
		 view.center_on(last_x, last_y);
}

void PlayerInst::draw(GameState* gs) {
	GameView& view = gs->window_view();
	GLImage& img = spr_player.img;
	bool b = gs->tile_radius_test(x, y, RADIUS);
	//gl_draw_rectangle(view, x-10,y-20,20,5, b ? Colour(255,0,0) : Colour(0,255,0));
	//gl_draw_circle(view, x,y,RADIUS);
	if (stats().hp < stats().max_hp)
		gl_draw_statbar(view, x - 10, y - 20, 20, 5, stats().hp,
				stats().max_hp);

	if (effects.get(EFFECT_HASTE)) {
		effect* e = effects.get(EFFECT_HASTE);
		float s = e->t_remaining/200;
		if (s> 1) s = 1;
		Colour blue(255*(1-s), 255*(1-s), 255);
		image_display(&img, x - img.width / 2 - view.x,
				y - img.height / 2 - view.y, blue);
	}else if (stats().hurt_cooldown > 0) {
		float s = 1 - stats().hurt_alpha() ;
		Colour red(255, 255*s, 255*s);
		image_display(&img, x - img.width / 2 - view.x,
				y - img.height / 2 - view.y, red);
	}else {
		image_display(&img, x - img.width / 2 - view.x,
				y - img.height / 2 - view.y);
	}


	//for (int i = 0; i < 10; i++)
//	gl_printf(gs->primary_font(), Colour(255,255,255), x-10-view.x, y-30-view.y, "id=%d", this->id);
//	gl_printf(gs->primary_font(), Colour(255,255,255), gs->mouse_x(), gs->mouse_y(),
//			"mx=%d,my=%d\nx=%d,y=%d",
//			gs->mouse_x(), gs->mouse_y(),
//			gs->mouse_x()+view.x, gs->mouse_y()+view.y);

//	gs->monster_controller().paths[0].draw(gs);
}

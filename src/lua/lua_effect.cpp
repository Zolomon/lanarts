#include <cstring>
#include <lua/lunar.h>

#include "lua_api.h"

#include "../world/GameState.h"
#include "../world/objects/EnemyInst.h"
#include "../world/objects/GameInst.h"
#include "../world/objects/PlayerInst.h"
#include "../gamestats/Stats.h"

#include "../data/item_data.h"
#include "../data/effect_data.h"
#include "../data/weapon_data.h"


class EffectsLuaBinding {
public:
  static const char className[];
  static Lunar<EffectsLuaBinding>::RegType methods[];

  EffectsLuaBinding(int effectnum) : effectnum(effectnum) {
  }

  EffectType& get_effects(){
	  return game_effect_data[effectnum];
  }
private:
  int effectnum;
};

typedef EffectsLuaBinding bind_t;
typedef Lunar<EffectsLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)


meth_t bind_t::methods[] = {
  meth_t(0,0)
};

void lua_pusheffects(lua_State* L, int effectnum){
	lunar_t::push(L, new EffectsLuaBinding(effectnum), true);
}

EffectType& lua_effects_arg(lua_State* L, int narg){
	bind_t* bind = lunar_t::check(L, narg);
	return bind->get_effects();
}

void lua_effects_bindings(GameState* gs, lua_State* L){
	lunar_t::Register(L);
     luaL_getmetatable(L, bind_t::className);
     lua_newtable(L);
     int top = lua_gettop(L);
     for (int i = 0; i < game_effect_n; i++) {
    	    lua_pushstring(L, game_effect_data[i].name);
    	    lua_pusheffects(L, i);
    	    lua_settable(L, top);
     }
     lua_setglobal(L, "effects");
//

}

const char EffectsLuaBinding::className[] = "Effects";
extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "tests.h"
#include "../lua/LuaValue.h"
#include "../serialize/SerializeBuffer.h"

static lua_State* L;

static void lua_serialize_number() {
	SerializeBuffer serializer = SerializeBuffer::plain_buffer();
	const double testnumber = 1.1;

	LuaValue value, result;
	lua_pushnumber(L, testnumber);

	value.pop(L);
	value.serialize(L, serializer);

	result.deserialize(L, serializer);
	result.push(L);

	UNIT_TEST_ASSERT(lua_isnumber(L, -1));
	UNIT_TEST_ASSERT(lua_tonumber(L, -1) == testnumber);

	lua_pop(L, 1);
}

static void lua_serialize_table() {
	SerializeBuffer serializer = SerializeBuffer::plain_buffer();
	const double testnumber = 1.1;
	const char* fieldname = "myfield";

	// Push {myfield = 1.1}
	lua_newtable(L);
	lua_pushnumber(L, testnumber);
	lua_setfield(L, -2, fieldname);

	{
		//serialize it
		LuaValue value, result;
		value.pop(L);
		value.serialize(L, serializer);
		//deserialize it
		result.deserialize(L, serializer);
		result.push(L);
	}

	UNIT_TEST_ASSERT(lua_istable(L, -1));
	lua_getfield(L, -1, fieldname);
	double tablevalue = lua_tonumber(L, -1);
	UNIT_TEST_ASSERT(lua_tonumber(L, -1) == testnumber);

	lua_pop(L, 2);
}
static void lua_serialize_novalue() {
	SerializeBuffer serializer = SerializeBuffer::plain_buffer();
	LuaValue value, result;
	value.serialize(L, serializer);
	result.deserialize(L, serializer);
	result.push(L);
	UNIT_TEST_ASSERT(lua_isnil(L, -1));

}
static void lua_serialize_nested_table() {
	SerializeBuffer serializer = SerializeBuffer::plain_buffer();
	const double testnumber = 1.1;
	const char* fieldname = "myfield";

	// Push {{myfield = 1.1}}
	lua_newtable(L);
	lua_newtable(L);
	lua_pushnumber(L, testnumber);
	lua_setfield(L, -2, fieldname);
	lua_setfield(L, -2, fieldname);

	{
		//serialize it
		LuaValue value, result;
		value.pop(L);
		value.serialize(L, serializer);
		result.deserialize(L, serializer);
		result.push(L);
	}

	UNIT_TEST_ASSERT(lua_istable(L, -1));
	lua_getfield(L, -1, fieldname);
	UNIT_TEST_ASSERT(lua_istable(L, -1));
	lua_getfield(L, -1, fieldname);
	double tablevalue = lua_tonumber(L, -1);
	UNIT_TEST_ASSERT(lua_tonumber(L, -1) == testnumber);

	lua_pop(L, 3);
}

void lua_serialize_tests(lua_State* LS) {
	L = LS;
	UNIT_TEST(lua_serialize_novalue);
	UNIT_TEST(lua_serialize_number);
	UNIT_TEST(lua_serialize_table);
	UNIT_TEST(lua_serialize_nested_table);
	L = NULL;
}

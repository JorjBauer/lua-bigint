#ifndef BIGINT_GLUE_H
#define BIGINT_GLUE_H

#include <lua.h>

int bigint_new(lua_State *L);
int bigint_destroy(lua_State *L);
int bigint_tostring(lua_State *L);
int bigint_raw(lua_State *L);
int bigint_tonumber(lua_State *L);
int bigint_concat(lua_State *L);
int bigint_add(lua_State *L);
int bigint_sub(lua_State *L);
int bigint_mul(lua_State *L);
int bigint_div(lua_State *L);
int bigint_mod(lua_State *L);
int bigint_pow(lua_State *L);
int bigint_negate(lua_State *L);
int bigint_equal(lua_State *L);
int bigint_lt(lua_State *L);
int bigint_le(lua_State *L);
int bigint_expmod(lua_State *L);
int bigint_inv(lua_State *L);
int bigint_gcd(lua_State *L);
int bigint_shiftleft(lua_State *L);
int bigint_shiftright(lua_State *L);

#endif

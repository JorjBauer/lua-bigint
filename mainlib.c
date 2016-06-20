/*
 * Copyright (c) 2015 Jorj Bauer
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */


#include <lua.h>
#include <lauxlib.h>
#include <assert.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

#include "bigint-glue.h"

/* metatable for pointer userdata */
static const luaL_Reg pointer_meta[] = {
  { "__gc", bigint_destroy },
  { "__tostring", bigint_tostring },
  { "__concat", bigint_concat },
  { "__add", bigint_add },
  { "__sub", bigint_sub },
  { "__mul", bigint_mul },
  { "__div", bigint_div },
  { "__mod", bigint_mod },
  { "__pow", bigint_pow },
  { "__unm", bigint_negate },
  { "__eq", bigint_equal },
  { "__lt", bigint_lt },
  { "__le", bigint_le },
  {NULL,    NULL       }
};

/* function table for this module */
static const struct luaL_Reg methods[] = {
  { "new",          bigint_new                  },
  { "tonumber",     bigint_tonumber             },
  { "tostring",     bigint_tostring             },
  { "raw",          bigint_raw                  },
  { "expmod",       bigint_expmod               },
  { "inv",          bigint_inv                  },
  { "gcd",          bigint_gcd                  },
  { "shiftleft",    bigint_shiftleft            },
  { "shiftright",   bigint_shiftright           },
  { NULL,           NULL                        }
};

/* Register a metatable with the given name, and return it on the stack */
int _register_metatable(lua_State *L, const char *name,
			 const struct luaL_Reg meta[])
{
  if (!luaL_newmetatable(L, name)) {
    // If it's already registered, then just return it.
    return 1;
  }

#if LUA_VERSION_NUM == 501
  luaL_openlib(L, 0, meta, 0);
#else
  luaL_setfuncs(L, meta, 0);
#endif

  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -3);
  lua_rawset(L, -3);
  lua_pushliteral(L, "__metatable");
  lua_pushvalue(L, -3);
  lua_rawset(L, -3);

  return 1;
}


/* Module initializer, called from Lua when the module is loaded. */
int luaopen_bigint(lua_State *L)
{
  _register_metatable(L, BIGINT_POINTER, pointer_meta);

  /* Construct a new namespace table for Lua and return it. */
#if LUA_VERSION_NUM == 501
  /* Lua 5.1: pollute the root namespace */
  luaL_openlib(L, "bigint", methods, 0);
#else
  /* Lua 5.2 and above: be a nice namespace citizen */
  lua_newtable(L);
  luaL_setfuncs(L, methods, 0);
#endif

  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -2);           // dup methods
  lua_rawset(L, -4);              // metatable.__index = methods; pops 2


  return 1;

}


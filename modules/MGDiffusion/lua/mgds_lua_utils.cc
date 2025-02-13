#include "mgds_lua_utils.h"

#define LUA_FMACRO1(x) lua_register(L, #x, x)
#define LUA_CMACRO1(x,y) \
        lua_pushnumber(L, y); \
        lua_setglobal(L, #x)

void mg_diffusion::mgd_lua_utils::RegisterLuaEntities(lua_State *L)
{
  LUA_FMACRO1(chiCFEMMGDiffusionSolverCreate);
  LUA_FMACRO1(chiCFEMMGDiffusionSetBCProperty);

//  LUA_CMACRO1(MAX_ITERATIONS, 1);
//  LUA_CMACRO1(TOLERANCE     , 2);
}
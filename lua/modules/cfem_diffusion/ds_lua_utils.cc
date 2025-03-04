#include "ds_lua_utils.h"

#define LUA_FMACRO1(x) lua_register(L, #x, x)
#define LUA_CMACRO1(x, y)                                                                          \
  lua_pushnumber(L, y);                                                                            \
  lua_setglobal(L, #x)

namespace opensnlua::cfem_diffusion
{

void
RegisterLuaEntities(lua_State* L)
{
  LUA_FMACRO1(CFEMDiffusionSolverCreate);
  LUA_FMACRO1(CFEMDiffusionSetBCProperty);

  LUA_CMACRO1(MAX_ITERATIONS, 1);
  LUA_CMACRO1(TOLERANCE, 2);
}

} // namespace opensnlua::cfem_diffusion

#include "framework/lua.h"
#include "framework/field_functions/interpolation/ffinterpolation.h"
#include "framework/runtime.h"
#include "ffinterpol_lua.h"
#include "framework/console/console.h"

RegisterLuaFunctionAsIs(FFInterpolationInitialize);
RegisterLuaFunctionAsIs(FFInterpolationExecute);

int
FFInterpolationInitialize(lua_State* L)
{
  const std::string fname = __FUNCTION__;
  const int num_args = lua_gettop(L);
  if (num_args != 1)
    LuaPostArgAmountError(fname, 1, num_args);

  // Get handle to field function
  const size_t ffihandle = lua_tonumber(L, 1);

  auto p_ffi = opensn::GetStackItemPtr(opensn::field_func_interpolation_stack, ffihandle, fname);

  p_ffi->Initialize();
  return 0;
}

int
FFInterpolationExecute(lua_State* L)
{
  const std::string fname = __FUNCTION__;
  const int num_args = lua_gettop(L);
  if (num_args != 1)
    LuaPostArgAmountError(fname, 1, num_args);

  // Get handle to field function
  const size_t ffihandle = lua_tonumber(L, 1);

  auto p_ffi = opensn::GetStackItemPtr(opensn::field_func_interpolation_stack, ffihandle, fname);

  p_ffi->Execute();
  return 0;
}

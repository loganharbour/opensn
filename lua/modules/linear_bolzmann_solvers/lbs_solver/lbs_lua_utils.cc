#include "lbs_common_lua_functions.h"

#define RegisterFunction(x) lua_register(L, #x, x)

#define RegisterNumber(x, y)                                                                       \
  lua_pushnumber(L, y);                                                                            \
  lua_setglobal(L, #x)

#define RegisterTable(x)                                                                           \
  lua_newtable(L);                                                                                 \
  lua_setglobal(L, #x)

#define RegisterNumberValueToTable(const_name, const_value, namespace_name)                        \
  lua_getglobal(L, #namespace_name);                                                               \
  lua_pushstring(L, #const_name);                                                                  \
  lua_pushnumber(L, const_value);                                                                  \
  lua_settable(L, -3);                                                                             \
  lua_pop(L, 1)

using namespace opensn;

namespace opensnlua::lbs
{
void
RegisterLuaEntities(lua_State* L)
{
  RegisterFunction(LBSSetProperty);
  RegisterNumber(DISCRETIZATION_METHOD, 1);
  RegisterNumber(PWLD, 3);
  RegisterNumber(PWLD1D, 4);
  RegisterNumber(PWLD2D, 5);
  RegisterNumber(PWLD3D, 6);
  RegisterNumber(PARTITION_METHOD, 2);
  RegisterNumber(SERIAL, 1);
  RegisterNumber(FROM_SURFACE, 2);
  RegisterNumber(BOUNDARY_CONDITION, 3);
  RegisterNumber(XMAX, 31);
  RegisterNumber(XMIN, 32);
  RegisterNumber(YMAX, 33);
  RegisterNumber(YMIN, 34);
  RegisterNumber(ZMAX, 35);
  RegisterNumber(ZMIN, 36);
  RegisterNumber(SCATTERING_ORDER, 4);
  RegisterNumber(MAX_MPI_MESSAGE_SIZE, 5);
  RegisterNumber(READ_RESTART_DATA, 6);
  RegisterNumber(WRITE_RESTART_DATA, 7);
  RegisterNumber(SAVE_ANGULAR_FLUX, 8);
  RegisterNumber(USE_SOURCE_MOMENTS, 9);
  RegisterNumber(VERBOSE_INNER_ITERATIONS, 10);
  RegisterNumber(VERBOSE_OUTER_ITERATIONS, 11);
  RegisterNumber(USE_PRECURSORS, 12);

  RegisterTable(LBSBoundaryTypes);
  RegisterNumberValueToTable(VACUUM, 1, LBSBoundaryTypes);
  RegisterNumberValueToTable(INCIDENT_ISOTROPIC, 2, LBSBoundaryTypes);
  RegisterNumberValueToTable(REFLECTING, 3, LBSBoundaryTypes);
  RegisterNumberValueToTable(INCIDENT_ANISTROPIC_HETEROGENEOUS, 4, LBSBoundaryTypes);

  RegisterTable(LBSGroupset);
  RegisterFunction(LBSCreateGroupset);
  RegisterFunction(LBSCreateGroup);
  RegisterFunction(LBSGroupsetAddGroups);
  RegisterFunction(LBSGroupsetSetQuadrature);
  RegisterFunction(LBSGroupsetSetAngleAggregationType);
  RegisterNumberValueToTable(ANGLE_AGG_SINGLE, 1, LBSGroupset);
  RegisterNumberValueToTable(ANGLE_AGG_POLAR, 2, LBSGroupset);
  RegisterNumberValueToTable(ANGLE_AGG_AZIMUTHAL, 3, LBSGroupset);
  RegisterFunction(LBSGroupsetSetAngleAggDiv);
  RegisterFunction(LBSGroupsetSetGroupSubsets);
  RegisterFunction(LBSGroupsetSetIterativeMethod);
  RegisterNumber(KRYLOV_RICHARDSON, 5);
  RegisterNumber(KRYLOV_RICHARDSON_CYCLES, 6);
  RegisterNumber(KRYLOV_GMRES, 7);
  RegisterNumber(KRYLOV_GMRES_CYCLES, 8);
  RegisterNumber(KRYLOV_BICGSTAB, 9);
  RegisterNumber(KRYLOV_BICGSTAB_CYCLES, 10);
  RegisterFunction(LBSGroupsetSetResidualTolerance);
  RegisterFunction(LBSGroupsetSetMaxIterations);
  RegisterFunction(LBSGroupsetSetGMRESRestartIntvl);
  RegisterFunction(LBSGroupsetSetEnableSweepLog);
  RegisterFunction(LBSGroupsetSetWGDSA);
  RegisterFunction(LBSGroupsetSetTGDSA);

  RegisterFunction(LBSGetScalarFieldFunctionList);

  RegisterFunction(LBSWriteGroupsetAngularFlux);
  RegisterFunction(LBSReadGroupsetAngularFlux);

  RegisterFunction(LBSWriteFluxMoments);
  RegisterFunction(LBSCreateAndWriteSourceMoments);
  RegisterFunction(LBSReadFluxMomentsAndMakeSourceMoments);
  RegisterFunction(LBSReadSourceMoments);
  RegisterFunction(LBSReadFluxMoments);

  RegisterFunction(LBSComputeFissionRate);
  RegisterFunction(LBSInitializeMaterials);

  RegisterFunction(LBSAddPointSource);
  RegisterFunction(LBSClearPointSources);
}
} // namespace opensnlua::lbs

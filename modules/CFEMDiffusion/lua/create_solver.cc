#include "chi_lua.h"

#include"../cfem_diffusion_solver.h"

#include "chi_runtime.h"
#include "chi_log.h"

namespace cfem_diffusion::cfem_diffusion_lua_utils
{

//#############################################################################
/** Creates a CFEM Diffusion solver.

\return Handle int Handle to the created solver.
\ingroup LuaDiffusion
*/
int chiCFEMDiffusionSolverCreate(lua_State *L)
{
  const std::string fname = __FUNCTION__;
  int num_args = lua_gettop(L);

  std::string solver_name = "CFEMDiffusionSolver";

  if (num_args == 1)
  {
    LuaCheckStringValue(fname, L, 1);
    solver_name = lua_tostring(L, 1);
  }

  auto new_solver = std::make_shared<cfem_diffusion::Solver>(solver_name);

  Chi::object_stack.push_back(new_solver);

  lua_pushinteger(L,
      static_cast<lua_Integer>(Chi::object_stack.size()-1));

  Chi::log.LogAllVerbose1()
    << "\nCFEMDiffusionSolverCreate: CFEM Diffusion solver created"
    << std::endl;
  return 1;
}

}//namespace cfem_diffusion::cfem_diffusion_lua_utils
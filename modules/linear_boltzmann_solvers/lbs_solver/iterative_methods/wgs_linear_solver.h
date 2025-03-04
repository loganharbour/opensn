#pragma once

#include "framework/math/linear_solver/linear_solver.h"

#include "modules/linear_boltzmann_solvers/lbs_solver/iterative_methods/wgs_context.h"

#include <memory>
#include <vector>
#include <functional>

namespace opensn
{
namespace lbs
{

/**Linear Solver specialization for Within GroupSet (WGS) solves.*/
class WGSLinearSolver : public LinearSolver
{
public:
  /**
   * Constructor.
   * \param gs_context_ptr Context Pointer to abstract context.
   */
  explicit WGSLinearSolver(std::shared_ptr<WGSContext> gs_context_ptr);
  ~WGSLinearSolver() override;

protected:
  void PreSetupCallback() override;
  void SetConvergenceTest() override;
  void SetSystemSize() override;
  void SetSystem() override;
  void SetPreconditioner() override;
  void PostSetupCallback() override;
  void PreSolveCallback() override;
  void SetRHS() override;
  void SetInitialGuess() override;
  void PostSolveCallback() override;

  std::vector<double> saved_q_moments_local_;
};

} // namespace lbs
} // namespace opensn

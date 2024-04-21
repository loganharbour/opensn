// SPDX-FileCopyrightText: 2024 The OpenSn Authors <https://open-sn.github.io/opensn/>
// SPDX-License-Identifier: MIT

#include "modules/linear_boltzmann_solvers/lbs_solver/acceleration/acceleration.h"
#include "modules/linear_boltzmann_solvers/discrete_ordinates_solver/sweep/boundary/sweep_boundary.h"
#include "framework/materials/multi_group_xs/multi_group_xs.h"
#include "framework/logging/log_exceptions.h"
#include "framework/runtime.h"

namespace opensn
{
namespace lbs
{

std::map<uint64_t, BoundaryCondition>
TranslateBCs(const std::map<uint64_t, std::shared_ptr<SweepBoundary>>& sweep_boundaries,
             bool vaccum_bcs_are_dirichlet)
{
  std::map<uint64_t, BoundaryCondition> bcs;
  for (auto& [bid, lbs_bndry] : sweep_boundaries)
  {
    if (lbs_bndry->Type() == BoundaryType::REFLECTING)
      bcs[bid] = {BCType::ROBIN, {0.0, 1.0, 0.0}};
    else if (lbs_bndry->Type() == BoundaryType::VACUUM)
      if (vaccum_bcs_are_dirichlet)
        bcs[bid] = {BCType::DIRICHLET, {0.0, 0.0, 0.0}};
      else
        bcs[bid] = {BCType::ROBIN, {0.25, 0.5}};
    else // dirichlet
      bcs[bid] = {BCType::DIRICHLET, {0.0, 0.0, 0.0}};
  }

  return bcs;
}

std::map<int, Multigroup_D_and_sigR>
PackGroupsetXS(const std::map<int, std::shared_ptr<MultiGroupXS>>& matid_to_xs_map,
               int first_grp_index,
               int last_group_index)
{
  const int num_gs_groups = last_group_index - first_grp_index + 1;
  OpenSnInvalidArgumentIf(num_gs_groups < 0, "last_grp_index must be >= first_grp_index");

  typedef Multigroup_D_and_sigR MultiGroupXS;
  typedef std::map<int, Multigroup_D_and_sigR> MatID2XSMap;
  MatID2XSMap matid_2_mgxs_map;
  for (const auto& matid_xs_pair : matid_to_xs_map)
  {
    const auto& mat_id = matid_xs_pair.first;
    const auto& xs = matid_xs_pair.second;

    std::vector<double> D(num_gs_groups, 0.0);
    std::vector<double> sigma_r(num_gs_groups, 0.0);

    size_t g = 0;
    const auto& diffusion_coeff = xs->DiffusionCoefficient();
    const auto& sigma_removal = xs->SigmaRemoval();
    for (size_t gprime = first_grp_index; gprime <= last_group_index; ++gprime)
    {
      D[g] = diffusion_coeff[gprime];
      sigma_r[g] = sigma_removal[gprime];
      ++g;
    } // for g

    matid_2_mgxs_map.insert(std::make_pair(mat_id, MultiGroupXS{D, sigma_r}));
  }

  return matid_2_mgxs_map;
}

} // namespace lbs
} // namespace opensn

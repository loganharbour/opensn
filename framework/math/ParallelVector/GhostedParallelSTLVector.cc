#include "GhostedParallelSTLVector.h"

#include "chi_log.h"

#include <algorithm>

namespace chi_math
{

std::unique_ptr<ParallelVector>
GhostedParallelSTLVector::MakeCopy() const
{
  return std::make_unique<GhostedParallelSTLVector>(*this);
}

std::unique_ptr<ParallelVector>
GhostedParallelSTLVector::MakeClone() const
{
  auto new_vec = std::make_unique<GhostedParallelSTLVector>(*this);

  new_vec->Set(0.0);

  return new_vec;
}

double
GhostedParallelSTLVector::GetGlobalValue(const int64_t global_id) const
{
  if (global_id >= extents_[location_id_] and global_id < extents_[location_id_ + 1])
    return values_[global_id - extents_[location_id_]];

  ChiInvalidArgumentIf(ghost_comm_.MapGhostToLocal(global_id) == -1,
                       "Invalid global id specified. Specified global ids must be "
                       "locally owned or ghosts.");
  return values_[ghost_comm_.MapGhostToLocal(global_id)];
}

} // namespace chi_math

#pragma once

#include "framework/mesh/mesh.h"
#include <set>
#include <memory>

namespace opensn
{
namespace lbs
{

enum class FaceOrientation : short
{
  PARALLEL = -1,
  INCOMING = 0,
  OUTGOING = 1
};

enum class AngleSetStatus
{
  NOT_FINISHED = 0,
  FINISHED = 1,
  RECEIVING = 2,
  READY_TO_EXECUTE = 3,
  EXECUTE = 4,
  NO_EXEC_IF_READY = 5,
  MESSAGES_SENT = 6,
  MESSAGES_PENDING = 7
};

struct Task
{
  unsigned int num_dependencies_;
  std::vector<uint64_t> successors_;
  uint64_t reference_id_;
  const Cell* cell_ptr_;
  bool completed_ = false;
};

/**Sweep Plane Local Subgrid (“spills”), a contiguous collection of cells
 * that defines the lowest level in the SPDS hierarchy. The intent is that
 * the processing “locations” responsible for executing sweeps on this
 * collection of cells can readily read to and write from a common data
 * structure. A SPLS contains one or more entire cellsets — it cannot split
 * a cellset.
 */
struct SPLS
{
  std::vector<int> item_id;
};

/**Stage Task Dependency Graphs*/
struct STDG
{
  std::vector<int> item_id;
};

/**Communicates location by location dependencies.*/
void CommunicateLocationDependencies(const std::vector<int>& location_dependencies,
                                     std::vector<std::vector<int>>& global_dependencies);

/**Print a sweep ordering to file.*/
void PrintSweepOrdering(SPDS* sweep_order, std::shared_ptr<MeshContinuum> vol_continuum);

} // namespace lbs
} // namespace opensn

#include "framework/mesh/mesh_handler/mesh_handler.h"
#include "framework/mesh/mesh_continuum/mesh_continuum.h"
#include "framework/mesh/volume_mesher/volume_mesher.h"

#include "framework/logging/log.h"

namespace opensn
{

std::shared_ptr<MeshContinuum>&
MeshHandler::GetGrid() const
{
  if (volume_mesher_ == nullptr)
    throw std::logic_error("MeshHandler::GetGrid: Volume mesher "
                           "undefined. This usually means a grid is not defined"
                           " or is incomplete.");

  auto& grid_ptr = volume_mesher_->GetContinuum();

  if (grid_ptr == nullptr)
    throw std::logic_error("MeshHandler::GetGrid: Volume mesher has "
                           "no grid available. Make sure the volume mesher has "
                           "been executed.");

  return grid_ptr;
}

VolumeMesher&
MeshHandler::GetVolumeMesher()
{
  if (volume_mesher_ == nullptr)
    throw std::logic_error("MeshHandler::GetVolumeMesher: "
                           "Volume mesher undefined This usually means a "
                           "grid is not defined or is incomplete.");
  return *volume_mesher_;
}

const VolumeMesher&
MeshHandler::GetVolumeMesher() const
{
  if (volume_mesher_ == nullptr)
    throw std::logic_error("MeshHandler::GetVolumeMesher: "
                           "Volume mesher undefined This usually means a "
                           "grid is not defined or is incomplete.");
  return *volume_mesher_;
}

} // namespace opensn

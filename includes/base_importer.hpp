#pragma once

#include "../third-party/PXD-STL/includes/absl/flat_hash_map.hpp"

namespace pxd::ass {

struct Mesh;
struct MeshNode;

class IImporter
{
public:
  virtual auto init(std::string_view&                           filepath,
                    absl::flat_hash_map<std::string, Mesh>&     meshes,
                    absl::flat_hash_map<std::string, MeshNode>& nodes,
                    std::vector<MeshNode*>& parent_nodes) -> bool = 0;
};

} // namespace pxd::ass
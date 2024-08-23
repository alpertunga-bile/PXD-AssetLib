#include "model.hpp"

#include "types.hpp"

#include <iostream>

int
main()
{
  pxd::ass::Model model;

  model.init("scenes/Sponza.gltf", pxd::ass::IMPORTER::ASSIMP);

  // model.optimize_meshes();

  std::vector<pxd::ass::Mesh> meshes = model.get_meshes();

  std::cout << model.parent_nodes[0]->meshes[0]->name;

  model.destroy();

  return 0;
}
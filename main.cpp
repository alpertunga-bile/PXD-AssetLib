#include "scene.hpp"

#include <iostream>

int
main()
{
  pxd::ass::Scene scene;

  scene.init("scenes/Sponza.gltf", pxd::ass::PXD_ASS_IMPORTER::ASSIMP);

  scene.optimize_meshes();

  for (auto& [k, v] : scene.meshes) {
    v.calculate_quads();
  }

  scene.destroy();

  return 0;
}
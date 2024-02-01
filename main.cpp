#include "scene.hpp"

#include <iostream>

int
main()
{
  pxd::ass::Scene scene;

  scene.init("scenes/Sponza.gltf", pxd::ass::PXD_ASS_IMPORTER::ASSIMP);

  scene.optimize_meshes();

  scene.destroy();

  return 0;
}
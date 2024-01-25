#include "scene.hpp"

#include <iostream>

int
main()
{
  pxd::ass::Scene scene;

  scene.init("scenes/structure.glb");

  pxd::ass::Mesh mesh;
  std::string    mesh_name = "hallreactor_circ4500_ReactorOuterWall.001";
  std::cout << scene.get_mesh_w_name(mesh_name, mesh);

  scene.destroy();

  return 0;
}
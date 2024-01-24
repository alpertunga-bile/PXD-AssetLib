#include "scene.hpp"

#include <iostream>

int main() {
  pxd_ass::Scene scene;

  scene.init("scenes/structure.glb");

  for (auto [k, v] : scene.meshes) {
    std::cout << v.positions.size() << "\n";
  }

  scene.destroy();

  return 0;
}
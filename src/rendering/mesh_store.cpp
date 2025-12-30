#include "mesh_store.h"

int MeshStore::add(MeshBuffer buffer) {
  counter += 1;
  meshes[counter] = buffer;
  return counter;
}

MeshBuffer MeshStore::operator[](int handle) const { return meshes.at(handle); }
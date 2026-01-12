#pragma once

#include <ecs/asset_store.h>
#include <ecs/component.h>
#include <rendering/concepts/material.h>

template <Material M>
struct MeshMaterial : ComponentBase {
  AssetHandle<M> material;
};
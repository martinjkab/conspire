#pragma once
#include <ecs/asset_store.h>
#include <rendering/concepts/material.h>
#include <rendering/texture.h>

struct StandardMaterial {
  AssetHandle<Texture> texture;

  void uploadUniforms() {}
};

static_assert(Material<StandardMaterial>);
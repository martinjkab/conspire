#pragma once

#include <ecs/asset_traits.h>
#include <rendering/materials/standard_material.h>

template <> struct AssetTraits<StandardMaterial> {
  using CPUDataType = StandardMaterial;
  static constexpr bool IsGpuAsset = true;
};

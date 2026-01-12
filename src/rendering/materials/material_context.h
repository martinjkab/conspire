#pragma once

#include <core/platform.h>
#include <ecs/asset_store.h>

struct MaterialContext {
  const AssetStore& assetStore;
  const VkDevice& device;
  const VkSampler& defaultSamplerNearest;
};
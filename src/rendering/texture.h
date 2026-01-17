#pragma once
#include <rendering/utils/vk_types.h>

struct Texture {
  AllocatedImage image;
};

template <>
struct AssetTraits<Texture> {
  using CPUDataType = std::tuple<std::vector<uint8_t>, unsigned, unsigned>;
};
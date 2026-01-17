#include <ecs/asset_loader.h>
#include <ecs/asset_store.h>
#include <lodepng.h>
#include <rendering/mesh_buffer.h>
#include <rendering/utils/vk_utils.h>
#include <rendering/vertex.h>

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <filesystem>
#include <ostream>

#include "texture.h"
#include "vk_images.h"
#include "vk_init.h"
#include "vk_utils.h"

template <>
AssetTraits<Texture>::CPUData AssetLoader<Texture>::load(
    const std::string& path) const {
  auto [data, width, height] = loadSprite(path);

  return {image};
}

std::tuple<std::vector<uint8_t>, unsigned, unsigned> loadSprite(
    std::string path) {
  std::vector<uint8_t> image;
  unsigned width, height;

  unsigned error = lodepng::decode(image, width, height, path);

  if (error) {
    fmt::print("Error loading sprite {}: {}\n", path,
               lodepng_error_text(error));
    return {};
  }

  fmt::print("Successfully loaded sprite: {} ({}x{}, RGBA)\n", path, width,
             height);
  return {image, width, height};
}

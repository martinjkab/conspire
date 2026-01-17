#include <ecs/asset_loader.h>
#include <ecs/asset_store.h>
#include <rendering/mesh_buffer.h>
#include <rendering/utils/vk_utils.h>
#include <rendering/vertex.h>

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <filesystem>

template <>
AssetTraits<MeshBuffer>::CPUData AssetLoader<MeshBuffer>::load(
    const std::string& path) const {
  fastgltf::Parser parser;
  auto data = fastgltf::GltfDataBuffer::FromPath(path);

  if (data.error() != fastgltf::Error::None) {
    throw data.error();
  }

  auto assetResult =
      parser.loadGltfBinary(data.get(), path.parent_path(),
                            fastgltf::Options::LoadGLBBuffers |
                                fastgltf::Options::LoadExternalBuffers);
  if (assetResult.error() != fastgltf::Error::None) {
    throw assetResult.error();
  }
  const auto& asset = assetResult.get();

  const auto& primitive = asset.meshes.at(0).primitives.at(0);

  auto& posAccessor =
      asset.accessors[primitive.findAttribute("POSITION")->accessorIndex];
  auto& uvAccessor =
      asset.accessors[primitive.findAttribute("TEXCOORD_0")->accessorIndex];
  auto& indexAccessor = asset.accessors[primitive.indicesAccessor.value()];

  std::vector<Vertex> vertices(posAccessor.count);
  std::vector<uint32_t> indices(indexAccessor.count);

  fastgltf::iterateAccessorWithIndex<std::uint32_t>(
      asset, indexAccessor,
      [&](std::uint32_t index, size_t idx) { indices[idx] = index; });

  fastgltf::iterateAccessorWithIndex<glm::vec3>(
      asset, posAccessor, [&](glm::vec3 p, size_t idx) {
        vertices[idx].position = glm::vec4{p, 1.};
      });

  fastgltf::iterateAccessorWithIndex<glm::vec2>(
      asset, uvAccessor,
      [&](glm::vec2 uv, size_t idx) { vertices[idx].tex = uv; });

  return uploadMesh(vertices, indices);
}

#pragma once
#include <ecs/asset_loader.h>
#include <render_context.h>
#include <rendering/materials/standard_material.h>
#include <rendering/materials/standard_material_asset.h>

template <> class AssetLoader<StandardMaterial> {
  VkDescriptorSetLayout layout;
  DescriptorAllocator* allocator;

public:
  AssetLoader() : layout(VK_NULL_HANDLE), allocator(nullptr) {}
  AssetLoader(VkDescriptorSetLayout layout, DescriptorAllocator* allocator)
      : layout(layout), allocator(allocator) {}

  AssetTraits<StandardMaterial>::CPUDataType
  loadCPU(const std::string& path) const {
    return {};
  }

  StandardMaterial
  loadGPU(const AssetTraits<StandardMaterial>::CPUDataType& CPUData,
          const RenderContext& context, const Uploader& uploader) const {
    StandardMaterial mat = CPUData;
    if (allocator && layout)
      mat.init(context.device, layout, *allocator);
    return mat;
  }
};

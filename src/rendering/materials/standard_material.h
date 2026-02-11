#pragma once
#include <ecs/asset_store.h>
#include <rendering/concepts/material.h>
#include <rendering/texture.h>
#include <rendering/utils/vk_descriptors.h>

struct StandardMaterial {
  AssetHandle<Texture> texture;
  VkDescriptorSet imageSet; // TODO: remove this, use Transient descriptor set

  void init(VkDevice device, VkDescriptorSetLayout layout,
            DescriptorAllocator& allocator) {
    imageSet = allocator.allocate(device, layout);
  }

  void uploadUniforms(const MaterialContext& context) const {
    VkDescriptorSet set = context.targetSet ? *context.targetSet : imageSet;
    auto textureData = context.assetStore[texture].value();
    {
      DescriptorWriter writer;
      writer.writeImage(0, textureData.image.imageView,
                        context.defaultSamplerNearest,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
      writer.updateSet(context.device, set);
    }
  }
};

static_assert(Material<StandardMaterial>);
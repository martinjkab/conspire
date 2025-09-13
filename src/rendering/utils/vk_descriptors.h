#include <vector>
#include <span>
#include <vulkan/vulkan.h>

struct DescriptorLayoutBuilder {

    std::vector<VkDescriptorSetLayoutBinding> bindings;

    void addBinding(uint32_t binding, VkDescriptorType type);
    VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
    void clear();
};

struct DescriptorAllocator {

    struct PoolSizeRatio {
        VkDescriptorType type;
        float ratio;
    };

    VkDescriptorPool pool;

    void initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
    void clearDescriptors(VkDevice device) const;
    void destroyPool(VkDevice device) const;

    VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout) const;
};
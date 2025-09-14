#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace vkutil
{
    bool loadShaderModule(const char *filePath, VkDevice device, VkShaderModule *outShaderModule);
}

class PipelineBuilder
{
public:
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;

    VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
    VkPipelineRasterizationStateCreateInfo _rasterizer;
    VkPipelineColorBlendAttachmentState _colorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo _multisampling;
    VkPipelineLayout _pipelineLayout;
    VkPipelineDepthStencilStateCreateInfo _depthStencil;
    VkPipelineRenderingCreateInfo _renderInfo;
    VkFormat _colorAttachmentformat;

    PipelineBuilder() { clear(); }

    void clear();
    void setShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
    void setInputTopology(VkPrimitiveTopology topology);
    void setPolygonMode(VkPolygonMode mode);
    void setMultisamplingNone();
    void setCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
    void disableBlending();
    void setColorAttachmentFormat(VkFormat format);
    void setDepthFormat(VkFormat format);
    void disableDepthtest();
    VkPipeline buildPipeline(VkDevice device);
};
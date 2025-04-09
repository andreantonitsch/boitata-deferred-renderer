#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "BttEnums.hpp"
#include "RenderTarget.hpp"
#include "Descriptors.hpp"
#include "../collections/Pool.hpp"
#include <glm/glm.hpp>
#include <renderer/modules/Camera.hpp>

namespace boitatah
{
    
    struct PushConstant{
        void* ptr;
        uint32_t offset;
        uint32_t size;
        SHADER_STAGE stages;
    };

    struct PushConstantDesc{
        uint32_t offset;
        uint32_t size;
        SHADER_STAGE stages;
    };

    // becomes a shaderlayout with 3 descriptor set layouts and push constants
    struct ShaderLayoutDesc
    {
        std::vector<Handle<DescriptorSetLayout>> setLayouts;
        //std::vector<PushConstantDesc> pushConstants;
    };

    struct ShaderLayoutDescVk
    {
        std::vector<VkDescriptorSetLayout> materialLayouts;
        std::vector<PushConstantDesc> pushConstants;

    };

    struct ShaderLayout
    {
        VkPipelineLayout pipeline;
        std::vector<Handle<DescriptorSetLayout>> descriptorSets;
    };

    struct ShaderModule
    {
        VkShaderModule shaderModule;
        std::string entryFunction = "main";
    };

    struct ShaderModules
    {
        ShaderModule vert;
        ShaderModule frag;
    };

    struct ShaderStage
    {
        std::vector<char> byteCode;
        std::string entryFunction = "main";
    };

    struct VertexAttribute
    {
        uint32_t location;
        IMAGE_FORMAT format;
        uint32_t offset;
    };

    struct VertexBindings
    {
        uint32_t stride;
        std::vector<VertexAttribute> attributes;
    };

    struct ColorBlend
    {

    };

    struct MakeShaderDesc
    {
        // required arguments
        std::string name;
        ShaderStage vert;
        ShaderStage frag;
        
        Handle<RenderPass> renderPass;
        
        Handle<ShaderLayout> layout;

        std::vector<ColorBlend> colorBlends;
        std::vector<VertexBindings> vertexBindings;
    };


    struct ShaderDescVk
    {
        // required arguments
        std::string name;
        VkRenderPass renderpass;
        VkPipelineLayout layout;
        bool use_depth = false;
        std::vector<ColorBlend> colorBlends;
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;
    };

    struct Shader
    {
        std::string name;
        ShaderModule vert;
        ShaderModule frag;

        VkPipeline pipeline;
        ShaderLayout layout;
        MakeShaderDesc description;

    };


}


#ifndef BOITATAH_SHADER_HPP
#define BOITATAH_SHADER_HPP

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "BttEnums.hpp"
#include "RenderTarget.hpp"
#include "Descriptors.hpp"
#include "../collections/Pool.hpp"
#include <glm/glm.hpp>

namespace boitatah
{

    // becomes a shaderlayout with 3 descriptor set layouts and push constants
    struct ShaderLayoutDesc
    {
        DescriptorSetLayoutDesc materialLayout;
    };

    struct ShaderLayoutDescVk
    {
        VkDescriptorSetLayout materialLayout;
        VkDescriptorSetLayout baseLayout;
    };

    struct ShaderLayout
    {
        VkPipelineLayout layout;
        VkDescriptorSetLayout setLayout;
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
        FORMAT format;
        uint32_t offset;
    };

    struct VertexBindings
    {
        uint32_t stride;
        std::vector<VertexAttribute> attributes;
    };

    struct ShaderDesc
    {
        // required arguments
        std::string name;
        ShaderStage vert;
        ShaderStage frag;

        // A compatible framebuffer.
        // sets the RenderPass for this PSO.
        Handle<RenderTarget> framebuffer; // optional
        Handle<ShaderLayout> layout;

        std::vector<VertexBindings> bindings;
    };

    struct ShaderDescVk
    {
        // required arguments
        std::string name;
        ShaderModule vert;
        ShaderModule frag;

        VkRenderPass renderpass;
        VkPipelineLayout layout;
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;
    };

    struct Shader
    {
        std::string name;
        ShaderModule vert;
        ShaderModule frag;

        VkPipeline pipeline;
    };

    // gets updated every frame
    struct FrameUniforms
    {
        glm::mat4 projection;
        glm::mat4 view;
        float time;
        float deltaTime;
    };

    enum class UNIFORM_TYPE
    {
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        STRUCT,

    };


}

#endif // BOITATAH_SHADER_HPP
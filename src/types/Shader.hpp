#ifndef BOITATAH_SHADER_HPP
#define BOITATAH_SHADER_HPP

#include <vulkan/vulkan.h>
#include <string>
#include "../types/BttEnums.hpp"
#include "Framebuffer.hpp"

namespace boitatah
{


    struct ShaderModule{
        VkShaderModule shaderModule;
        std::string entryFunction = "main";
    };

    struct ShaderStage{
        std::vector<char> byteCode;
        std::string entryFunction = "main";
    };

    struct ShaderDesc{
        //required arguments
        std::string name;
        ShaderStage vert;
        ShaderStage frag;

        // A compatible framebuffer.
        // sets the RenderPass for this PSO.
        Handle<Framebuffer> framebuffer;
    };

    struct ShaderDescVk{
        //required arguments
        std::string name;
        ShaderModule vert;
        ShaderModule frag;

        VkRenderPass renderpass;
    };

    struct Shader
    {
        std::string name;
        ShaderModule vert;
        ShaderModule frag;

        Handle<Framebuffer> frameBuffer;

        VkPipelineLayout layout;
        //VkRenderPass renderPass;
        VkPipeline pipeline;

    };

    // struct ShaderHot{

    // };
    // struct ShaderCold{

    // }; than use templates
}

#endif // BOITATAH_SHADER_HPP
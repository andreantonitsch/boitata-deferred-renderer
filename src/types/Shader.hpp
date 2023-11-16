#ifndef BOITATAH_SHADER_HPP
#define BOITATAH_SHADER_HPP

#include <vulkan/vulkan.h>
#include <string>
#include "../types/BttEnums.hpp"
#include "RenderTarget.hpp"

namespace boitatah
{
    struct PipelineLayoutDesc{
        
    };

    struct PipelineLayout{
        VkPipelineLayout layout;
    };


    struct ShaderModule{
        VkShaderModule shaderModule;
        std::string entryFunction = "main";
    };
    
    struct ShaderModules{
        ShaderModule vert;
        ShaderModule frag;
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
        Handle<RenderTarget> framebuffer; //optional
        Handle<PipelineLayout> layout;
    };

    struct ShaderDescVk{
        //required arguments
        std::string name;
        ShaderModule vert;
        ShaderModule frag;

        VkRenderPass renderpass;
        VkPipelineLayout layout;
    };



    struct Shader
    {
        std::string name;
        ShaderModule vert;
        ShaderModule frag;

        VkPipeline pipeline;
        //Handle<PipelineLayout> layout; <--- for shader recreation
        //Handle<Swapchain*> compatibleSwapchain <-- for comparisoon of swapchain compatibility
        //for shader recreation
        //Handle<ShaderDesc> description;
        //Handle<ShaderModules> modules;
        //int Swapchain swapchain;

    };

}

#endif // BOITATAH_SHADER_HPP
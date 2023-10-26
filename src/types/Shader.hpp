#ifndef BOITATAH_SHADER_HPP
#define BOITATAH_SHADER_HPP

#include <vulkan/vulkan.h>
#include <string>

namespace boitatah
{

    


    struct ShaderStage{
        std::vector<char> byteCode;
        const std::string entryFunction = "main";
    };

    struct ShaderDesc{
        std::string name;
        ShaderStage vert;
        ShaderStage frag;


    };

    struct Shader
    {
        std::string name;
        VkShaderModule vert;
        VkShaderModule frag;

        VkPipelineLayout layout;
        VkRenderPass renderPass;
        VkPipeline PSO;

        // ShaderHot hot;
        // ShaderCold cold;
    };

    // struct ShaderHot{

    // };
    // struct ShaderCold{

    // }; than use templates
}

#endif // BOITATAH_SHADER_HPP
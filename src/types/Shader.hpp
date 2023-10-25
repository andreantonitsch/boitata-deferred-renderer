#ifndef BOITATAH_SHADER_HPP
#define BOITATAH_SHADER_HPP

#include <vulkan/vulkan.h>
#include <string>

namespace boitatah
{

    struct ShaderModule
    {

    };

    struct ShaderStage{
        std::vector<char> byteCode;
        const std::string entryFunction = "main";
    };

    struct CreateShader{
        std::string name;
        ShaderStage vert;
        ShaderStage frag;
    };

    struct Shader
    {
        std::string name;
        

        Handle<ShaderModule> vert;
        Handle<ShaderModule> vert;

        // ShaderHot hot;
        // ShaderCold cold;
    };

    // struct ShaderHot{

    // };
    // struct ShaderCold{

    // }; than use templates
}

#endif // BOITATAH_SHADER_HPP
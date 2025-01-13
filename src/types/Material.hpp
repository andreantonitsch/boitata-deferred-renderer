#pragma once

#include <glm/glm.hpp>
#include "../buffers/Buffer.hpp"
#include "../collections/Pool.hpp"
#include "../renderer/resources/GPUBuffer.hpp"
#include "Descriptors.hpp"
#include "Shader.hpp"

namespace boitatah{


    struct Material
    {
        Handle<Shader> shader;
        // at most 2, one per material and one per instance
        std::vector<DescriptorSetLayout> materialLayouts;
        //Bindings perMaterialBindings;
        //Bindings perInstanceBindings;
    };

    //Binds resources to shaders
    struct Uniform
    {
        uint32_t layout;
        uint32_t set;
        uint32_t binding;
        Handle<GPUBuffer> uniform;
    };

    struct Bindings
    {
        std::span<Uniform> bindings;
    };

}


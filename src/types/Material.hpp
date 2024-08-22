#ifndef BOITATAH_MATERIAL_HPP
#define BOITATAH_MATERIAL_HPP

#include "../buffers/Buffer.hpp"
#include "Descriptors.hpp"
#include "../collections/Pool.hpp"
#include <glm/glm.hpp>
#include "Shader.hpp"
#include "Uniform.hpp"

namespace boitatah{


    struct Material
    {
        Handle<Shader> shader;
        // at most 2, one per material and one per instance
        std::vector<DescriptorSetLayout> materialLayouts;
        // Bindings perMaterialBindings;
    };

    struct Binding
    {
        uint32_t layout;
        uint32_t set;
        uint32_t binding;
        Handle<Uniform> uniform;
    };

    struct Bindings
    {
        std::span<Bindings> bindings;
    };

}

#endif //BOITATAH_MATERIAL_HPP
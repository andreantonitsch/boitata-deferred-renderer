#pragma once

#include<vector>
#include <collections/Pool.hpp>
#include <types/Geometry.hpp>
#include <string>

namespace boitatah{

    class GPUBuffer;
    class FixedTexture;
    class RenderTexture;


    struct MaterialBindingAtt{
        DESCRIPTOR_TYPE type;
        union {
            Handle<GPUBuffer> buffer;
            Handle<FixedTexture> fixedTex;
            Handle<RenderTexture> renderTex;
            Handle<Image> image;
        }binding_handle;
    };
    struct MaterialBinding{
        std::vector<MaterialBindingAtt> bindings;
    };
    
    ///each material can bind one set of variables.
    struct Material{
        /// @brief material parent is related to descriptor sets.
        Handle<Material> parent;
        Handle<Shader> shader;
        std::vector<Handle<MaterialBinding>> bindings;
        std::vector<VERTEX_BUFFER_TYPE> vertexBufferBindings;
        std::string name;
    };

    struct MaterialCreate{
        Handle<Material> parent;
        Handle<Shader> shader;
        //this fetches the descriptor set.
        std::vector<Handle<MaterialBinding>> bindings; 
        std::vector<VERTEX_BUFFER_TYPE> vertexBufferBindings;
        std::string name;
    };
};

#pragma once

#include<vector>
#include <collections/Pool.hpp>
#include <renderer/resources/Geometry.hpp>
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
            Handle<Sampler> sampler;
        }binding_handle;
    };
    struct MaterialBinding{
        std::vector<MaterialBindingAtt> bindings;
    };
    
    ///each material can bind one set of variables.
    struct Material{
        //Stage mask marks which stage this material will render to.
        uint32_t stage_mask = UINT32_MAX;
        //priority is used to order materials. 
        uint32_t priority = 0U;
        /// @brief material parent is related to descriptor sets.
        //Handle<Material> parent;
        Handle<Shader> shader;
        std::vector<Handle<MaterialBinding>> bindings;
        std::vector<VERTEX_BUFFER_TYPE> vertexBufferBindings;
        std::string name;


        std::weak_ordering operator<=>(const Material& o) const{
            if(stage_mask == o.stage_mask)
                return priority <=> o.priority;
            else
                return stage_mask <=> o.stage_mask;
        };

        private:
            friend class MaterialManager;
            //Render order when materials are ordered.
            uint32_t order = 0u;
    };

    struct MaterialCreate{
        uint32_t stage_mask = UINT32_MAX;
        uint32_t priority = 0U;     
        //Handle<Material> parent;
        Handle<Shader> shader;
        //this fetches the descriptor set.
        std::vector<Handle<MaterialBinding>> bindings; 
        std::vector<VERTEX_BUFFER_TYPE> vertexBufferBindings;
        std::string name;
    };
};

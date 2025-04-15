#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "../buffers/BufferStructs.hpp"
#include "../buffers/Buffer.hpp"
#include "../collections/Pool.hpp"
#include <renderer/resources/GPUBuffer.hpp>
#include <renderer/resources/GPUResource.hpp>
#include <renderer/resources/ResourceStructs.hpp>
//#include <renderer/modules/GPUResourceManager.hpp>

#include <array>

namespace boitatah
{

    class GPUResourceManager;


    enum class LIGHT_TYPE : uint8_t{
        POINT = 1u,
    }; 

    struct Light{
        
        //public:
        glm::vec4 position;
        glm::vec4 color;
        float intensity;
        float a;
        LIGHT_TYPE type;
        uint8_t active;
        uint8_t b;
        uint8_t c;
        //private:
        //friend class LightArray;
        uint32_t index;
    };
    
    class LightArray 
    {

        private:
            std::shared_ptr<GPUResourceManager> m_manager;
            Handle<GPUBuffer> m_light_buffer;
            Handle<GPUBuffer> m_lightmetada;
            std::vector<Light> light_content;
            std::vector<uint32_t> light_index;
            uint32_t m_active_lights = 0u;
            uint32_t m_light_capacity = 0u;
            Light m_default_light;
        public:
            LightArray() = default;
            LightArray(uint32_t light_capacity, 
                       Light&& default_light,
                       std::shared_ptr<GPUResourceManager> manager);

            ~LightArray() = default;
            LightArray(const LightArray& other) = default;
            LightArray(LightArray& other) = default;

            uint32_t addLight(Light&& light);
            void removeLight(uint32_t index);

            Light& operator[](int idx){return light_content[idx];};
            void update();

            Handle<GPUBuffer> metadata();
            Handle<GPUBuffer> light_array();


    };


}


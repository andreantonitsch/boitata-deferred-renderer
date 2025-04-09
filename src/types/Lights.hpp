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
    class LightArray;
    struct LightArrayRenderData{

    };

    struct LightArrayGPUData {};
    template<>
    struct ResourceTraits<LightArray>{
        using ContentType = LightArrayGPUData;
        using CommandBufferWriter = vk::VkCommandBufferWriter;
        using RenderData = LightArrayRenderData;
    };


    enum class LIGHT_TYPE : uint8_t{
        POINT = 1u,
    }; 

    struct Light{
        private:
        friend class LightArray;
        uint32_t index;
        
        public:
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
        LIGHT_TYPE type;
        uint8_t active;
        uint8_t b;
        uint8_t c;
    };
    
    struct LightArrayCreateDescription
    {
        uint32_t capacity;
    };

    class LightArray : public MutableGPUResource<LightArray>
    {
        friend class GPUResourceManager;
        private:
            std::shared_ptr<GPUResourceManager> m_manager;
            Handle<GPUBuffer> m_buffer;
            std::vector<Light> light_content;
            std::vector<uint32_t> light_index;
            uint32_t active_lights = 0u;
            bool dirty = false;
        public:
            LightArray() = default;
            LightArray(std::shared_ptr<GPUResourceManager> manager):
                MutableGPUResource<LightArray>({ //Base Constructor
                                                    .sharing = SHARING_MODE::EXCLUSIVE,
                                                    .type = RESOURCE_TYPE::GEOMETRY,
                                                    .mutability = RESOURCE_MUTABILITY::MUTABLE,
                                                  }, manager),
                m_manager(manager) { };

            ~LightArray() = default;
            LightArray(const LightArray& other) = default;

            uint32_t addLight(Light&& light);
            void removeLight(uint32_t index);

            Light& operator[](int idx){dirty = true; return light_content[idx];};
            void update();
            //Handle<GPUBuffer> operator[](int idx) const{return m_buffers[idx];};

            LightArrayRenderData GetRenderData() {return LightArrayRenderData{};}
            LightArrayGPUData CreateGPUData() {return LightArrayGPUData{};}
            bool ReadyForUse(LightArrayGPUData& content){ return !dirty; };
            void SetContent(LightArrayGPUData& content){};
            void ReleaseData(LightArrayGPUData& content){};
            void Release();

    };


}


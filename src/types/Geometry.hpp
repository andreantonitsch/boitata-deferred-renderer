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
    class Geometry;
    class GPUResourceManager;

    /// Type also Defines the Buffer Indexes.
    enum class VERTEX_BUFFER_TYPE : uint8_t{
        POSITION = 0U,
        NORMAL = 1U,
        UV = 2U,
        UV2 = 3U,
        COLOR = 4U,
        MISC0 = 5U,
        MISC1 = 6U,
        MISC2 = 7U
    };

    struct GeometryGPUData {};
    template<>
    struct ResourceTraits<Geometry>{
        using ContentType = GeometryGPUData;
        using CommandBufferWriter = vk::VkCommandBufferWriter;
    };


    enum class GEO_DATA_TYPE{
        Ptr,
        UIntVector,
        GPUBuffer,
    }; 

    struct GeometryBufferDataDesc{
        VERTEX_BUFFER_TYPE buffer_type;
        GEO_DATA_TYPE data_type;
        uint32_t vertexCount;
        uint32_t vertexSize;
        const void* vertexDataPtr;
        Handle<GPUBuffer> buffer;
    };

    
    struct GeometryIndexDataDesc{   
        GEO_DATA_TYPE data_type;
        uint32_t count;
        const void* dataPtr;
        std::vector<uint32_t> index_vector;
        Handle<GPUBuffer> buffer;
    };


    struct GeometryCreateDescription
    {
        glm::ivec2 vertexInfo;
        std::vector<GeometryBufferDataDesc> bufferData;
        GeometryIndexDataDesc indexData;
    };



    class Geometry : public ImmutableGPUResource<Geometry>
    {
        friend class GPUResourceManager;
        private:
            std::vector<Handle<GPUBuffer>> m_ownedBuffers;
            std::vector<Handle<GPUBuffer>> m_buffers;
            std::array<uint8_t, 8> m_bufferIndexes = {255,255,255,255,
                                                      255,255,255,255};

            //Count, begin
            glm::ivec2 vertexInfo;
            Handle<GPUBuffer> indexBuffer;
            uint32_t indiceCount;

            uint8_t typeToIndex(VERTEX_BUFFER_TYPE type){
                uint8_t index = static_cast<uint8_t>(type);
                return m_bufferIndexes[index];
            };

        public:
            Geometry() = default;
            Geometry(std::shared_ptr<GPUResourceManager> manager):
                ImmutableGPUResource<Geometry>({ //Base Constructor
                                                    .sharing = SHARING_MODE::EXCLUSIVE,
                                                    .type = RESOURCE_TYPE::GEOMETRY,
                                                    .mutability = RESOURCE_MUTABILITY::MUTABLE,
                                                  }, manager) 
                                                  { };
            ~Geometry() = default;
            Geometry(const Geometry& other) = default;

            Handle<GPUBuffer>& operator[](int idx){return m_buffers[idx];};
            Handle<GPUBuffer> operator[](int idx) const{return m_buffers[idx];};

            void addOwnedBuffer(Handle<GPUBuffer>& buffer, VERTEX_BUFFER_TYPE type){
                m_bufferIndexes[static_cast<uint8_t>(type)] = m_buffers.size();

                m_ownedBuffers.push_back(buffer);
                m_buffers.push_back(buffer);
            }
            
            void addExternalBuffer(Handle<GPUBuffer>& buffer, VERTEX_BUFFER_TYPE type){
                m_bufferIndexes[static_cast<uint8_t>(type)] = m_buffers.size();
                m_buffers.push_back(buffer);
            }

            Handle<GPUBuffer> getBuffer(VERTEX_BUFFER_TYPE type){
                uint8_t index = typeToIndex(type);
                //if unset buffer type, return null handle
                if(index > 8)
                    return Handle<GPUBuffer>();

                return m_buffers[index];
            };

            glm::ivec2 VertexInfo(){
                return vertexInfo;
             };

             uint32_t IndexCount(){
                return indiceCount;
             };

             Handle<GPUBuffer> IndexBuffer(){
                return indexBuffer;
             };

            GeometryGPUData CreateGPUData() {return GeometryGPUData{};}
            bool ReadyForUse(GeometryGPUData& content){ return true; };
            void SetContent(GeometryGPUData& content){};
            void ReleaseData(GeometryGPUData& content){};
            void Release();
            // {
                
            //     auto manager = std::shared_ptr<GPUResourceManager>(m_manager);

            //     for(auto& buffer : m_ownedBuffers ){
            //         manager->destroy(buffer);
            //     }
            //     manager->destroy(indexBuffer);
            //  };

            //expensive call
            void ComputeSmoothNormals();

            //expensive call
            void ComputeFlatNormals();


    };


}


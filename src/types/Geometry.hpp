#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "../buffers/BufferStructs.hpp"
#include "../buffers/Buffer.hpp"
#include "../collections/Pool.hpp"
#include <renderer/resources/GPUResource.hpp>
#include <renderer/resources/ResourceStructs.hpp>

#include <array>

namespace boitatah
{
    class Geometry;

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
        void* vertexDataPtr;
        Handle<GPUBuffer> buffer;
    };

    
    struct GeometryIndexDataDesc{   
        GEO_DATA_TYPE data_type;
        uint32_t count;
        void* dataPtr;
        std::vector<uint32_t> index_vector;
        Handle<GPUBuffer> buffer;
    };


    struct GeometryCreateDescription
    {
        glm::ivec2 vertexInfo;
        std::vector<GeometryBufferDataDesc> bufferData;
        GeometryIndexDataDesc indexData;
    };



    class Geometry : public MutableGPUResource<Geometry>
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
                index = m_bufferIndexes[index];
                return index;
            };

        public:
            Geometry() = default;
            Geometry(std::shared_ptr<GPUResourceManager> manager):
                MutableGPUResource<Geometry>({ //Base Constructor
                                                    .sharing = SHARING_MODE::EXCLUSIVE,
                                                    .type = RESOURCE_TYPE::GPU_BUFFER,
                                                    .mutability = RESOURCE_MUTABILITY::MUTABLE,
                                                  }, manager) 
                                                  { };;
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

            // TODO this is very jank
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
            void Release() {
                
                auto manager = std::shared_ptr<GPUResourceManager>(m_manager);

                for(auto& buffer : m_ownedBuffers ){
                    manager->destroy(buffer);
                }
                manager->destroy(indexBuffer);
             };


    };

    //geom data
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
    };

    struct GeometryData
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };

    static GeometryData triangleVertices()
    {
        return {
            .vertices = {
                {{0.0f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 1.0f}},
                {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}}},
            .indices = {0U, 1U, 2U},
        };
    }

    static GeometryData quadVertices()
    {
        return {
            .vertices = {
                {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
                {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 1.0f}},
                {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 1.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}},
                },
            .indices = {0U, 1U, 2U,
                        1U, 3U, 2U,
            }
        };
    }

    static GeometryData squareVertices()
    {
        return {
            .vertices = {
                {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}},
                {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}}},
            .indices = {0, 1, 2, 2, 3, 0},
        };
    }

    static GeometryData planeVertices(const float width,
                                      const float height,
                                      const uint32_t widthSegments,
                                      const uint32_t heightSegments)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        float w = width / widthSegments;
        float h = height / heightSegments;
        for(uint32_t j = 0; j <= heightSegments; j++){
            for(uint32_t i = 0; i <= widthSegments; i ++ ){
                float iw = i * w;
                float jh = j * h;

                vertices.push_back({
                    {iw - (0.5 * width), jh - (0.5 * height), 0.0f}, 
                    {iw, jh, 0.0f},
                });
            }
        }

        for(uint32_t i = 0; i < widthSegments; i ++ ){
            for(uint32_t j = 0; j < heightSegments; j++){

                
                indices.push_back(j * (widthSegments+1) + i);
                indices.push_back(j * (widthSegments+1) + i + 1);
                indices.push_back((j+1) * (widthSegments+1) + i);


                indices.push_back(j * (widthSegments+1) + i + 1);
                indices.push_back((j+1) * (widthSegments+1) + i + 1);
                indices.push_back((j+1) *(widthSegments+1) + i);
            }
        }
        
        return {.vertices = vertices, .indices = indices,};
    }

}


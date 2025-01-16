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
    struct GeometryGPUData {};
    template<>
    struct ResourceTraits<Geometry>{
        using ContentType = GeometryGPUData;
        using CommandBufferWriter = vk::VkCommandBufferWriter;
    };


    struct GeometryBufferData{
        Handle<GPUBuffer> buffer;
        uint32_t count;
        uint32_t elementSize;
    };

    struct GeometryBufferDataDesc{
        uint32_t vertexCount;
        uint32_t vertexSize;
        void* vertexDataPtr;
    };

    struct GeometryIndexDataDesc{
        uint32_t count;
        void* dataPtr;
    };

    // struct GeometryDesc
    // {
    //     glm::ivec2 vertexInfo;
    //     uint32_t vertexSize;
    //     uint32_t vertexDataSize;
    //     void *vertexData;
    //     uint32_t indexCount;
    //     void *indexData;
    // };

    struct GeometryCreateDescription
    {
        glm::ivec2 vertexInfo;
        std::vector<GeometryBufferDataDesc> bufferData;
        GeometryIndexDataDesc indexData;
    };

    // struct Geometry
    // {
    //     std::vector<Handle<BufferAddress>> buffers;
    //     glm::ivec2 vertexInfo; //begin, count
    //     uint32_t vertexSize;
    //     Handle<BufferAddress> indexBuffer;
    //     uint32_t indiceCount; 
    // };

    class Geometry : public MutableGPUResource<Geometry>
    {
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

            std::vector<GeometryBufferData> buffers;
            glm::ivec2 vertexInfo;
            Handle<GPUBuffer> indexBuffer;
            uint32_t indiceCount;
            GeometryGPUData CreateGPUData() {return GeometryGPUData{};}
            bool ReadyForUse(GeometryGPUData& content){ return true; };
            void SetContent(GeometryGPUData& content){ };
            void ReleaseData(GeometryGPUData& content){};
            void Release() {
                
                auto manager = std::shared_ptr<GPUResourceManager>(m_manager);

                for(auto& bufferData : buffers ){
                    manager->destroy(bufferData.buffer);
                }
                manager->destroy(indexBuffer);
             }
    };

    //geom data
    struct Vertex
    {
        glm::vec2 pos;
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
                {{0.0f, -0.5f}, {1.0f, 1.0f, 0.0f}},
                {{0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
                {{-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}}},
            .indices = {0U, 1U, 2U},
        };
    }

    static GeometryData squareVertices()
    {
        return {
            .vertices = {
                {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}},
                {{-0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}}},
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
                    {iw - (0.5 * width), jh - (0.5 * height)}, 
                    {iw, jh, 0.0f}
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


#ifndef BOITATAH_GEOMETRY_HPP
#define BOITATAH_GEOMETRY_HPP

#include <glm/glm.hpp>
#include <vector>
#include "../buffers/BufferStructs.hpp"
#include "../buffers/Buffer.hpp"
#include "../collections/Pool.hpp"
#include <array>
namespace boitatah
{

    struct GeometryDesc
    {
        glm::ivec2 vertexInfo;
        uint32_t vertexSize;
        uint32_t vertexDataSize;
        void *vertexData;
        uint32_t indexCount;
        void *indexData;
    };

    struct Geometry
    {
        std::vector<Handle<BufferAddress>> buffers;
        Handle<BufferAddress> indexBuffer;
        glm::ivec2 vertexInfo;
        uint32_t vertexSize;
        uint32_t indiceCount;
    };

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
                {{-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
            },
            .indices = {0, 1, 2},
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
        for(size_t j = 0; j <= heightSegments; j++){
            for(size_t i = 0; i <= widthSegments; i ++ ){
                float iw = i * w;
                float jh = j * h;

                vertices.push_back({
                    {iw - (0.5 * width), jh - (0.5 * height)}, 
                    {iw, jh, 0.0f}
                });
            }
        }

        for(size_t i = 0; i < widthSegments; i ++ ){
            for(size_t j = 0; j < heightSegments; j++){

                
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

#endif
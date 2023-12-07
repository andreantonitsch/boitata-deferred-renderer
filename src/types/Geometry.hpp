#ifndef BOITATAH_GEOMETRY_HPP
#define BOITATAH_GEOMETRY_HPP

#include <glm/glm.hpp>
#include <vector>
#include "Buffer.hpp"
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
        std::vector<Handle<BufferReservation>> buffers;
        std::vector<Handle<BufferReservation>> indexBuffer;
        glm::ivec2 vertexInfo;
        uint32_t vertexSize;
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
                {{-0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
                {{0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
                {{-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
                {{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            },
            .indices = {0, 1, 2, 2, 3, 0},
        };
    }

}

#endif
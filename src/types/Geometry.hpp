#ifndef BOITATAH_GEOMETRY_HPP
#define BOITATAH_GEOMETRY_HPP

#include <glm/glm.hpp>
#include <vector>
#include "Buffer.hpp"
#include "../collections/Pool.hpp"
#include <array>
namespace boitatah{
    

    struct GeometryDesc{
        glm::ivec2 vertexInfo; 
        uint32_t vertexSize;
        uint32_t dataSize;
        void* data;
    };

    struct Geometry{
        std::vector<Handle<BufferReservation>> buffers;
        glm::ivec2 vertexInfo;
        uint32_t vertexSize;
    };

    struct Vertex{
        glm::vec2 pos;
        glm::vec3 color;
    };

    static std::vector<Vertex> triangleVertices() {
        return {
            {{0.0f, -0.5f}, {1.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
         };
    }
}

#endif
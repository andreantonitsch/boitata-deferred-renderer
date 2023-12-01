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
        uint64_t vertexSize;
        uint64_t dataSize;
        void* data;
    };

    struct Geometry{
        uint32_t reservationCount;
        Handle<BufferReservation>* reservations;
        glm::ivec2 vertexInfo;
    };

    struct Vertex{
        glm::vec2 pos;
        glm::vec3 color;
    };

    static std::vector<Vertex> triangleVertices() {
        return {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
         };
    }
}

#endif
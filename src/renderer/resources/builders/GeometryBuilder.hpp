#pragma once

#include <initializer_list>

#include <buffers/BufferManager.hpp>
#include <types/BttEnums.hpp>
#include <types/Geometry.hpp>
#include <renderer/resources/GPUBuffer.hpp>
#include <renderer/Renderer.hpp>
#include <renderer/modules/GPUResourceManager.hpp>
#include <collections/Pool.hpp>
#include <memory>

namespace boitatah{
    using namespace buffer;

    class GeometryBuilder{
        private:
            GeometryCreateDescription m_description;
            std::shared_ptr<GPUResourceManager> m_manager;

            
        public:
            static GeometryBuilder createGeoemtry(std::shared_ptr<GPUResourceManager> manager);
            static GeometryBuilder createGeoemtry(std::shared_ptr<Renderer> renderer);

            GeometryBuilder& SetIndexes(std::initializer_list<uint32_t>&& indices);
            GeometryBuilder& SetIndexes(std::vector<uint32_t>& indices);
            GeometryBuilder& SetIndexes(Handle<GPUBuffer> indices, uint32_t count);
            GeometryBuilder& SetIndexes(uint32_t* indices, uint32_t count);
            
            template<typename T>
            GeometryBuilder& AddBuffer(std::initializer_list<T>&& buffer);
            template<typename T>
            GeometryBuilder& AddBuffer(std::vector<T>& buffer);
            template<typename T>
            GeometryBuilder& AddBuffer(std::vector<T>& buffer, uint32_t count, uint32_t stride);
            template<typename T>
            GeometryBuilder& AddBuffer(T* buffer, uint32_t count);            
            template<typename T>
            GeometryBuilder& AddBuffer(T* buffer, uint32_t count, uint32_t stride);

            GeometryBuilder& AddBuffer(Handle<GPUBuffer> buffer); //needs stride and count set in buffer

            GeometryBuilder& SetVertexInfo(uint32_t beginVertex, uint32_t vertexCount);

            Handle<Geometry> Finish();

            static Handle<Geometry> Triangle(GPUResourceManager& manager);
            static Handle<Geometry> Triangle(Renderer& renderer);

            static Handle<Geometry> Plane(GPUResourceManager& manager, float width, float height, uint32_t widthDiv, uint32_t heightDiv);
            static Handle<Geometry> Plane(Renderer& renderer, float width, float height, uint32_t widthDiv, uint32_t heightDiv);

            static Handle<Geometry> Sphere(GPUResourceManager& manager, float radius, uint32_t detail);
            static Handle<Geometry> Sphere(Renderer& renderer, float radius, uint32_t detail);

            static Handle<Geometry> Circle(GPUResourceManager& manager, float radius, uint32_t sides);
            static Handle<Geometry> Circle(Renderer& renderer, float radius, uint32_t sides);

            static Handle<Geometry> Cylinder(GPUResourceManager& manager, float radius, float height, uint32_t sides);
            static Handle<Geometry> Cylinder(Renderer& renderer);
    };

    template <typename T>
    inline GeometryBuilder &GeometryBuilder::AddBuffer(std::initializer_list<T> &&buffer)
    {
        AddBuffer(std::vector<T>(buffer));
        return *this;
    }

    template <typename T>
    inline GeometryBuilder &GeometryBuilder::AddBuffer(std::vector<T> &buffer)
    {
        m_description.bufferData.push_back(GeometryBufferDataDesc{
            .type = GEO_BUFFER_TYPE::Ptr,
            .vertexDataPtr = buffer.data(),
            .count = buffer.size(),
            .elementSize = sizeof(T)
        });
        return *this;
    }
    
    template <typename T>
    inline GeometryBuilder &GeometryBuilder::AddBuffer(std::vector<T> &buffer, uint32_t count, uint32_t stride)
    {
        m_description.bufferData.push_back(GeometryBufferDataDesc{
            .type = GEO_BUFFER_TYPE::Ptr,
            .vertexDataPtr = buffer.data(),
            .count = count,
            .elementSize = stride
        });
        return *this;
    }

    template <typename T>
    inline GeometryBuilder &GeometryBuilder::AddBuffer(T *buffer, uint32_t count)
    {
        m_description.bufferData.push_back(GeometryBufferDataDesc{
            .type = GEO_BUFFER_TYPE::Ptr,
            .vertexDataPtr = buffer,
            .count = count,
            .elementSize = sizeof(T)
        });
        return *this;
    }
    template <typename T>
    inline GeometryBuilder &GeometryBuilder::AddBuffer(T *buffer, uint32_t count, uint32_t stride)
    {
        m_description.bufferData.push_back(GeometryBufferDataDesc{
            .type = GEO_BUFFER_TYPE::Ptr,
            .vertexDataPtr = buffer,
            .count = count,
            .elementSize = stride
        });
        return *this;
    }
};
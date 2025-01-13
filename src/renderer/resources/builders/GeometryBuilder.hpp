#pragma once

#include <initializer_list>

#include <buffers/BufferManager.hpp>
#include <types/BttEnums.hpp>
#include <renderer/resources/Geometry.hpp>
#include <renderer/resources/GPUBuffer.hpp>


namespace boitatah{
    using namespace buffer;

    class GeometryBuilder{


        static GeometryBuilder createGeoemtry(GPUResourceManager& manager);

        GeometryBuilder SetIndexes(std::initializer_list<uint32_t> indices);
        GeometryBuilder SetIndexes(Handle<BufferAddress> indices);
        GeometryBuilder SetIndexes(Handle<GPUBuffer> indices);

        GeometryBuilder AddBuffer(std::initializer_list<uint32_t> indices);
        GeometryBuilder AddBuffer(Handle<BufferAddress> indices);
        GeometryBuilder AddBuffer(Handle<GPUBuffer> indices);

        GeometryBuilder SetVertexInfo(Handle<GPUBuffer> info);
        GeometryBuilder SetVertexInfo(uint32_t beginVertex, uint32_t vertexCount);

        GeometryBuilder SetVertexDescription();

        Geometry Finish();

    };


};
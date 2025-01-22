#include "GeometryBuilder.hpp"


namespace boitatah{

    GeometryBuilder GeometryBuilder::createGeoemtry(std::shared_ptr<GPUResourceManager> manager)
    {
        GeometryBuilder builder;
        builder.m_manager = manager;
        return builder;
    }

    GeometryBuilder GeometryBuilder::createGeoemtry(std::shared_ptr<Renderer> renderer)
    {
        GeometryBuilder builder;
        builder.m_manager = std::shared_ptr<GPUResourceManager>(&renderer->getResourceManager());
        return builder;
    }

    GeometryBuilder &GeometryBuilder::SetIndexes(std::initializer_list<uint32_t> &&indices)
    {
        m_description.indexData.type = GEO_BUFFER_TYPE::UIntVector;
        m_description.indexData.count = indices.size();
        m_description.indexData.index_vector = std::vector<uint32_t>(indices);
        return *this;
    }

    GeometryBuilder &GeometryBuilder::SetIndexes(std::vector<uint32_t> &indices)
    {
        m_description.indexData.type = GEO_BUFFER_TYPE::UIntVector;
        m_description.indexData.count = indices.size();
        m_description.indexData.index_vector = indices;
        return *this;
    }

    GeometryBuilder &GeometryBuilder::SetIndexes(Handle<GPUBuffer> indices, uint32_t count)
    {
        m_description.indexData.type = GEO_BUFFER_TYPE::GPUBuffer;
        m_description.indexData.buffer = indices;
        m_description.indexData.count = count;
        return *this;
    }
    GeometryBuilder &GeometryBuilder::SetIndexes(uint32_t *indices, uint32_t count)
    {
        m_description.indexData.type = GEO_BUFFER_TYPE::Ptr;
        m_description.indexData.dataPtr = indices;
        m_description.indexData.count = count;
        return *this;
    }

    GeometryBuilder &GeometryBuilder::AddBuffer(Handle<GPUBuffer> buffer)
    {
        m_description.bufferData.push_back(GeometryBufferDataDesc{
            .type = GEO_BUFFER_TYPE::GPUBuffer,
            .buffer = buffer,
        });
        return *this;
    }
    GeometryBuilder &GeometryBuilder::SetVertexInfo(uint32_t beginVertex, uint32_t vertexCount)
    {
        m_description.vertexInfo = glm::vec2(beginVertex, vertexCount);
        return *this;
    }
    Handle<Geometry> GeometryBuilder::Finish()
    {
        return m_manager->create(m_description);
    }
}

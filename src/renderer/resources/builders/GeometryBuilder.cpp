#include "GeometryBuilder.hpp"


namespace boitatah{

    GeometryBuilder GeometryBuilder::createGeoemtry(GPUResourceManager& manager)
    {
        GeometryBuilder builder(manager);
        return builder;
    }

    GeometryBuilder GeometryBuilder::createGeoemtry(Renderer& renderer)
    {
        GeometryBuilder builder(renderer.getResourceManager());
        return builder;
    }

    GeometryBuilder& GeometryBuilder::SetIndexes(std::initializer_list<uint32_t> &&indices)
    {
        m_description.indexData.data_type = GEO_DATA_TYPE::UIntVector;
        m_description.indexData.count = indices.size();
        m_description.indexData.index_vector = std::vector<uint32_t>(indices);
        return *this;
    }

    GeometryBuilder& GeometryBuilder::SetIndexes(std::vector<uint32_t> &indices)
    {
        m_description.indexData.data_type = GEO_DATA_TYPE::Ptr;
        m_description.indexData.count = indices.size();
        m_description.indexData.dataPtr = indices.data();
        return *this;
    }

    GeometryBuilder& GeometryBuilder::SetIndexes(Handle<GPUBuffer> indices, uint32_t count)
    {
        m_description.indexData.data_type = GEO_DATA_TYPE::GPUBuffer;
        m_description.indexData.buffer = indices;
        m_description.indexData.count = count;
        return *this;
    }
    GeometryBuilder& GeometryBuilder::SetIndexes(uint32_t *indices, uint32_t count)
    {
        m_description.indexData.data_type = GEO_DATA_TYPE::Ptr;
        m_description.indexData.dataPtr = indices;
        m_description.indexData.count = count;
        return *this;
    }

    GeometryBuilder& GeometryBuilder::AddBuffer(VERTEX_BUFFER_TYPE type, Handle<GPUBuffer> buffer)
    {
        m_description.bufferData.push_back(GeometryBufferDataDesc{
            .buffer_type = type,
            .data_type = GEO_DATA_TYPE::GPUBuffer,
            .buffer = buffer,
        });
        return *this;
    }
    GeometryBuilder& GeometryBuilder::SetVertexInfo(uint32_t vertexCount, uint32_t beginVertex)
    {
        m_description.vertexInfo = glm::vec2(vertexCount, beginVertex);
        return *this;
    }
    Handle<Geometry> GeometryBuilder::Finish()
    {
        return m_manager.create(m_description);
    }
}

#include "GeometryBuilder.hpp"


namespace boitatah{



    Handle<Geometry> GeometryBuilder::geometryFromGeometryData(const GeometryBuildData &data)
    {
        Handle<Geometry> geo = SetVertexInfo(data.vertices.size(), 0)
                            .SetIndexes(data.indices)
                            .AddBuffer(VERTEX_BUFFER_TYPE::POSITION, data.vertices)
                            .AddBuffer(VERTEX_BUFFER_TYPE::UV, data.uv)
                            .AddBuffer(VERTEX_BUFFER_TYPE::COLOR, data.color)
                            .Finish();
        return  geo;
    }

    GeometryBuilder GeometryBuilder::createGeometry(GPUResourceManager &manager)
    {
        GeometryBuilder builder(manager);
        return builder;
    }

    GeometryBuilder GeometryBuilder::createGeometry(Renderer& renderer)
    {
        GeometryBuilder builder(renderer.getResourceManager());
        return builder;
    }

    GeometryBuilder& GeometryBuilder::SetIndexes(const std::initializer_list<uint32_t> &&indices)
    {
        m_description.indexData.data_type = GEO_DATA_TYPE::UIntVector;
        m_description.indexData.count = indices.size();
        m_description.indexData.index_vector = std::vector<uint32_t>(indices);
        return *this;
    }

    GeometryBuilder& GeometryBuilder::SetIndexes(const std::vector<uint32_t> &indices)
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

    Handle<Geometry> GeometryBuilder::Triangle(GPUResourceManager &manager)
    {
        auto builder = createGeometry(manager);
        Handle<Geometry> geo = builder.geometryFromGeometryData(triangleVertices());
        return geo;
    }
    Handle<Geometry> GeometryBuilder::Triangle(Renderer &renderer)
    {
        return GeometryBuilder::Triangle(renderer.getResourceManager());
    }
    Handle<Geometry> GeometryBuilder::Quad(GPUResourceManager &manager)
    {
        auto builder = createGeometry(manager);
        Handle<Geometry> geo = builder.geometryFromGeometryData(quadVertices());
        return geo;
    }
    Handle<Geometry> GeometryBuilder::Quad(Renderer &renderer)
    {
        return GeometryBuilder::Quad(renderer.getResourceManager());
    }
}

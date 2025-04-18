#include "GeometryBuilder.hpp"

#include <utils/utils.hpp>
namespace boitatah{



    Handle<Geometry> GeometryBuilder::geometryFromGeometryData(const GeometryBuildData &data)
    {
        Handle<Geometry> geo = SetVertexInfo(data.vertices.size(), 0)
                            .SetIndexes(data.indices)
                            .AddBuffer(VERTEX_BUFFER_TYPE::POSITION, data.vertices)
                            .AddBuffer(VERTEX_BUFFER_TYPE::UV, data.uv)
                            .AddBuffer(VERTEX_BUFFER_TYPE::NORMAL, data.normal)
                            .AddBuffer(VERTEX_BUFFER_TYPE::COLOR, data.color)
                            .Finish();
        return  geo;
    }

    GeometryBuilder GeometryBuilder::createGeometry(GPUResourceManager &manager)
    {
        GeometryBuilder builder(manager);
        return builder;
    }

    // GeometryBuilder GeometryBuilder::createGeometry(Renderer& renderer)
    // {
    //     GeometryBuilder builder(renderer.getResourceManager());
    //     return builder;
    // }

    GeometryBuilder GeometryBuilder::createProceduralGeometry(GPUResourceManager &manager)
    {
        GeometryBuilder builder(manager);
        return builder;
    }

    // GeometryBuilder GeometryBuilder::createProceduralGeometry(Renderer &renderer)
    // {
    //     GeometryBuilder builder(renderer.getResourceManager());
    //     return builder;
    // }

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

    GeometryBuilder &GeometryBuilder::addTriangle()
    {
        procedural_geometry = procedural_geometry + triangleVertices().transform(transform);
        return *this;
    }

    GeometryBuilder &GeometryBuilder::ApplySetProceduralTransform(  glm::vec3   position, 
                                                                    glm::vec3   scale, 
                                                                    glm::vec3   rotation)
    {
        procedural_geometry.transform(utils::getTransformMatrix(position, scale, rotation));
        return *this;
    }

    GeometryBuilder &GeometryBuilder::addQuad()
    {
        procedural_geometry = procedural_geometry + quadVertices().transform(transform);
        return *this;
    }

    GeometryBuilder &GeometryBuilder::addPlane( float       width,
                                                float       height, 
                                                uint32_t    widthDiv, 
                                                uint32_t    heightDiv)
    {
        procedural_geometry = procedural_geometry + planeVertices(
                                                            width,
                                                            height,
                                                            widthDiv,
                                                            heightDiv)
                                                    .transform(transform);
        return *this;
    }

    GeometryBuilder &GeometryBuilder::addCircle(float radius, uint32_t sides)
    {
        procedural_geometry = procedural_geometry + circle( radius,
                                                            sides)
                                                    .transform(transform);
        return *this;
    }



    Handle<Geometry> GeometryBuilder::Triangle(
                                            GPUResourceManager &manager)
    {
        auto builder = createGeometry(manager);
        Handle<Geometry> geo = builder.geometryFromGeometryData(triangleVertices());
        return geo;
    }

    // Handle<Geometry> GeometryBuilder::Triangle(
    //                                         Renderer &renderer)
    // {
    //     return GeometryBuilder::Triangle(renderer.getResourceManager());
    // }

    Handle<Geometry> GeometryBuilder::Quad(
                                        GPUResourceManager &manager)
    {
        auto builder = createGeometry(manager);
        Handle<Geometry> geo = builder.geometryFromGeometryData(quadVertices());
        return geo;
    }

    Handle<Geometry> GeometryBuilder::Plane(
                                        GPUResourceManager  &manager, 
                                        float               width, 
                                        float               height, 
                                        uint32_t            widthDiv, 
                                        uint32_t            heightDiv)
    {
        auto builder = createGeometry(manager);
        Handle<Geometry> geo = builder.geometryFromGeometryData(
                                                planeVertices(  width, 
                                                                height, 
                                                                widthDiv, 
                                                                heightDiv));
        return geo;
    }

    // Handle<Geometry> GeometryBuilder::Plane(Renderer    &renderer, 
    //                                         float       width, 
    //                                         float       height, 
    //                                         uint32_t    widthDiv, 
    //                                         uint32_t    heightDiv)
    // {
    //     return GeometryBuilder::Plane(  renderer.getResourceManager(), 
    //                                     width, 
    //                                     height, 
    //                                     widthDiv, 
    //                                     heightDiv);
    // }

    Handle<Geometry> GeometryBuilder::Circle(GPUResourceManager &manager, float radius, uint32_t sides)
    {
        auto builder = createGeometry(manager);
        Handle<Geometry> geo = builder.geometryFromGeometryData(circle(radius, sides));
        return geo;
    }
    // Handle<Geometry> GeometryBuilder::Circle(Renderer &renderer, float radius, uint32_t sides)
    // {
    //      return GeometryBuilder::Circle(renderer.getResourceManager(), radius, sides);
    // }

    Handle<Geometry> GeometryBuilder::Cylinder(GPUResourceManager &manager, float radius, float height, float heightSegments, uint32_t sides)
    {
        auto builder = createGeometry(manager);
        auto geo = builder.geometryFromGeometryData(cylinder(radius, height, heightSegments, sides));
        return geo;
    }

    // Handle<Geometry> GeometryBuilder::Cylinder(Renderer &renderer, float radius, float height, float heightSegments, uint32_t sides)
    // {
    //      return GeometryBuilder::Cylinder(renderer.getResourceManager(), radius, height, heightSegments, sides);
    // }

    Handle<Geometry> GeometryBuilder::Pipe(GPUResourceManager &manager, float radius, float height, float heightSegments, uint32_t sides)
    {
        auto builder = createGeometry(manager);
        Handle<Geometry> geo = builder.geometryFromGeometryData(pipe(radius, height, heightSegments, sides));
        return geo;
    }

    // Handle<Geometry> GeometryBuilder::Pipe(Renderer &renderer, float radius, float height, float heightSegments, uint32_t sides)
    // {
    //     return GeometryBuilder::Pipe(renderer.getResourceManager(), radius, height, heightSegments, sides);
    // }
    
    // Handle<Geometry> GeometryBuilder::Quad(Renderer &renderer)
    // {
    //     return GeometryBuilder::Quad(renderer.getResourceManager());
    // }


    Handle<Geometry> GeometryBuilder::Icosahedron(
                                            GPUResourceManager &manager)
    {
        auto builder = createGeometry(manager);
        Handle<Geometry> geo = builder.geometryFromGeometryData(icosahedron());
        return geo;
    }

    // Handle<Geometry> GeometryBuilder::Icosahedron(Renderer& renderer)
    // {
    //     return GeometryBuilder::Icosahedron(renderer.getResourceManager());
    // }


}

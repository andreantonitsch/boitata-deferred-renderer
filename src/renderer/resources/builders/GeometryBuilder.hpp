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

    struct GeometryBuildData
        {
            std::vector<glm::vec3> vertices;
            std::vector<glm::vec3> color;
            std::vector<glm::vec2> uv;
            
            std::vector<uint32_t> indices;
        };


        static GeometryBuildData triangleVertices()
        {
            return {
                .vertices = {
                    {0.0f, -0.5f, 0.0f},
                    {0.5f, 0.5f, 0.0f},
                    {-0.5f, 0.5f, 0.0f},},

                .color ={{1.0f, 1.0f, 0.0f}, 
                        {1.0f, 0.0f, 1.0f},
                        {0.0f, 1.0f, 1.0f}},
                
                .uv = {{0.5f, 0.0f},
                    {0.0f, 1.0f},
                    {1.0f, 1.0f}},

                .indices = {0U, 1U, 2U},
            };
        }

        static GeometryBuildData quadVertices()
        {
            return {
                .vertices = {
                    {-0.5f, -0.5f, 0.0f}, 
                    {0.5f, -0.5f, 0.0f}, 
                    {-0.5f, 0.5f, 0.0f}, 
                    {0.5f, 0.5f, 0.0f},},

                .color ={{1.0f, 1.0f, 0.0f}, 
                        {1.0f, 0.0f, 1.0f},
                        {1.0f, 0.0f, 1.0f},
                        {0.0f, 1.0f, 1.0f}},

                .uv = {{0.5f, 0.0f},
                    {0.0f, 1.0f},
                    {0.0f, 1.0f},
                    {1.0f, 1.0f}},

                .indices = {0U, 1U, 2U,
                            1U, 3U, 2U,
                }
            };
        }


        static  GeometryBuildData planeVertices(const float width,
                                        const float height,
                                        const uint32_t widthSegments,
                                        const uint32_t heightSegments)
        {
            std::vector<glm::vec3> vertices;
            std::vector<glm::vec3> color;
            std::vector<glm::vec2> uv;
            std::vector<uint32_t> indices;

            float w = width / widthSegments;
            float h = height / heightSegments;
            for(uint32_t j = 0; j <= heightSegments; j++){
                for(uint32_t i = 0; i <= widthSegments; i ++ ){
                    float iw = i * w;
                    float jh = j * h;

                    vertices.push_back(
                        {iw - (0.5 * width), jh - (0.5 * height), 0.0f});
                    color.push_back({iw, jh, 0.0f});
                    uv.push_back({iw,jh});
                }
                        
            }

            for(uint32_t i = 0; i < widthSegments; i ++ ){
                for(uint32_t j = 0; j < heightSegments; j++){

                    
                    indices.push_back(j * (widthSegments+1) + i);
                    indices.push_back(j * (widthSegments+1) + i + 1);
                    indices.push_back((j+1) * (widthSegments+1) + i);


                    indices.push_back(j * (widthSegments+1) + i + 1);
                    indices.push_back((j+1) * (widthSegments+1) + i + 1);
                    indices.push_back((j+1) *(widthSegments+1) + i);
                }
            }
            
            return {
            .vertices = vertices, 
            .color = color,
            .uv = uv,
            .indices = indices};
        }

    class GeometryBuilder{
        private:


            GeometryCreateDescription m_description;
            GPUResourceManager& m_manager;

            template<typename T>
            GeometryBuilder& AddBuffer(VERTEX_BUFFER_TYPE type, std::initializer_list<T> buffer);
            
            Handle<Geometry> geometryFromGeometryData(const GeometryBuildData& data);
            
        public:
            static GeometryBuilder createGeometry(GPUResourceManager& manager);
            static GeometryBuilder createGeometry(Renderer& renderer);

            GeometryBuilder(GPUResourceManager& manager) : m_manager(manager){};

            GeometryBuilder& SetIndexes(const std::initializer_list<uint32_t>&& indices);
            GeometryBuilder& SetIndexes(const std::vector<uint32_t>& indices);
            GeometryBuilder& SetIndexes(Handle<GPUBuffer> indices, uint32_t count);
            GeometryBuilder& SetIndexes(uint32_t* indices, uint32_t count);
            
            ///
            // template<typename ...Ts>
            // GeometryBuilder& AddBuffer(VERTEX_BUFFER_TYPE type, Ts... buffer);

            template<typename T>
            GeometryBuilder& AddBuffer(VERTEX_BUFFER_TYPE type, const std::vector<T>& buffer);
            template<typename T>
            GeometryBuilder& AddBuffer(VERTEX_BUFFER_TYPE type, const std::vector<T>& buffer, uint32_t count, uint32_t stride);
            template<typename T>
            GeometryBuilder& AddBuffer(VERTEX_BUFFER_TYPE type, T* buffer, uint32_t count);            
            template<typename T>
            GeometryBuilder& AddBuffer(VERTEX_BUFFER_TYPE type, T* buffer, uint32_t count, uint32_t stride);

            GeometryBuilder& AddBuffer(VERTEX_BUFFER_TYPE type, Handle<GPUBuffer> buffer); //needs stride and count set in buffer

            GeometryBuilder& SetVertexInfo(uint32_t beginVertex, uint32_t vertexCount);

            Handle<Geometry> Finish();

            static Handle<Geometry> Triangle(GPUResourceManager& manager);
            static Handle<Geometry> Triangle(Renderer& renderer);

            static Handle<Geometry> Quad(GPUResourceManager& manager);
            static Handle<Geometry> Quad(Renderer& renderer);

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
    inline GeometryBuilder& GeometryBuilder::AddBuffer(VERTEX_BUFFER_TYPE type, std::initializer_list<T> buffer)
    {
        AddBuffer(type, std::vector<T>(buffer));
        return *this;
    }

    template <typename T>
    inline GeometryBuilder& GeometryBuilder::AddBuffer(VERTEX_BUFFER_TYPE type,const std::vector<T> &buffer)
    {
        m_description.bufferData.push_back(GeometryBufferDataDesc{
            .buffer_type = type,
            .data_type = GEO_DATA_TYPE::Ptr,
            .vertexCount = static_cast<uint32_t>(buffer.size()),
            .vertexSize = sizeof(T),
            .vertexDataPtr = buffer.data(),
        });
        return *this;
    }
    
    template <typename T>
    inline GeometryBuilder& GeometryBuilder::AddBuffer(VERTEX_BUFFER_TYPE type, const std::vector<T> &buffer, uint32_t count, uint32_t stride)
    {
        m_description.bufferData.push_back(GeometryBufferDataDesc{
            .buffer_type = type,
            .data_type = GEO_DATA_TYPE::Ptr,
            .vertexCount = count,
            .vertexSize = stride,
            .vertexDataPtr = buffer.data(),
        });
        return *this;
    }

    template <typename T>
    inline GeometryBuilder& GeometryBuilder::AddBuffer(VERTEX_BUFFER_TYPE type, T *buffer, uint32_t count)
    {
        m_description.bufferData.push_back(GeometryBufferDataDesc{
            .buffer_type = type,
            .data_type = GEO_DATA_TYPE::Ptr,
            .vertexCount = count,
            .vertexSize = sizeof(T),
            .vertexDataPtr = buffer,
        });
        return *this;
    }
    template <typename T>
    inline GeometryBuilder& GeometryBuilder::AddBuffer(VERTEX_BUFFER_TYPE type, T *buffer, uint32_t count, uint32_t stride)
    {
        m_description.bufferData.push_back(GeometryBufferDataDesc{
            .buffer_type = type,
            .data_type = GEO_DATA_TYPE::Ptr,
            .vertexCount = count,
            .vertexSize = stride,
            .vertexDataPtr = buffer,
        });
        return *this;
    }
    // template <typename... Ts>
    // inline GeometryBuilder &GeometryBuilder::AddBuffer(VERTEX_BUFFER_TYPE type, Ts... buffer)
    // {
    //     return AddBuffer(type, {buffer...});
    // }

};
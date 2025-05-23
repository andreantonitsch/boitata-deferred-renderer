#pragma once

#include <initializer_list>
#include <memory>

#include <boitatah/buffers/BufferManager.hpp>
#include <boitatah/BoitatahEnums.hpp>
#include <boitatah/resources/Geometry.hpp>
#include <boitatah/resources/GPUBuffer.hpp>
#include <boitatah/modules/GPUResourceManager.hpp>
#include <boitatah/collections.hpp>
#include <boitatah/utils/utils.hpp>

namespace boitatah{

    struct GeometryBuildData
    {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> color;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normal;
        std::vector<uint32_t> indices;

        void clear(){

            vertices.clear();
            color.clear();
            uv.clear();
            normal.clear();
            indices.clear();
        }

        GeometryBuildData& transform(glm::mat4x4 transform_matrix){
            
            for(uint32_t i = 0; i < vertices.size(); i++){
                vertices[i] = transform_matrix * glm::vec4(vertices[i], 1.0f);
                normal[i] = transform_matrix * glm::vec4(normal[i], 0.0f);
            }
            return *this;
        };

        GeometryBuildData operator+(GeometryBuildData& rhs){
            
            GeometryBuildData res{};

            uint32_t lhs_index_count = indices.size();
            uint32_t lhs_vertex_count = vertices.size();
            uint32_t rhs_vertex_count = rhs.vertices.size();
            uint32_t rhs_index_count = rhs.indices.size();


            utils::concatenate_vectors(res.vertices, vertices);
            utils::concatenate_vectors(res.color, color);
            utils::concatenate_vectors(res.uv, uv);
            utils::concatenate_vectors(res.normal, normal);

            utils::concatenate_vectors(res.vertices, rhs.vertices);
            utils::concatenate_vectors(res.color, rhs.color);
            utils::concatenate_vectors(res.uv, rhs.uv);
            utils::concatenate_vectors(res.normal, rhs.normal);

            for(uint32_t i = 0; i < lhs_index_count; i++){
                res.indices.push_back(indices[i]);
            }

            for(uint32_t i = 0; i < rhs_index_count; i++){
                res.indices.push_back(rhs.indices[i] + lhs_vertex_count);
            }

            return res;
        }

        GeometryBuildData& operator+=(GeometryBuildData& rhs){
            
            uint32_t lhs_index_count = indices.size();
            uint32_t lhs_vertex_count = vertices.size();
            uint32_t rhs_vertex_count = rhs.vertices.size();
            uint32_t rhs_index_count = rhs.indices.size();


            utils::concatenate_vectors(vertices, rhs.vertices);
            utils::concatenate_vectors(color, rhs.color);
            utils::concatenate_vectors(uv, rhs.uv);
            utils::concatenate_vectors(normal, rhs.normal);

            for(uint32_t i = 0; i < rhs_index_count; i++){
                indices.push_back(rhs.indices[i] + lhs_vertex_count);
            }

            return *this;
        }

        // moves all indices from src to dst, then deletes dest and fixes indices.
        GeometryBuildData& merge_vertex(uint32_t dest, uint32_t src){
            return *this;
        }

    };


    static  constexpr  GeometryBuildData triangleVertices()
    {
        return {
            .vertices = {
                {0.0f, -0.5f, 0.0f},
                {0.5f, 0.5f, 0.0f},
                {-0.5f, 0.5f, 0.0f},},

            .color ={{1.0f, 1.0f, 1.0f}, 
                    {1.0f, 1.0f, 1.0f},
                    {1.0f, 1.0f, 1.0f}},
            
            .uv = { {0.5f, 0.0f},
                    {1.0f, 1.0f},
                    {0.0f, 1.0f}},

            .normal = {
                {0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, 1.0f}},

            .indices = {0U, 1U, 2U},
            
        };
    }

    static constexpr  GeometryBuildData quadVertices()
    {
        return {
            .vertices = {
                {-1.0f, -1.0f, 0.0f}, 
                {1.0f, -1.0f, 0.0f}, 
                {-1.0f, 1.0f, 0.0f}, 
                {1.0f, 1.0f, 0.0f},},

            .color ={{1.0f, 1.0f, 1.0f}, 
                    {1.0f, 1.0f, 1.0f},
                    {1.0f, 1.0f, 1.0f},
                    {1.0f, 1.0f, 1.0f}},

            .uv = {{0.0f, 0.0f},
                {1.0f, 0.0f},
                {0.0f, 1.0f},
                {1.0f, 1.0f}},

            .normal = {
                {0.0f, 0.0f, -1.0f},
                {0.0f, 0.0f, -1.0f},
                {0.0f, 0.0f, -1.0f},
                {0.0f, 0.0f, -1.0f},},

            .indices = {0U, 1U, 2U,
                        1U, 3U, 2U,
            },
            
        };
    }


    static  constexpr GeometryBuildData planeVertices(const float width,
                                    const float height,
                                    const uint32_t widthSegments,
                                    const uint32_t heightSegments)
    {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> color;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normal;
        std::vector<uint32_t> indices;

        float w = width / widthSegments;
        float h = height / heightSegments;
        for(uint32_t j = 0; j <= heightSegments; j++){
            for(uint32_t i = 0; i <= widthSegments; i ++ ){
                float iw = i * w;
                float jh = j * h;

                vertices.push_back(
                {iw - (0.5 * width), jh - (0.5 * height), 0.0f});
                color.push_back({1.0, 1.0, 1.0f});
                uv.push_back({iw,jh});
                normal.push_back({0.0f, 0.0f, 1.0f});
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
        .normal = normal,
        .indices = indices};
    }

    //pre condition: sides <= 3
    static constexpr GeometryBuildData circle(const float radius, const uint32_t sides){
        
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> color;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normal;
        std::vector<uint32_t> indices;
        
        //place center
        vertices.push_back({0.0, 0.0, 0.0});
        color.push_back({1.0, 1.0, 1.0});
        uv.push_back({0.5, 0.5});
        normal.push_back({0.0, 0.0, 1.0});

        float ratio =  (2 * glm::pi<float>()) / static_cast<float>(sides);
        

        uint32_t vertex_count = 1;
        for(uint32_t j = 1; j <= (sides+1); j++){
                float angle = static_cast<float>(j) * ratio;
                float x = glm::cos(angle);
                float y = glm::sin(angle);
                vertices.push_back({x * radius, y * radius, 0.0 });
                color.push_back({1.0, 1.0, 1.0f});
                uv.push_back({x / 2.0 + 0.5,y / 2.0 + 0.5});
                normal.push_back({0.0, 0.0f, 1.0});

                if((j <= sides)){
                    indices.push_back(vertex_count);
                    indices.push_back(vertex_count + 1);
                    indices.push_back(0);
                }
                vertex_count++;
            }
        
        return {
        .vertices = vertices, 
        .color = color,
        .uv = uv,
        .normal = normal,
        .indices = indices};
    };


    //pre condition: sides <= 3
    static constexpr GeometryBuildData pipe(const float         radius, 
                                            const float         height,
                                            const uint32_t      heightSegments, 
                                            const uint32_t      sides){
        
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> color;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normal;
        std::vector<uint32_t> indices;
        

        float angle_ratio =         (2 * glm::pi<float>()) / static_cast<float>(sides);
        float height_ratio =        height / static_cast<float>(heightSegments);
        uint32_t total_vertices =   (sides + 1) * (heightSegments + 1);

        uint32_t vertex_count = 0;
        for(uint32_t j = 0; j <= (sides); j++){
                float angle = static_cast<float>(j) * angle_ratio;
                float x = glm::cos(angle);
                float z = glm::sin(angle);
            for(uint32_t i = 0; i <= heightSegments; i++){
                float segmentHeight = height_ratio * i - (height * 0.5);
                vertices.push_back({x * radius, segmentHeight, z * radius });
                color.push_back({1.0, 1.0, 1.0f});
                uv.push_back({angle,segmentHeight / height});
                normal.push_back({x, 0.0f, z});

                if(i != heightSegments && (j < sides)){
                    indices.push_back(vertex_count);
                    indices.push_back((vertex_count + heightSegments + 1));
                    indices.push_back(vertex_count + 1);

                    indices.push_back((vertex_count + heightSegments + 1));
                    indices.push_back((vertex_count + heightSegments + 2));
                    indices.push_back(vertex_count + 1);
                }
                vertex_count++;
            }
        }
        
        return {
        .vertices = vertices, 
        .color = color,
        .uv = uv,
        .normal = normal,
        .indices = indices};
    };

    //pre condition: sides >= 3
    static constexpr GeometryBuildData cylinder(const float         radius, 
                                                const float         height,
                                                const uint32_t      heightSegments, 
                                                const uint32_t      sides){

        auto cyl = pipe(radius, height, heightSegments, sides);

        float half_height = height / 2;

        auto c = circle(radius, sides);
        glm::mat4 m_transform = glm::mat4(1.0f);
        m_transform = glm::eulerAngleXYZ(glm::radians(90.0f), 0.0f, 0.0f) * m_transform;
        m_transform = glm::translate(m_transform, glm::vec3(0.0f, 0.0f, half_height));
        c.transform(m_transform);

        cyl += c;
        
        c = circle(radius, sides);
        m_transform = glm::mat4(1.0f);
        m_transform = glm::eulerAngleXYZ(glm::radians(-90.0f), 0.0f, 0.0f) * m_transform;
        m_transform = glm::translate(m_transform, glm::vec3(0.0f, 0.0f, -half_height));
        c.transform(m_transform);

        cyl += c;

        return cyl;
    };

    //pre condition: longitude_segments >= 3
    //pre condition: latitude_segments >= 3
    static constexpr GeometryBuildData uv_sphere(const float         radius, 
                                                 const uint32_t      longitude_segments,
                                                 const uint32_t      latitude_segments){
        
        auto sphere = planeVertices(1.0f, 1.0f, longitude_segments, latitude_segments);

        for(uint32_t i = 0; i < sphere.vertices.size(); i++){

            auto uv = sphere.uv[i];
            auto lat = uv.y * glm::pi<float>() - glm::half_pi<float>();
            auto lon = uv.x * glm::two_pi<float>();
            sphere.vertices[i] = {glm::cos(lon) * glm::cos(lat),
                                  glm::sin(lat),
                                  glm::sin(lon)* glm::cos(lat)};
            sphere.normal[i] = glm::normalize(sphere.vertices[i]);
        }

        //TODO remove extra vertices

        return sphere;
    };

    static constexpr GeometryBuildData icosahedron(){

        auto phi = glm::golden_ratio<float>();

        auto ihp = 1.0f / phi;

        GeometryBuildData ico{};

        auto center = glm::vec3(0.0f);

        ico.vertices.push_back({0, ihp, -1}); //0
        ico.vertices.push_back({ihp, 1, 0});  //1
        ico.vertices.push_back({-ihp, 1, 0}); //2
        ico.vertices.push_back({0, ihp, 1});  //3
        ico.vertices.push_back({0, -ihp, 1}); //4

        ico.vertices.push_back({-1, 0, ihp}); //5
        ico.vertices.push_back({0, -ihp, -1});//6
        ico.vertices.push_back({1, 0, -ihp});  //7
        ico.vertices.push_back({1, 0, ihp}); //8
        ico.vertices.push_back({-1, 0, -ihp}); //9

        ico.vertices.push_back({ihp, -1, 0}); //10
        ico.vertices.push_back({-ihp, -1,  0}); //11

        for(int i = 0; i < 12; i++)
            ico.color.push_back(glm::vec3(1));

        for(int i = 0; i < 12; i++){
            
            auto vert = ico.vertices[i];
            auto u = glm::atan2<float>(vert.z, vert.x) * glm::one_over_two_pi<float>();
            // auto u = glm::atan2<float>(glm::vec2(vert.z, vert.x)) / glm::two_pi<float>();
            auto v = asin(vert.y) / glm::pi<float>() + 0.5f;
            ico.uv.push_back({u,v});
        }

        for(int i = 0; i < 12; i++){
            ico.vertices[i] = glm::normalize(ico.vertices[i]);
            ico.normal.push_back(ico.vertices[i]);
        }

        //faces
        ico.indices = { 1,   2,   0, //
                        2,   1,   3,
                        4,   5,   3,
                        8,   4,   3,
                        6,   7,   0,
                        9 ,  6 ,  0,
                        10,  11,  4,
                        11,   10,   6,
                        5,   9,   2, 
                        9,   5,   11,
                        7,   8,   1,
                        8,   7,   10,
                        5,   2,   3,
                        1,   8,   3,
                        2,   9,  0,
                        7,   1,   0,
                        9,   11,   6,
                        10,  7,  6,
                        11,  5,   4,
                        8,   10,  4  };
        

        return ico;

    };

    //https://catlikecoding.com/unity/tutorials/procedural-meshes/cube-sphere/
    static constexpr GeometryBuildData cube();
    static constexpr GeometryBuildData cube_sphere();
    static constexpr GeometryBuildData to_sphere(GeometryBuildData& mesh);
    static constexpr GeometryBuildData sub_div(GeometryBuildData& mesh);

 
    class GeometryBuilder{
        private:


            GeometryCreateDescription m_description;
            GPUResourceManager& m_manager;

            template<typename T>
            GeometryBuilder& AddBuffer(VERTEX_BUFFER_TYPE type, std::initializer_list<T> buffer);
            
            glm::mat4 transform = glm::mat4(1.0f);
            GeometryBuildData procedural_geometry;

            Handle<Geometry> geometryFromGeometryData(const GeometryBuildData& data);
        public:
            static GeometryBuilder createGeometry(GPUResourceManager& manager);
            //static GeometryBuilder createGeometry(Renderer& renderer);
            static std::vector<glm::vec3> computeSmoothNormals(const std::vector<glm::vec3>& vertices);
            static std::vector<glm::vec3> computeFlatNormals(const std::vector<glm::vec3>& vertices);

            static GeometryBuilder createProceduralGeometry(GPUResourceManager& manager);
            //static GeometryBuilder createProceduralGeometry(Renderer& renderer);

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

            GeometryBuilder& SetProceduralTransform(
                                                            glm::vec3        position,
                                                            glm::vec3        scale,
                                                            glm::vec3        rotation);
            GeometryBuilder& addTriangle();

            GeometryBuilder &ApplySetProceduralTransform(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation);

            GeometryBuilder& addQuad();

            GeometryBuilder& addPlane(
                                     float      width,
                                     float      height, 
                                     uint32_t   widthDiv, 
                                     uint32_t   heightDiv);

            GeometryBuilder& addSphere(
                                    float       radius, 
                                    uint32_t    detail);

            GeometryBuilder& addCircle(
                                    float       radius, 
                                    uint32_t    sides);

            GeometryBuilder& addCylinder(
                                    float       radius, 
                                    float       height, 
                                    float       heightSegments, 
                                    uint32_t    sides);


            static Handle<Geometry> Triangle(GPUResourceManager& manager);
            // static Handle<Geometry> Triangle(Renderer& renderer);

            static Handle<Geometry> Quad(GPUResourceManager& manager);
            // static Handle<Geometry> Quad(Renderer& renderer);

            static Handle<Geometry> Plane(GPUResourceManager& manager, float width, float height, uint32_t widthDiv, uint32_t heightDiv);
            // static Handle<Geometry> Plane(Renderer& renderer, float width, float height, uint32_t widthDiv, uint32_t heightDiv);

            static Handle<Geometry> Sphere(GPUResourceManager& manager, float radius, uint32_t detail);
            // static Handle<Geometry> Sphere(Renderer& renderer, float radius, uint32_t detail);

            static Handle<Geometry> Circle(GPUResourceManager& manager, float radius, uint32_t sides);
            // static Handle<Geometry> Circle(Renderer& renderer, float radius, uint32_t sides);

            static Handle<Geometry> Cylinder(GPUResourceManager& manager, float radius, float height, float heightSegments, uint32_t sides);
            // static Handle<Geometry> Cylinder(Renderer& renderer, float radius, float height, float heightSegments, uint32_t sides);
            
            static Handle<Geometry> Pipe(GPUResourceManager& manager, float radius, float height, float heightSegments, uint32_t sides);
            // static Handle<Geometry> Pipe(Renderer& renderer, float radius, float height, float heightSegments, uint32_t sides);
            
            static Handle<Geometry> Capsule(GPUResourceManager& manager, float radius, float height, float heightSegments, uint32_t sides);

            static Handle<Geometry> Icosahedron(GPUResourceManager& manager);
            // static Handle<Geometry> Icosahedron(Renderer& renderer);

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
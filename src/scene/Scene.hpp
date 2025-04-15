#pragma once

#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <glm/ext/vector_float2.hpp>    // vec2
#include <glm/ext/vector_float3.hpp>    // vec3
#include <glm/ext/matrix_float4x4.hpp>  // mat4x4
#include <glm/ext/matrix_transform.hpp> //rotate, translate, scale, identity
//#include <glm/trigonometric.hpp>
#include <glm/gtx/euler_angles.hpp>
// #include <glm/ext/quaternion_common.hpp>


#include <vector>
#include "../types/Shader.hpp"
#include "../types/Material.hpp"
#include <optional>
#include <string>
#include "../types/Geometry.hpp"

namespace boitatah
{

    struct SceneNode;

    struct SceneNodeDesc
    {
        std::string name = "node";
        std::vector<SceneNode *> children;
        std::optional<SceneNode *> parentNode;
        Handle<Geometry> geometry = Handle<Geometry>();
        Handle<Material> material = Handle<Material>();
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    };

    struct SceneNode
    {
        private:
            bool m_dirtyMatrix = true;

        public:
            std::string name = "node";
            std::vector<SceneNode *> children;
            std::optional<SceneNode *> parentNode;
            Handle<Material> material;

            // Transform
            glm::mat4 m_localTransform;
            glm::mat4 m_globalTransform;
            std::vector<void*> customPushConstants;

            // Mesh
            Handle<Geometry> geometry;

            // Material

            // Constructor
            SceneNode(const SceneNodeDesc &desc) : material(desc.material),
                                            //children(desc.children),
                                            name(desc.name),
                                            //parentNode(desc.parentNode),
                                            geometry(desc.geometry)
            {
                m_localTransform = glm::mat4(1.0f);
                scale(desc.scale);
                rotate(desc.rotation);
                translate(desc.position);
            }

            void sceneAsList(std::vector<SceneNode*> &sceneList) const
            {
                sceneList.insert(sceneList.end(), children.begin(), children.end());

                for (const auto &child : children)
                {
                    child->sceneAsList(sceneList);
                }
            }

            void scale(const glm::vec3 &scales)
            {
                dirty();
                m_localTransform = glm::scale(m_localTransform, scales);
            }

            void rotate(const glm::vec3 &axis, float angle_radians)
            {
                dirty();
                m_localTransform = glm::rotate(m_localTransform,
                                            angle_radians, axis);
            }

            void rotate(const glm::vec3 &eulerAngles)
            {
                dirty();
                glm::mat4 rotationMatrix = glm::eulerAngleXYX(eulerAngles.x,
                                                            eulerAngles.y,
                                                            eulerAngles.z);

                m_localTransform = m_localTransform * rotationMatrix;
            }

            void translate(const glm::vec3 &translation)
            {
                dirty();
                m_localTransform = glm::translate(m_localTransform,
                                                translation);
            }

            void set_position(const glm::vec3 &position)
            {
                dirty();
                m_localTransform[3][0] = position.x;
                m_localTransform[3][1] = position.y;
                m_localTransform[3][2] = position.z;
            }

            void updateGlobalMatrix()
            {
                if (parentNode.has_value())
                {
                    m_globalTransform = parentNode.value()->getGlobalMatrix() *
                                        m_localTransform;
                }
                else
                {
                    m_globalTransform = m_localTransform;
                }
            }
            glm::mat4 getLocalMatrix() { return m_localTransform; }
            glm::mat4 getGlobalMatrix()
            {
                if (m_dirtyMatrix)
                {
                    updateGlobalMatrix();
                }
                return m_globalTransform;
            }

            void dirty(){
                if(m_dirtyMatrix) return;
                m_dirtyMatrix = true;
                for(auto& child : children){
                    child->dirty();
                }
            }

            void add(SceneNode* node){
                children.push_back(node);
                node->dirty();
                node->parentNode = this;
            }
    };
}


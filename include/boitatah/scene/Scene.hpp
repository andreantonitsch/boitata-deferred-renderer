#pragma once

#define GLM_FORCE_RADIANS
#include <glm/ext/vector_float3.hpp>    // vec3
#include <glm/ext/matrix_float4x4.hpp>  // mat4x4
#include <glm/ext/matrix_transform.hpp> //rotate, translate, scale, identity
#include <glm/gtx/euler_angles.hpp>

#include <vector>
#include <string>
#include <memory>

namespace boitatah
{

    template<typename T>
    struct SceneTree;

    template<typename T>  
    struct SceneNodeDesc
    {
        std::string name = "node";
        std::vector<std::shared_ptr<SceneTree<T>>> children;
        std::shared_ptr<SceneTree<T>> parentNode;
        T content;
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    };

    template<typename T>
    struct SceneTree : std::enable_shared_from_this<SceneTree<T>>
    {
        private:
        bool m_dirtyMatrix = true;
        protected:
            SceneTree(const SceneNodeDesc<T> &desc) : content(desc.content),
                                                   name(desc.name),
                                                   parentNode(desc.parentNode),
                                                   children(desc.children)
            {
                m_localTransform = glm::mat4(1.0f);
                scale(desc.scale);
                rotate(desc.rotation);
                translate(desc.position);
            }

        public:
            std::string name = "node";
            std::vector<std::shared_ptr<SceneTree<T>>> children;
            std::shared_ptr<SceneTree<T>> parentNode;

            // Transform
            glm::mat4 m_localTransform;
            glm::mat4 m_globalTransform;
            std::vector<void*> customPushConstants;
            T content;

            // Constructor
            static std::shared_ptr<SceneTree<T>> create_node(const SceneNodeDesc<T> &desc){
                return std::shared_ptr<SceneTree<T>>(new SceneTree<T>(desc));
            }

            void sceneAsList(std::vector<std::weak_ptr<SceneTree<T>>> &sceneList) const
            {
                sceneList.insert(sceneList.end(), children.begin(), children.end());

                for (const auto &child : children)
                {
                    auto child_ptr = std::shared_ptr<SceneTree<T>>(child);
                    child_ptr->sceneAsList(sceneList);
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
                if (parentNode != nullptr)
                {
                    m_globalTransform = parentNode->getGlobalMatrix() *
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

            void add(SceneTree<T>* node){
                children.push_back(std::shared_ptr<SceneTree<T>>(node));
                node->dirty();
                node->parentNode = this->shared_from_this();
            }

            void add(std::shared_ptr<SceneTree<T>> node){
                children.push_back(std::shared_ptr<SceneTree<T>>(node));
                node->dirty();
                node->parentNode = this->shared_from_this();
            }
    };
}


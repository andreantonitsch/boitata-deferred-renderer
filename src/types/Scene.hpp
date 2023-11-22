#ifndef BOITATAH_SCENE_HPP
#define BOITATAH_SCENE_HPP

#include <vector>
#include "Shader.hpp"
#include <optional>
#include <string>

namespace boitatah{

    struct SceneNode{
        std::string name = "node";
        std::vector<SceneNode> children;
        std::optional<SceneNode*> parentNode;
        Handle<Shader> shader;
        //Mat4x4 transform matrix
        //Mesh
        //Material
        glm::ivec2 vertexInfo;
        glm::ivec2 instanceInfo;

        void sceneAsList(std::vector<SceneNode> &sceneList) const{

            sceneList.insert(sceneList.end(), children.begin(), children.end());

            for(const auto& child : children){
                child.sceneAsList(sceneList);
            }
        }
    };
}

#endif //BOITATAH_SCENE_HPP
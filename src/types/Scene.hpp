#ifndef BOITATAH_SCENE_HPP
#define BOITATAH_SCENE_HPP

#include <vector>
#include "Shader.hpp"

namespace boitatah{

    struct SceneNode{
        std::vector<SceneNode> children;
        Handle<Shader> shader;
        //Mat4x4 transform matrix
        //Mesh
        //Material
        Vector2<int> vertexInfo;
        Vector2<int> instanceInfo;
    };
}

#endif //BOITATAH_SCENE_HPP
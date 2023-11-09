#ifndef BOITATAH_SCENE_HPP
#define BOITATAH_SCENE_HPP

#include <vector>

namespace boitatah{

    struct SceneNode{
        std::vector<SceneNode> children;
        //Mat4x4 transform matrix
        //Mesh
        //Material
    };
}

#endif //BOITATAH_SCENE_HPP
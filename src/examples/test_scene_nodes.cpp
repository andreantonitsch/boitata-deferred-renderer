#include "../types/Scene.hpp"
#include <vector>
#include <random>
#include <glm/vec4.hpp>
int main()
{
    using namespace boitatah;

    SceneNode grandchild({.name = "grandchild",
                          .position = glm::vec3(3, 5, 0),
                          .scale = glm::vec3(1.1,1,2)});
    SceneNode child({
        .name = "child",
        .position = glm::vec3(3, 0, 0),
    });
    child.add(&grandchild);
    SceneNode root({
        .name = "root",
    });
    root.add(&child);
    auto point  = (grandchild.getGlobalMatrix() * glm::vec4(1, 1, 1, 1));
            std::cout << point.x<< " " <<
            point.y<< " " <<
            point.z<< " " <<
        std::endl;
}
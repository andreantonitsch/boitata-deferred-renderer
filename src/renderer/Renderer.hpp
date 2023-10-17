#ifndef VELLY_RENDERER_HPP
#define VELLY_RENDERER_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace velly::dr
{
    class Renderer{
        public:
            //Renderer();
            ~Renderer(void);
            void render();
            void initVulkan();
            bool closed();
        private:
            // Base objects
                GLFWwindow* window;

            void initWindow();
            void cleanup();
    };
}
#endif //VELLY_RENDERER_HPP
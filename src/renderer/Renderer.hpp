#ifndef VELLY_RENDERER_HPP
#define VELLY_RENDERER_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "../types/Dimension.hpp"

namespace boitatah
{
    struct RendererOptions{
        Dimension2<uint32_t> windowDimensions = {800, 600};
    };


    class Renderer{
        public:
            Renderer(RendererOptions options);
            ~Renderer(void);
            void render();
            void initVulkan();
            bool isWindowClosed();
            bool funcao();
        private:
            // Base objects
                GLFWwindow* window;

            // Options
            int windowWidth;
            int windowHeight;


            void windowEvents();

            void initWindow();
            void cleanupWindow();
            void cleanup();
    };
}
#endif //VELLY_RENDERER_HPP
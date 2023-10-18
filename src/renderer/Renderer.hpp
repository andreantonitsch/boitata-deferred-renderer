#ifndef VELLY_RENDERER_HPP
#define VELLY_RENDERER_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "../types/Dimension.hpp"
#include "../vulkan/Vulkan.hpp"

namespace boitatah
{
    using namespace vulkan;

    struct RendererOptions{
        Dimension2<uint32_t> windowDimensions = {800, 600};
    };


    class Renderer{
        public:
            //Constructors / Destructors
            Renderer(RendererOptions options);
            ~Renderer(void);

            //Methods
            void render();
            void initVulkan();
            void initWindow();
            bool isWindowClosed();

        private:
            // Base objects
            Vulkan* vk; //Is this right?
            GLFWwindow* window;

            // Options Members
            int windowWidth;
            int windowHeight;

            // Window Functions
            const char** requiredWindowExtensions(uint32_t& extensionCount);
            void windowEvents();
            void cleanupWindow();

            // Vulkan Instance
            void createVkInstance();

            void cleanup();
    };
}
#endif //VELLY_RENDERER_HPP
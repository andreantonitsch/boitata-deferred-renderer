#ifndef BOITATAH_WINDOW_HPP
#define BOITATAH_WINDOW_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include <glm/vec2.hpp>

namespace boitatah::window{

    struct WindowDesc{
        glm::u32vec2 dimensions;
        const char* windowName;
    };

    class WindowManager {
        public:
            WindowManager(WindowDesc& desc);
            ~WindowManager(void);
            void initSurface(VkInstance instance);
            void destroySurface(VkInstance instance);
            VkSurfaceKHR getSurface();
            bool isWindowClosed();
            void windowEvents();
            glm::ivec2 getWindowDimensions();
            const std::vector<const char *> requiredWindowExtensions();
            GLFWwindow *window;

        private:
            VkSurfaceKHR surface;
            glm::ivec2 windowDimensions;
            static void framebufferResizeCallback(GLFWwindow* window, int width, int heigth);
    };

}

#endif //BOITATAH_WINDOW_HPP
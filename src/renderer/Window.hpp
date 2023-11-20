#ifndef BOITATAH_WINDOW_HPP
#define BOITATAH_WINDOW_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vector>
#include "../types/Vector.hpp"
#include <string>

namespace boitatah::window{

    struct WindowDesc{
        Vector2<uint32_t> dimensions;
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
            Vector2<int> getWindowDimensions();
            const std::vector<const char *> requiredWindowExtensions();
            GLFWwindow *window;

        private:
            VkSurfaceKHR surface;
            Vector2<int> windowDimensions;
            static void framebufferResizeCallback(GLFWwindow* window, int width, int heigth);
    };

}

#endif //BOITATAH_WINDOW_HPP
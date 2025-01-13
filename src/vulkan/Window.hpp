#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include "Vulkan.hpp"
#include "Window.hpp"
#include <memory>

#include <vector>
#include <string>

#include <glm/vec2.hpp>

namespace boitatah::vk{
    
    struct WindowDesc{
        glm::u32vec2 dimensions;
        const char* windowName;
    };

    class WindowManager {
        public:
            WindowManager(WindowDesc& desc);
            ~WindowManager(void);
            void initSurface(std::shared_ptr<boitatah::vk::Vulkan> vk);
            void destroySurface(VkInstance instance);
            VkSurfaceKHR getSurface();
            bool isWindowClosed();
            void windowEvents();
            glm::ivec2 getWindowDimensions();
            const std::vector<const char *> requiredWindowExtensions();
            GLFWwindow *window;

        private:
            std::shared_ptr<boitatah::vk::Vulkan> m_vk;
            VkSurfaceKHR surface;
            glm::ivec2 windowDimensions;
            static void framebufferResizeCallback(GLFWwindow* window, int width, int heigth);
    };

}

#include "Window.hpp"
#include <stdexcept>
#include "../vulkan/Vulkan.hpp"
#include <iostream>
namespace boitatah::window
{
    WindowManager::WindowManager(WindowDesc &desc)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(desc.dimensions.x,
                                  desc.dimensions.y,
                                  desc.windowName,
                                  nullptr,
                                  nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }
    WindowManager::~WindowManager(void)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    void WindowManager::initSurface(VkInstance instance)
    {

        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create Window Surface");
    
    }
    void WindowManager::destroySurface(VkInstance instance)
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }
    VkSurfaceKHR WindowManager::getSurface()
    {
        return surface;
    }
    bool WindowManager::isWindowClosed()
    {
        return glfwWindowShouldClose(window);
    }
    const std::vector<const char *> WindowManager::requiredWindowExtensions()
    {
        std::vector<const char *> requiredExtensions;
        uint32_t extensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        for (uint32_t i = 0; i < extensionCount; i++)
        {
            requiredExtensions.emplace_back(glfwExtensions[i]);
        }
        return requiredExtensions;
    }
    void WindowManager::framebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto windowManager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
        windowManager->windowDimensions = {width, height};
    }
    void WindowManager::windowEvents()
    {
        glfwPollEvents();
    }

    Vector2<int> WindowManager::getWindowDimensions()
    {
        return windowDimensions;
    }
}

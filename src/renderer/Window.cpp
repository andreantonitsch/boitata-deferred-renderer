#include "Window.hpp"

namespace boitatah::window
{
    WindowManager::WindowManager(WindowDesc &desc)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO temp
        window = glfwCreateWindow(desc.dimensions.x,
                                  desc.dimensions.y,
                                  desc.windowName,
                                  nullptr,
                                  nullptr);
    }
    WindowManager::~WindowManager(void)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
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
    void WindowManager::windowEvents()
    {
        glfwPollEvents();
    }

}

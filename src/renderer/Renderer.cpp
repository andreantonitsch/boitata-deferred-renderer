#pragma once

#include <iostream>
#include <stdexcept>

#include "Renderer.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace boitatah
{
    /// CONSTRUCTORS
    Renderer::Renderer(RendererOptions options)
    {
        windowWidth = options.windowDimensions.x;
        windowHeight = options.windowDimensions.y;
    }
    /// END CONSTRUCTORS

    void Renderer::render()
    {
        windowEvents();
    }

    void Renderer::initVulkan()
    {
        createVkInstance();
    }

    // VULKAN INSTANCE
    void Renderer::createVkInstance()
    {
        uint32_t extensionCount = 0;
        vk = new Vulkan({
            .appName = "Application",
            .extensions = requiredWindowExtensions(extensionCount),
            .extensionsCount = extensionCount
        });
    }

    // END OF VULKAN INSTANCE

    // Clean Up // Destructors
    void Renderer::cleanup()
    {
        delete vk;
        cleanupWindow();
    }

    Renderer::~Renderer(void)
    {
        cleanup();
    }

    /// WINDOWN FUNCTIONS

    void Renderer::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //TODO temp
        window = glfwCreateWindow(windowWidth,
                    windowHeight,
                    "Vulkan",
                    nullptr,
                    nullptr);
    }
    void Renderer::cleanupWindow()
    {        
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool Renderer::isWindowClosed()
    {
        return glfwWindowShouldClose(window);
    }

    const char **Renderer::requiredWindowExtensions(uint32_t &extensionCount)
    {
        return glfwGetRequiredInstanceExtensions(&extensionCount);
    }

    void Renderer::windowEvents()
    {
        glfwPollEvents();
    }

    /// END WINDOW FUNCTIONS

}

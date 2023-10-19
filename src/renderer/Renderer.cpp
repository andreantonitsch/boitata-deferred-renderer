#pragma once

#include <iostream>
#include <stdexcept>

#include "Renderer.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <stdexcept>

namespace boitatah
{
    /// CONSTRUCTORS
    Renderer::Renderer(RendererOptions opts)
    {
        options = opts;
    }
    /// END CONSTRUCTORS

    void Renderer::render()
    {
        windowEvents();
    }

    void Renderer::init()
    {
        initWindow();
        createVkInstance();
    }

    // VULKAN INSTANCE
    void Renderer::createVkInstance()
    {
        uint32_t extensionCount = 0;
        vk = new Vulkan({.appName = (char *)options.appName,
                         .extensions = requiredWindowExtensions(extensionCount),
                         .extensionsCount = extensionCount});
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
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO temp
        window = glfwCreateWindow(options.windowDimensions.x,
                                  options.windowDimensions.y,
                                  options.appName,
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

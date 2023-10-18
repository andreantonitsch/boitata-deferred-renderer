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
        initWindow();
    }

    void Renderer::cleanup()
    {
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

        void Renderer::windowEvents()
    {
        glfwPollEvents();
    }

    /// END WINDOW FUNCTIONS

}

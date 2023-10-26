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
        initWindow();
        createVulkan();
        buildSwapchain();
    }
    /// END CONSTRUCTORS

    void Renderer::render()
    {
        windowEvents();
    }

    void Renderer::createVulkan()
    {
        uint32_t extensionCount = 0;

        vk = new Vulkan({.appName = (char *)options.appName,
                         .extensions = requiredWindowExtensions(),
                         .useValidationLayers = options.debug,
                         .debugMessages = options.debug,
                         .window = window});
    }

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

    void Renderer::buildSwapchain()
    {
        vk->buildSwapchain(options.swapchainFormat);
    }

    Handle<Shader> Renderer::createShader(ShaderDesc data)
    {
        Shader shader{
            .name = data.name,
            .vert = vk->createShaderModule(data.vert.byteCode),
            .frag = vk->createShaderModule(data.frag.byteCode),
            };

        vk->buildShader(data, shader);

        return shaderPool.set(shader);
    }

    void Renderer::destroyShader(Handle<Shader> handle)
    {
        Shader shader;
        if(shaderPool.clear(handle, shader)){
            vk->destroyShaderModule(shader.vert);
            vk->destroyShaderModule(shader.frag);
        }
        else {
            std::cout << "Shader Double Destruction" << std::endl;
        }
    }

    const std::vector<const char *> Renderer::requiredWindowExtensions()
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

    void Renderer::windowEvents()
    {
        glfwPollEvents();
    }

    /// END WINDOW FUNCTIONS

}

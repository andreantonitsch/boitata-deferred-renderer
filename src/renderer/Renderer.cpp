#pragma once

#include <iostream>
#include <stdexcept>
#include "Renderer.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
const uint32_t WINDOW_WIDTH = 1024;
const uint32_t WINDOW_HEIGHT = 768;

namespace velly::dr
{
    void Renderer::render()
    {
        glfwPollEvents();
    }
    void Renderer::initWindow(){
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //TODO temp
        window = glfwCreateWindow(WINDOW_WIDTH,
                    WINDOW_HEIGHT,
                    "Vulkan",
                    nullptr,
                    nullptr);
    }
    bool Renderer::closed(){
        return glfwWindowShouldClose(window);
    }

    void Renderer::initVulkan()
    {
        initWindow();
    }

    void Renderer::cleanup()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    Renderer::~Renderer(void){
        cleanup();
    }
}

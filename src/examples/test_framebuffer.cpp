#include "../renderer/Renderer.hpp"
// #pragma once
#include <iostream>
// #include <unistd.h>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../utils/utils.hpp"
#include "../structures/Pool.hpp"

using namespace boitatah;

int main()
{

    Renderer r({.windowDimensions = {1024, 768},
                     .appName = "Test Frame Buffer",
                     .debug = true,
                     .swapchainFormat = boitatah::FORMAT::BGRA_8_SRGB});

    Handle<Shader> shader = r.createShader({
        .name = "test",
        .vert = {
            .byteCode = utils::readFile("./src/09_shader_base_vert.spv"),
            .entryFunction = "main"},
        .frag = {
            .byteCode = utils::readFile("./src/09_shader_base_frag.spv"),
            .entryFunction = "main"},
    });

    while (!r.isWindowClosed())
    {
        r.render();
    }

    r.destroyShader(shader);
    return EXIT_SUCCESS;
}
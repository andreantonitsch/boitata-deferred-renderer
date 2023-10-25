#include "../renderer/Renderer.hpp"
// #pragma once
#include <iostream>
// #include <unistd.h>
#include "../types/FORMAT.hpp"
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
            .byteCode = utils::readFile("./src/vert.spv"),
            .entryFunction = "main"},
        .frag = {
            .byteCode = utils::readFile("./src/frag.spv"),
            .entryFunction = "main"},
    });

    r.destroyShader(shader);


    while (!r.isWindowClosed())
    {
        r.render();
        //    r.present(target);
    }


    return EXIT_SUCCESS;
}
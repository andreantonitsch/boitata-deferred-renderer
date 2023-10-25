#include "../renderer/Renderer.hpp"
// #pragma once
#include <iostream>
// #include <unistd.h>
#include "../types/FORMAT.hpp"
#include "../types/RenderTarget.hpp"

namespace btt = boitatah;

int main()
{

    btt::Renderer r({.windowDimensions = {1024, 768},
                     .appName = "Test Frame Buffer",
                     .debug = true,
                     .swapchainFormat = boitatah::FORMAT::BGRA_8_SRGB});

    //RenderTarget target = r.createFrameBuffer();


    while (!r.isWindowClosed())
    {
    //    r.render(target);
    //    r.present(target);
    }

 
    return EXIT_SUCCESS;
}
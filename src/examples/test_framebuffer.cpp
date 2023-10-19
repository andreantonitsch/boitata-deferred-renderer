#include "../renderer/Renderer.hpp"
// #pragma once
#include <iostream>
// #include <unistd.h>

namespace btt = boitatah;

int main()
{

    btt::Renderer r({.windowDimensions = {1024, 768},
                     .appName = "Test Frame Buffer",
                     .debug = true});
    r.init();

    while (!r.isWindowClosed())
    {
        r.render();
    }

 
    return EXIT_SUCCESS;
}
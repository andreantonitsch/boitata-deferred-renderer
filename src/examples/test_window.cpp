#include "../renderer/Renderer.hpp"
//#pragma once
#include <iostream>

int main(){

    velly::dr::Renderer r;
    r.initVulkan();

    while(!r.closed()){
        r.render();
    }

    return EXIT_SUCCESS;
}
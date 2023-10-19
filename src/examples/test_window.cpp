#include "../renderer/Renderer.hpp"
//#pragma once
#include <iostream>

namespace btt = boitatah;

int main(){

    btt::Renderer r({
        .windowDimensions = {1024, 768}
        });
    r.init();

    while(!r.isWindowClosed()){
        r.render();
    }

    return EXIT_SUCCESS;
}
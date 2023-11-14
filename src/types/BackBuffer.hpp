#ifndef BOITATAH_BACKBUFFER_HPP
#define BOITATAH_BACKBUFFER_HPP

#include <vulkan/vulkan.h>
#include "RenderTarget.hpp"
#include <vector>

namespace boitatah{

    struct BackBufferManager{
        std::vector<Handle<RenderTarget>> buffers;
        int current;

        Handle<RenderTarget> getNext(){
            current = (current + 1) % buffers.size();
            return buffers[current];
        }

        Handle<RenderTarget> getCurrent(){
            return buffers[current];
        }

    };
}

#endif //BOITATAH_BACKBUFFER_HPP
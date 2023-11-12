#ifndef BOITATAH_BACKBUFFER_HPP
#define BOITATAH_BACKBUFFER_HPP

#include <vulkan/vulkan.h>
#include "RenderTarget.hpp"
#include <vector>

namespace boitatah{

    struct BackBufferManager{
        std::vector<RenderTarget> buffers;
    };

}

#endif //BOITATAH_BACKBUFFER_HPP
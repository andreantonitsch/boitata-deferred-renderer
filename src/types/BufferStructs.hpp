#ifndef BOITATAH_BUFFER_STRUCTS_HPP
#define BOITATAH_BUFFER_STRUCTS_HPP

#include <vulkan/vulkan.h>
#include "BttEnums.hpp"
#include "../collections/BufferAllocator.hpp"
#include "Buffer.hpp"
#include "BufferVkStructs.hpp"
namespace boitatah{

    class Buffer;


    struct BufferReservation
    {
        Buffer* buffer;
        uint32_t size;
        uint32_t offset;
        Handle<Block> reservedBlock;
    };



}
#endif //BOITATAH_BUFFER_STRUCTS_HPP
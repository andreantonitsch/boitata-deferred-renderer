#ifndef BOITATAH_BUFFER_STRUCTS_HPP
#define BOITATAH_BUFFER_STRUCTS_HPP

#include "../collections/Pool.hpp"
#include "../types/BttEnums.hpp"
#include "BufferAllocator.hpp"

namespace boitatah::buffer
{

    struct BufferReservationRequest
    {
        uint32_t request;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };

    struct BufferReservation
    {
        //Buffer* buffer;
        uint32_t size;
        uint32_t requestSize;
        uint32_t offset;
        Handle<Block> reservedBlock;
    };

    struct BufferDesc
    {
        // uint32_t alignment;
        uint32_t estimatedElementSize;
        // power of 2.
        uint32_t partitions;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };

    struct BufferUploadDesc{
        uint32_t dataSize;
        const void* data;
        BUFFER_USAGE usage;
    };

    class Buffer;
    struct BufferAddress{
        Handle<Buffer *> buffer;
        Handle<BufferReservation> reservation;
        uint32_t size;
    };

    struct BufferAddressDesc
    {
        uint32_t request;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };
}

#endif // BOITATAH_BUFFER_STRUCTS_HPP
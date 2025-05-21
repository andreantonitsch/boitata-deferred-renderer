#pragma once

#include <memory>
#include <boitatah/collections/Pool.hpp>
#include <boitatah/BoitatahEnums.hpp>

namespace boitatah::buffer
{
    class Buffer;
    class BufferManager;
    class Block;
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

        //if sharing == sharing_mode::exclusive, requires a buffermanager
        std::weak_ptr<BufferManager> bufferManager;
        //std::shared_ptr<Buffer> stagingBuffer;
    };
    
    struct BufferAddress{
        Handle<Buffer *> buffer;
        Handle<BufferReservation> reservation;
        uint32_t size;
    };

    struct BufferUploadDesc{
        Handle<BufferAddress> address;
        uint32_t dataSize;
        void* data;
    };

    struct BufferAddressDesc
    {
        uint32_t request;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };

    struct BufferAccessData{
        Buffer* buffer;
        uint32_t offset;
        uint32_t size;
    };
}

#ifndef BOITATAH_BUFFER_HPP
#define BOITATAH_BUFFER_HPP

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>

#include "../vulkan/Vulkan.hpp"

#include "../collections/Pool.hpp"
#include "BufferStructs.hpp"
#include "BufferAllocator.hpp"

#include "../types/BttEnums.hpp"

#include "BufferManager.hpp"

namespace boitatah::buffer
{
   class BufferManager;

   struct QueuedTransfer{

        Handle<BufferAddress> stagingBufferAddress;
        Handle<BufferAddress> finalBufferAddress;
   };

    class Buffer
    {
    public:
        Buffer(const BufferDesc &desc, const vk::Vulkan *vulkan);
        ~Buffer(void);

        uint32_t getID() const;
        VkBuffer getBuffer() const;
        VkDeviceMemory getMemory() const;
        bool getReservationData(const Handle<BufferReservation> handle, BufferReservation& reservation) const;

        Handle<BufferReservation> reserve(const uint32_t request);
        bool unreserve(const Handle<BufferReservation> reservation);

        // for SHARINGMODE::EXCLUSIVE requires a buffer queueUpdate after
        void copyData(const Handle<BufferReservation> handle, void * data);
        void queueUpdates();
        void clearTransferQueue();

        bool checkCompatibility(const BufferReservationRequest &compatibility);

    private:
        const vk::Vulkan *vulkan;

        BufferDesc description;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
        uint32_t alignment;
        uint32_t actualSize;
        uint32_t blockSize;

        // sharing mode concurrent data
        void *mappedMemory;
        
        vk::BufferVkData bufferData;

        std::unique_ptr<BufferAllocator> mainAllocator;
        std::unique_ptr<Pool<BufferReservation>> mainReservPool;

        //Staged Buffer Data
        std::weak_ptr<BufferManager> bufferManager;
        std::vector<QueuedTransfer> queuedTransfers;


        // bookkeeping parameters
        uint32_t buffer_id;
        inline static uint32_t buffer_quantity = 0;
        

        //initialization method
        void setupBuffer(uint32_t desiredBlockSize, uint32_t partitions);

    };

}

#endif // BOITATAH_BUFFER_HPP
#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>

#include <vulkan/Vulkan.hpp>

#include "../collections/Pool.hpp"
#include <vulkan/VulkanStructs.hpp>

#include "BufferStructs.hpp"
#include "BufferAllocator.hpp"

#include "../types/BttEnums.hpp"
#include "../command_buffers/CommandBufferWriter.hpp"
#include "BufferManager.hpp"


namespace boitatah::buffer
{
    using namespace boitatah::command_buffers;

   class BufferManager;

   struct QueuedTransfer{
        const Handle<BufferAddress> stagingBufferAddress; //external address to concurrent buffer
        const Handle<BufferReservation> finalBufferReservation; //local address to exclusive buffer
        
        QueuedTransfer(Handle<BufferAddress> stagingBufferAddress,
                    Handle<BufferReservation> finalBufferReservation)
                    : stagingBufferAddress(stagingBufferAddress),
                    finalBufferReservation(finalBufferReservation)
        {}
   };

    class Buffer
    {

        friend class BufferManager;
        public:
            Buffer(const BufferDesc &desc, const vk::Vulkan *vulkan);
            ~Buffer(void);

            VkBuffer getBuffer() const;
            VkDeviceMemory getMemory() const;
            uint32_t getID() const;
            
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

            //void queueTransfers();
            //void clearTransferQueue();

            bool checkCompatibility(const BufferReservationRequest &compatibility);

            // for SHARINGMODE::EXCLUSIVE requires a buffer queueUpdate after
            void copyData(const Handle<BufferReservation> handle, const void * data);
            void copyData(const Handle<BufferReservation> handle, const void * data, uint32_t size);
            //void copyDataFromBuffer(const Handle<BufferReservation> dst, const Handle<BufferAddress> srcBuffer);
            // returns a buffer address to the staging buffer

            bool getReservationData(const Handle<BufferReservation> handle, BufferReservation& reservation) const;
           
            Handle<BufferReservation> reserve(const uint32_t request);
            bool unreserve(const Handle<BufferReservation> reservation);

            //void queueTransfer(Handle<BufferAddress> src, Handle<BufferReservation> dst);

            //template<class T>
            //void queueTransfer(Handle<BufferAddress> src, Handle<BufferReservation> dst, CommandBufferWriter<T> &writer);

            bool hasUpdates();
    };



}

#ifndef BOITATAH_BUFFER_MANAGER_HPP
#define BOITATAH_BUFFER_MANAGER_HPP

#include <vector>

#include "../vulkan/Vulkan.hpp"
#include "BufferStructs.hpp"
#include "Buffer.hpp"

#include "../collections/Pool.hpp"

namespace boitatah::buffer
{
    class Buffer;
    using namespace boitatah::vk;
    
    class BufferManager
    {
        private:
            uint32_t partitionsPerBuffer = 1u << 10;

            Vulkan* m_vk;
            std::vector<Handle<Buffer *>> activeBuffers;
             std::vector<Handle<Buffer *>> activeStagingBuffers;

            Pool<Buffer *> bufferPool = Pool<Buffer *>({.size = 1<<16, .name = "uniforms pool"});
            Pool<std::shared_ptr<Buffer>> stagingBufferPool = Pool<std::shared_ptr<Buffer>>({.size = 1<<16, .name = "uniforms pool"});
            
            Pool<BufferAddress> addressPool = Pool<BufferAddress>({.size = 1<<20, .name = "uniforms pool"});
            CommandBuffer transferBuffer;
            VkFence m_transferFence;

            Handle<Buffer*> createBuffer(const BufferDesc &&description);
            std::shared_ptr<Buffer> createStagingBuffer(const BufferDesc &&description);
            
            void releaseBuffer(Handle<Buffer*> handle);

            Handle<Buffer *> findOrCreateCompatibleBuffer(const BufferReservationRequest &compatibility);
            uint32_t findCompatibleBuffer(const BufferReservationRequest &compatibility);
            
            std::shared_ptr<Buffer> findOrCreateCompatibleStagingBuffer(const BufferReservationRequest &compatibility);
            uint32_t findCompatibleStagingBuffer(const BufferReservationRequest &compatibility);
            

        public:
            BufferManager(Vulkan* vk_instance);
            ~BufferManager(void);
            Handle<BufferAddress> reserveBuffer(const BufferReservationRequest &request);
            bool uploadToBuffer(const BufferUploadDesc &desc);
            
            void queueUpdates(); //queues updates
            void startBufferUpdates(); //setup queue buffer updates
            void endBufferUpdates();    // ship queue buffer updates


            bool getAddressReservation(const Handle<BufferAddress> handle, BufferReservation& reservation);
            bool getAddressBuffer(const Handle<BufferAddress> handle, Buffer*& buffer);

            void freeBufferReservation(Handle<BufferAddress> handle);
            
            bool areTransfersFinished() const;
            void waitForTransferToFinish() const;
            CommandBuffer getTransferBuffer();
    };
};

#endif //BOITATAH_BUFFER_MANAGER_HPP
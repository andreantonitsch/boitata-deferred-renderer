#ifndef BOITATAH_BUFFER_MANAGER_HPP
#define BOITATAH_BUFFER_MANAGER_HPP

#include <vector>

#include "../vulkan/Vulkan.hpp"
#include "BufferStructs.hpp"
#include "Buffer.hpp"

#include "../collections/Pool.hpp"

namespace boitatah::buffer
{
    using namespace boitatah::vk;
    class BufferManager
    {
        private:
            Vulkan* m_vk;
            std::vector<Handle<Buffer *>> activeBuffers;
            Pool<Buffer *> bufferPool = Pool<Buffer *>({.size = 1<<16, .name = "uniforms pool"});
            Pool<BufferAddress> addressPool = Pool<BufferAddress>({.size = 1<<16, .name = "uniforms pool"});
            CommandBuffer transferBuffer;
            VkFence m_trasnferFence;

            Handle<Buffer*> createBuffer(const BufferDesc &&description);
            void releaseBuffer(Handle<Buffer*> handle);

            Handle<Buffer *> findOrCreateCompatibleBuffer(const BufferReservationRequest &compatibility);
            uint32_t findCompatibleBuffer(const BufferReservationRequest &compatibility);
        
        public:
            BufferManager(Vulkan* vk_instance);
            ~BufferManager();
            Handle<BufferAddress> reserveBuffer(const BufferReservationRequest &request);
            Handle<BufferAddress> uploadBuffer(const BufferUploadDesc &desc);
            
            void queueUpdateReservation(Handle<BufferAddress> reservation, void* new_data); //queues updates
            void startBufferUpdates(); //setup queue buffer updates
            void endBufferUpdates();    // ship queue buffer updates


            const BufferReservation& getAddressReservation(const Handle<BufferAddress> handle) const;
            Buffer* getAddressBuffer(const Handle<BufferAddress> handle);

            void freeBufferReservation(Handle<BufferAddress> handle);
            
            bool areTransfersFinished() const;
            void waitForTransferToFinish() const;

    };
};

#endif //BOITATAH_BUFFER_MANAGER_HPP
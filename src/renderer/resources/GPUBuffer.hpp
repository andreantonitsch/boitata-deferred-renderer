#ifndef BOITATAH_GPU_BUFFER_HPP
#define BOITATAH_GPU_BUFFER_HPP

#include <memory>

#include <glm/glm.hpp>
#include <vector>
#include "ResourceStructs.hpp"
#include "../../buffers/BufferStructs.hpp"
#include "../../buffers/Buffer.hpp"
#include "../../collections/Pool.hpp"
#include "GPUResource.hpp"
//#include "../modules/GPUResourceManager.hpp"
#include "../../vulkan/VkCommandBufferWriter.hpp"

#include <array>

namespace boitatah
{
    using namespace boitatah::command_buffers;
    class GPUBuffer;
    class GPUResourceManager;

    struct BufferGPUData;

    template<>
    struct ResourceTraits<GPUBuffer>{
        using ContentType = BufferGPUData;
    };

    struct BufferGPUData : public ResourceGPUContent<GPUBuffer>{
        Handle<BufferAddress> buffer;
        uint32_t buffer_capacity;
        bool dirty;
    };

    struct BufferMetaData : ResourceMetaContent<GPUBuffer>{
        void* buffer_ptr;
    };
 
    class GPUBuffer : public MutableGPUResource<GPUBuffer>
    {
        friend class MutableGPUResource<GPUBuffer>;
        //~GPUBuffer(void){};

        public :
            GPUBuffer() = default;
            ~GPUBuffer() = default;
            GPUBuffer(const GPUBuffer& other) = default;
 
            // Constructor
            GPUBuffer(const GPUBufferCreateDescription &createDescription, std::shared_ptr<GPUResourceManager> manager) 
                : MutableGPUResource<GPUBuffer>({ //Base Constructor
                                                .sharing = createDescription.sharing_mode,
                                                .type = RESOURCE_TYPE::GPU_BUFFER,
                                                .mutability = RESOURCE_MUTABILITY::MUTABLE,
                                                })
            {
                m_manager = manager;
                size = createDescription.size;
                usage = createDescription.usage;
            };

            void copyData(void * data, uint32_t frameIndex = 0);

            //TODO: Implement
            bool readData(void* dstPtr);

            //TODO: Implement
            bool asyncReadData(void* dstPtr);
            


        private:
            uint32_t size;
            BUFFER_USAGE usage;
            Handle<BufferAddress> stagingBuffer;
            BufferMetaData meta_data;

            /// @brief ready for use for buffers is trivially handled by MutableGPUResource<T>
            /// @param content 
            /// @return that this buffer is ready for use.
            bool ReadyForUse(ResourceGPUContent<GPUBuffer>& content);
            void SetContent(ResourceGPUContent<GPUBuffer>& content);

            // create Resource
            BufferGPUData CreateGPUData();
            void ReleaseData(BufferGPUData& data);
            void Release();

            void WriteTransfer(BufferGPUData& data, CommandBufferWriter<VkCommandBufferWriter> &writer);
    };
    

}

#endif
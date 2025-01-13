#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <vector>
#include "../../buffers/BufferStructs.hpp"
#include "../../buffers/Buffer.hpp"
#include "../../collections/Pool.hpp"
#include "ResourceStructs.hpp"
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
        using CommandBufferWriter = vk::VkCommandBufferWriter;
    };

    struct BufferGPUData //: public ResourceGPUContent<GPUBuffer>
    {
        Handle<BufferAddress> buffer;
        uint32_t buffer_capacity;
        bool dirty;
    };

    struct BufferMetaData : ResourceMetaContent<GPUBuffer>{
        void* buffer_ptr;
    };

    //TODO solve this issue in a more elegant manner
    class GPUBufferHelper{
        public:
            uint32_t size;
            BUFFER_USAGE usage;
            GPUBufferHelper() = default;
            GPUBufferHelper(const GPUBufferCreateDescription &createDescription)  :       size(createDescription.size),
                    usage(createDescription.usage){};
    };
 
    class GPUBuffer : public GPUBufferHelper, public MutableGPUResource<GPUBuffer>
    {
        friend class MutableGPUResource<GPUBuffer>;
        //~GPUBuffer(void){};

        public :
            GPUBuffer() = default;
            ~GPUBuffer() = default;
            GPUBuffer(const GPUBuffer& other) = default;
 
            // Constructor
            GPUBuffer(const GPUBufferCreateDescription &createDescription, std::shared_ptr<GPUResourceManager> manager) :   
                    GPUBufferHelper(createDescription),
                    MutableGPUResource<GPUBuffer>({ //Base Constructor
                                                    .sharing = createDescription.sharing_mode,
                                                    .type = RESOURCE_TYPE::GPU_BUFFER,
                                                    .mutability = RESOURCE_MUTABILITY::MUTABLE,
                                                  }, manager) 
                                                  { };

            void copyData(void * data);

            //TODO: Implement
            bool readData(void* dstPtr);

            //TODO: Implement
            bool asyncReadData(void* dstPtr);
            
        private:

            Handle<BufferAddress> stagingBuffer;
            BufferMetaData meta_data;

            /// @brief ready for use for buffers is trivially handled by MutableGPUResource<T>
            /// @param content 
            /// @return that this buffer is ready for use.
            bool ReadyForUse(BufferGPUData& content);
            void SetContent(BufferGPUData& content);

            // create Resource
            BufferGPUData CreateGPUData();
            void ReleaseData(BufferGPUData& data);
            void Release();

            void WriteTransfer(BufferGPUData& data, CommandBufferWriter<vk::VkCommandBufferWriter> &writer);
    };
    

}


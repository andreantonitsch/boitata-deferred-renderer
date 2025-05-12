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

    struct BufferGPUData //: public ResourceGPUContent<GPUBuffer>
    {
        Handle<BufferAddress> buffer;
        uint32_t buffer_capacity;
    };

    template<>
    struct ResourceTraits<GPUBuffer>{
        using ContentType = BufferGPUData;
        using CommandBufferWriter = vk::VkCommandBufferWriter;
        using RenderData = buffer::BufferAccessData;
    };



    struct BufferMetaData{
        uint32_t stride;
        uint32_t count;
    };

    //TODO solve this issue in a more elegant manner
    class GPUBufferHelper{
        public:
            uint32_t size;
            BUFFER_USAGE usage;
            GPUBufferHelper() = default;
            GPUBufferHelper(const GPUBufferCreateDescription &createDescription)  :
                   size(createDescription.size),
                   usage(createDescription.usage){};
    };
 
    class GPUBuffer : public GPUBufferHelper, public GPUResource<GPUBuffer, 2>
    {
        friend class GPUResource<GPUBuffer, 2>;
        //~GPUBuffer(void){};

        public :
            GPUBuffer() = default;
            ~GPUBuffer() = default;
            GPUBuffer(const GPUBuffer& other) = default;
 
            // Constructor
            GPUBuffer(const GPUBufferCreateDescription &createDescription, std::shared_ptr<GPUResourceManager> manager) :   
                    GPUBufferHelper(createDescription),
                    GPUResource<GPUBuffer, 2>({ //Base Constructor
                                                    .sharing = createDescription.sharing_mode,
                                                    .type = RESOURCE_TYPE::GPU_BUFFER,
                                                    .mutability = RESOURCE_MUTABILITY::MUTABLE,
                                                  }, manager) 
                                                  { };

            void copyData(const void * data, uint32_t length);
            buffer::BufferAccessData GetRenderData(uint32_t frame_index);

            //TODO: Implement
            bool readData(void* dstPtr);

            //TODO: Implement
            bool asyncReadData(void* dstPtr);

            void setStride(uint32_t stride){meta_data.stride = stride;};
            void setCount(uint32_t count){meta_data.count = count;};
            void setStrideCount(uint32_t stride, uint32_t count){
                setStride(stride);
                setCount(count);
            }
            
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


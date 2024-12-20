#ifndef BOITATAH_VULKAN_COMMAND_BUFFER_WRITER_HPP
#define BOITATAH_VULKAN_COMMAND_BUFFER_WRITER_HPP

#include "../command_buffers/CommandBufferWriter.hpp"
#include <memory>
#include "../vulkan/Vulkan.hpp"

namespace boitatah::command_buffers{
    using namespace boitatah::vk;

    class VkCommandBufferWriter;

    struct VulkanWrappedCommandBuffer : WrappedCommandBuffer<VkCommandBufferWriter>
    {
        VkCommandBuffer buffer;
    };

    struct VulkanWriterBeginCommand : WriterBeginCommand<VkCommandBufferWriter>{};

    struct VulkanWriterResetCommand : WriterResetCommand<VkCommandBufferWriter> {};

    struct VulkanWriterCopyBufferCommand : WriterCopyBufferCommand<VkCommandBufferWriter>
    {
        uint32_t srcOffset;
        uint32_t dstOffset;
        uint32_t size;
        VkBuffer srcBuffer;
        VkBuffer dstBuffer;
        
    };

    class VkCommandBufferWriter : CommandBufferWriter<VkCommandBufferWriter>
    {
        friend class  CommandBufferWriter<VkCommandBufferWriter>;
        public:
            using  CommandBufferWriter<VkCommandBufferWriter>::begin;
            VkCommandBufferWriter(std::shared_ptr<Vulkan> vk_instance) : vk_instance(vk_instance){};

        private:
            std::shared_ptr<Vulkan> vk_instance;
            VkCommandBuffer& unwrapCommandBuffer(){
                return static_cast<VulkanWrappedCommandBuffer&>(bufferWrapper).buffer;
            };

            void __imp_begin(VulkanWriterBeginCommand &command){
                    auto buffer = unwrapCommandBuffer();

                    VkCommandBufferBeginInfo beginInfo{
                        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                        .pInheritanceInfo = nullptr};
                    if (vkBeginCommandBuffer(unwrapCommandBuffer(), 
                                             &beginInfo) != VK_SUCCESS)
                    {
                        throw std::runtime_error("failed to initialize buffer");
                    }
            };

            void __imp_reset(VulkanWriterResetCommand &command){
                vkResetCommandBuffer(unwrapCommandBuffer(), 0);
            };

            void __imp_copyBuffer(VulkanWriterCopyBufferCommand& command){
                VkBufferCopy copy{
                        .srcOffset = command.srcOffset,
                        .dstOffset = command.dstOffset,
                        .size = command.size};
                    vkCmdCopyBuffer(unwrapCommandBuffer(),
                                    command.srcBuffer,
                                    command.dstBuffer,
                                    1,
                                    &copy);
            };
    };



};

#endif
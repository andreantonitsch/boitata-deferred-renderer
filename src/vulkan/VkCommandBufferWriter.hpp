#ifndef BOITATAH_VULKAN_COMMAND_BUFFER_WRITER_HPP
#define BOITATAH_VULKAN_COMMAND_BUFFER_WRITER_HPP

#include "../vulkan/Vulkan.hpp"
#include <vulkan/vulkan.h>
#include "VkCommandBufferWriterStructs.hpp"
#include "../command_buffers/CommandBufferWriterStructs.hpp"
#include <memory>
#include "../command_buffers/CommandBufferWriter.hpp"




namespace boitatah::vk{
    using namespace boitatah::command_buffers;

    class VkCommandBufferWriter : CommandBufferWriter<VkCommandBufferWriter>
    {
        friend class  CommandBufferWriter<VkCommandBufferWriter>;
        public:
            using  CommandBufferWriter<VkCommandBufferWriter>::setCommandBuffer;
            using  CommandBufferWriter<VkCommandBufferWriter>::begin;
            using  CommandBufferWriter<VkCommandBufferWriter>::reset;
            using  CommandBufferWriter<VkCommandBufferWriter>::end;
            using  CommandBufferWriter<VkCommandBufferWriter>::submit;

            VkCommandBufferWriter(std::shared_ptr<Vulkan> vk_instance) : vk_instance(vk_instance){};

        private:


            std::weak_ptr<Vulkan> vk_instance;
            // CommandWriterTraits<VkCommandBufferWriter>::CommandBufferType& unwrapCommandBuffer(){
            //     return bufferWrapper.unwrap();
            // };

            void __imp_begin(const VulkanWriterBeginCommand &command, VkCommandBuffer buffer){
                    //auto buffer = wrappedBuffer;//unwrapCommandBuffer();

                    VkCommandBufferBeginInfo beginInfo{
                        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                        .pInheritanceInfo = nullptr};
                    if (vkBeginCommandBuffer(buffer, 
                                             &beginInfo) != VK_SUCCESS)
                    {
                        throw std::runtime_error("failed to initialize buffer");
                    }
            };

            void __imp_reset(const VulkanWriterResetCommand &command, VkCommandBuffer buffer){
                vkResetCommandBuffer(buffer, 0);
            };

            void __imp_copyBuffer(const VulkanWriterCopyBufferCommand& command, VkCommandBuffer buffer){
                    VkBufferCopy copy{
                        .srcOffset = command.srcOffset,
                        .dstOffset = command.dstOffset,
                        .size = command.size};
                    vkCmdCopyBuffer(buffer,
                                    command.srcBuffer,
                                    command.dstBuffer,
                                    1,
                                    &copy);
            };

            void __imp_submit(const VulkanWriterSubmitCommand& command, VkCommandBuffer buffer){

            };
    };



};

#endif
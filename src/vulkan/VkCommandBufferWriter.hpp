#ifndef BOITATAH_VULKAN_COMMAND_BUFFER_WRITER_HPP
#define BOITATAH_VULKAN_COMMAND_BUFFER_WRITER_HPP

#include <memory>
#include <vulkan/vulkan.h>

#include <vulkan/Vulkan.hpp>

#include "VkCommandBufferWriterStructs.hpp"
#include "../command_buffers/CommandBufferWriterStructs.hpp"
#include "../command_buffers/CommandBufferWriter.hpp"




namespace boitatah::vk{
    using namespace boitatah::command_buffers;

    class VkCommandBufferWriter : public CommandBufferWriter<VkCommandBufferWriter>
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
                
                auto vk = std::shared_ptr<Vulkan>(vk_instance);
                vkEndCommandBuffer(buffer);
                vkResetFences(vk->getDevice(), 1, &m_fence);

                VkSubmitInfo submit{
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &buffer,
                };

                if (m_signal != VK_NULL_HANDLE)
                {
                    submit.signalSemaphoreCount = 1;
                    submit.pSignalSemaphores = &m_signal;
                }

                std::vector<VkPipelineStageFlags> stages{};

                VkQueue queue;
                if (command.submitType == COMMAND_BUFFER_TYPE::TRANSFER)
                {
                    stages.push_back(VK_PIPELINE_STAGE_TRANSFER_BIT);
                    queue = vk->getTransferQueue();
                }

                if (m_signal != VK_NULL_HANDLE)
                {
                    submit.waitSemaphoreCount = 1;
                    submit.pWaitSemaphores = &m_signal;
                    submit.pWaitDstStageMask = stages.data();
                }

                vkQueueSubmit(queue, 1, &submit, m_fence);
            };

            bool __imp_checkTransfers(){
                auto vk = std::shared_ptr<Vulkan>(vk_instance);
                return vk->checkFenceStatus(m_fence);
            };

            void __imp_waitForTransfers(){
                auto vk = std::shared_ptr<Vulkan>(vk_instance);
                return vk->waitForFence(m_fence);
            };
    };



};

#endif
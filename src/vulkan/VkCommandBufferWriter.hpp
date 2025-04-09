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
            using  CommandBufferWriter<VkCommandBufferWriter>::transitionImage;
            using  CommandBufferWriter<VkCommandBufferWriter>::copyBuffer;
            using  CommandBufferWriter<VkCommandBufferWriter>::copyImage;
            using  CommandBufferWriter<VkCommandBufferWriter>::copyBufferToImage;
            using  CommandBufferWriter<VkCommandBufferWriter>::waitForTransfers;

            VkCommandBufferWriter(std::shared_ptr<Vulkan> vk_instance) : vk_instance(vk_instance), CommandBufferWriter<VkCommandBufferWriter>(){};

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

                std::vector<VkPipelineStageFlags> stages{};

                VkQueue queue;
                if (command.submitType == COMMAND_BUFFER_TYPE::TRANSFER)
                {
                    stages.push_back(VK_PIPELINE_STAGE_TRANSFER_BIT);
                    queue = vk->getTransferQueue();
                    submit.pWaitDstStageMask = stages.data();
                }

                if(m_wait != VK_NULL_HANDLE){
                    submit.waitSemaphoreCount = 1;
                    submit.pWaitSemaphores = &(self().m_wait);
                }

                if (m_signal != VK_NULL_HANDLE && command.signal)
                {
                    //std::cout << "signaling " << &m_signal << std::endl;
                    submit.signalSemaphoreCount = 1;
                    submit.pSignalSemaphores = &(self().m_signal);
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

            // TODO make the pipeline stage customizable?
            void __imp_transitionImage(const VulkanWriterTransitionLayoutCommand &command, VkCommandBuffer buffer ){

                auto vk = std::shared_ptr<Vulkan>(vk_instance);
                auto vk_command = TransitionLayoutCmdVk{
                    .buffer = buffer,
                    .src = command.src,
                    .dst = command.dst,
                    .image = command.image,
                    .srcStage = command.srcStage,
                    .dstStage = command.dstStage,
                };

                switch(command.srcStage){
                    case  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT:
                        vk_command.srcAccess = 0;
                        break;
                    case VK_PIPELINE_STAGE_TRANSFER_BIT:
                        vk_command.srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
                        break;
                    default: vk_command.srcAccess = 0; break;
                }

                switch(command.dstStage){
                    case VK_PIPELINE_STAGE_TRANSFER_BIT:
                        vk_command.dstAccess = VK_ACCESS_TRANSFER_READ_BIT;
                        break;
                    case VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT:
                        vk_command.dstAccess = 0;
                        break;
                    default: vk_command.dstAccess = 0; break;
                }

                vk->CmdTransitionLayout(vk_command);

            }

            void __imp_copyImage(const VulkanWriterCopyImageCommand &command, VkCommandBuffer buffer){

                auto vk = std::shared_ptr<Vulkan>(vk_instance);
                vk->CmdCopyImage({
                    .buffer = buffer,
                    // VkQueue queue;
                    .srcImage = command.srcImage,
                    .srcImgLayout = command.srcLayout,
                    .dstImage = command.dstImage,
                    .dstImgLayout = command.dstLayout,
                    .extent = command.extent
                });

            }

            void __imp_copyBufferToImage(const VulkanWriterCopyBufferToImageCommand& command, VkCommandBuffer commandBuffer){
                
                __imp_transitionImage({
                    .src = command.srcImgLayout,
                    .dst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .image = command.image,
                    .srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    .dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
                }, commandBuffer);
                
                VkBufferImageCopy copy{};
                copy.bufferOffset = command.buffOffset;
                copy.bufferRowLength = 0;       //In case of padded images
                copy.bufferImageHeight  = 0;    //in case of padded images
                
                copy.imageSubresource.aspectMask = command.aspect;
                copy.imageSubresource.baseArrayLayer = 0;
                copy.imageSubresource.layerCount = 1;
                copy.imageSubresource.mipLevel = 0;
                
                copy.imageOffset = {command.offset.x, command.offset.y, command.offset.z};
                copy.imageExtent = {command.extent.x, 
                                    command.extent.y, 
                                    command.extent.z};

                vkCmdCopyBufferToImage(
                    commandBuffer,
                    command.buffer,
                    command.image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &copy
                );

                __imp_transitionImage({
                    .src = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .dst = command.dstImgLayout,
                    .image = command.image,                    
                    .srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
                    .dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                }, commandBuffer);

            }
    };



};

#endif
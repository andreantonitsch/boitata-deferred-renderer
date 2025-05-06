#pragma once

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
            // using  CommandBufferWriter<VkCommandBufferWriter>::set_commandbuffer;
            // using  CommandBufferWriter<VkCommandBufferWriter>::begin;
            // using  CommandBufferWriter<VkCommandBufferWriter>::reset;
            // using  CommandBufferWriter<VkCommandBufferWriter>::end;
            // using  CommandBufferWriter<VkCommandBufferWriter>::submit;
            // using  CommandBufferWriter<VkCommandBufferWriter>::begin_renderpass;
            // using  CommandBufferWriter<VkCommandBufferWriter>::transitionImage;
            // using  CommandBufferWriter<VkCommandBufferWriter>::copyBuffer;
            // using  CommandBufferWriter<VkCommandBufferWriter>::copyImage;
            // using  CommandBufferWriter<VkCommandBufferWriter>::copyBufferToImage;
            // using  CommandBufferWriter<VkCommandBufferWriter>::waitForTransfers;

            VkCommandBufferWriter(std::shared_ptr<Vulkan> vk_instance)
                :   vk_instance(vk_instance),
                    CommandBufferWriter<VkCommandBufferWriter>(){
                        m_signal = VK_NULL_HANDLE;
                        m_buffer = VK_NULL_HANDLE;
                        m_fence  = VK_NULL_HANDLE;
                    };

        private:
            std::weak_ptr<Vulkan> vk_instance;
            // CommandWriterTraits<VkCommandBufferWriter>::CommandBufferType& unwrapCommandBuffer(){
            //     return bufferWrapper.unwrap();
            // };

            void __imp_begin(const VulkanWriterBegin &command,
                                   VkCommandBuffer buffer) {
                    //auto buffer = wrappedBuffer;//unwrapCommandBuffer();

                    VkCommandBufferBeginInfo beginInfo{
                        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                        .pInheritanceInfo = nullptr};
                    if (vkBeginCommandBuffer(buffer,&beginInfo) != VK_SUCCESS)
                    {
                        throw std::runtime_error("failed to initialize buffer");
                    }
            };

            void __imp_reset(const VulkanWriterReset &command,
                                   VkCommandBuffer buffer) {
                vkResetCommandBuffer(buffer, 0);
            };

            void __imp_copy_buffer(const VulkanWriterCopyBuffer& command,
                                         VkCommandBuffer buffer) {
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

            void __imp_submit(const VulkanWriterSubmit& command,
                                    VkCommandBuffer buffer) {
                
                auto vk = std::shared_ptr<Vulkan>(vk_instance);
                vkEndCommandBuffer(buffer);
                
                // if(m_fence != VK_NULL_HANDLE)
                //     vkResetFences(vk->getDevice(), 1, &m_fence);
                    
                VkSubmitInfo submit{
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &buffer,
                };

                std::vector<VkPipelineStageFlags> stages{};

                VkQueue queue;
                if (command.submitType == COMMAND_BUFFER_TYPE::TRANSFER)
                {
                    queue = vk->getTransferQueue();
                    
                    stages.push_back(VK_PIPELINE_STAGE_TRANSFER_BIT);
                    for(int i = 0; i < m_wait.size(); i++)
                        stages.push_back(VK_PIPELINE_STAGE_TRANSFER_BIT);
                    submit.pWaitDstStageMask = stages.data();
                }
                
                if (command.submitType == COMMAND_BUFFER_TYPE::GRAPHICS)
                {
                    queue = vk->getGraphicsQueue();
                    
                    for(int i = 0; i < m_wait.size(); i++)
                        stages.push_back(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
                    submit.pWaitDstStageMask = stages.data();
                }

                if(m_wait.size() != 0){
                    submit.waitSemaphoreCount = static_cast<uint32_t>(m_wait.size());
                    submit.pWaitSemaphores = self().m_wait.data();
                }

                if (m_signal != VK_NULL_HANDLE && command.signal)
                {
                    submit.signalSemaphoreCount = 1;
                    submit.pSignalSemaphores = &(self().m_signal);
                }

                vkQueueSubmit(queue, 1, &submit, m_fence);
            };

            bool __imp_check_transfers(){
                auto vk = std::shared_ptr<Vulkan>(vk_instance);
                return vk->checkFenceStatus(m_fence);
            };

            void __imp_wait_for_transfers(){
                if(m_fence == VK_NULL_HANDLE) return;

                auto vk = std::shared_ptr<Vulkan>(vk_instance);
                vk->waitForFence(m_fence);
                vk->reset_fence(m_fence);
            };
            
            void __imp_transition_image(const VulkanWriterTransitionLayout &command,
                                              VkCommandBuffer buffer ) {

                auto vk = std::shared_ptr<Vulkan>(vk_instance);

                //Set access for the chpsen stages.
                VkAccessFlags srcAccess;
                VkAccessFlags dstAccess;
                switch(command.srcStage){
                    case  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT:
                        srcAccess = 0;
                        break;
                    case VK_PIPELINE_STAGE_TRANSFER_BIT:
                        srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
                        break;
                    default: srcAccess = 0; break;
                }

                switch(command.dstStage){
                    case VK_PIPELINE_STAGE_TRANSFER_BIT:
                        dstAccess = VK_ACCESS_TRANSFER_READ_BIT;
                        break;
                    case VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT:
                        dstAccess = 0;
                        break;
                    default: dstAccess = 0; break;
                }

                //write the barrier.
                VkImageMemoryBarrier barrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .srcAccessMask = srcAccess,
                    .dstAccessMask = dstAccess,
                    .oldLayout = command.src,
                    .newLayout = command.dst,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = command.image,
                    .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
                };

                //correction if source is a depthstencil.
                if(command.src == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                //set the pipeline barrier with the transtion.
                vkCmdPipelineBarrier(
                    buffer,
                    command.srcStage, 
                    command.dstStage, 
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

            }

            void __imp_copy_image(const VulkanWriterCopyImage &command,
                                        VkCommandBuffer buffer) {

                auto vk = std::shared_ptr<Vulkan>(vk_instance);

                //copy data
                VkImageCopy copy{
                    .srcSubresource = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .mipLevel = 0,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
                    .srcOffset = {0, 0, 0},
                    .dstSubresource = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .mipLevel = 0,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
                    .dstOffset = {0, 0, 0},
                    .extent = {
                        static_cast<uint32_t>(command.extent.x),
                        static_cast<uint32_t>(command.extent.y),
                        static_cast<uint32_t>(1)},
                };

                // transition source image to
                // transfer source layout.
                __imp_transition_image({
                    .src = command.srcLayout,
                    .dst = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    .image = command.srcImage,
                    .srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    .dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
                }, buffer);

                // transition destination image to
                // transfer destination layout
                __imp_transition_image({
                    .src = VK_IMAGE_LAYOUT_UNDEFINED,
                    .dst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .image = command.dstImage,
                    .srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    .dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
                }, buffer);

                //copy
                vkCmdCopyImage(buffer,
                            command.srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            command.dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1, &copy);

                // transition destination image back
                // to its original layout
                __imp_transition_image({
                    .src = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .dst = command.dstLayout,
                    .image = command.dstImage,

                    .srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
                    .dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                }, buffer);

                // transition source image back
                //  to its original layout
                __imp_transition_image({
                    .src = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    .dst = command.srcLayout,
                    .image = command.srcImage,

                    .srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
                    .dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                }, buffer);

            };

            void __imp_copy_buffer_to_image(const VulkanWriterCopyBufferToImage& command,
                                                  VkCommandBuffer commandBuffer){
                
                __imp_transition_image({
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

                __imp_transition_image({
                    .src = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .dst = command.dstImgLayout,
                    .image = command.image,                    
                    .srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
                    .dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                }, commandBuffer);

        }

        void __imp_begin_renderpass(const VulkanWriterBeginRenderpass &command,
                                     VkCommandBuffer command_buffer){
            std::vector<VkClearValue> clear_colors;
            for(auto& clear_color : command.clearColors){
                VkClearValue cc;
                cc.color = {{clear_color.x,
                            clear_color.y,
                            clear_color.z,
                            clear_color.w}};
                clear_colors.push_back(cc);
            }

            VkRect2D scissor = {
                .offset = {command.scissorOffset.x, command.scissorOffset.y},
                .extent = {.width = static_cast<uint32_t>(command.scissorDims.x),
                        .height = static_cast<uint32_t>(command.scissorDims.y)},
            };
            VkViewport viewport{
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(scissor.extent.width),
                .height = static_cast<float>(scissor.extent.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            };
            vkCmdSetViewport(command_buffer, 0, 1, &viewport);
            vkCmdSetScissor(command_buffer, 0, 1, &scissor);

            VkRenderPassBeginInfo passInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = command.pass,
                .framebuffer = command.frame_buffer,
                .renderArea = scissor,
                .clearValueCount = static_cast<uint32_t>(clear_colors.size()),
                .pClearValues = clear_colors.data(),
            };
            vkCmdBeginRenderPass(command_buffer, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
        }

        void __imp_end_renderpass(const VulkanWriterEndRenderpass &command,
                                        VkCommandBuffer &command_buffer){
            vkCmdEndRenderPass(command_buffer);                       
        }

        void __imp_bind_vertexbuffer(const VulkanWriterBindVertexBuffer &command,
                                           VkCommandBuffer    &command_buffer){
            vkCmdBindVertexBuffers(command_buffer, 
                                0, 
                                static_cast<uint32_t>(command.buffers.size()), 
                                command.buffers.data(), 
                                command.offsets.data());
        };

        void __imp_bind_indexbuffer(const VulkanWriterBindIndexBuffer &command,
                                          VkCommandBuffer &command_buffer){
            vkCmdBindIndexBuffer(command_buffer,
                                command.buffers,
                                command.offsets,
                                VK_INDEX_TYPE_UINT32);
        }; 

        void __imp_bind_pipeline(const VulkanWriterBindPipeline &command,
                                       VkCommandBuffer &command_buffer){
            vkCmdBindPipeline(command_buffer,
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              command.pipeline);
        }; 

        void __imp_bind_set(const VulkanWriterBindSet &command,
                                  VkCommandBuffer &command_buffer){
            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    command.layout, command.set_index,
                                    1, &(command.set),
                                    0, nullptr);
        }

        void __imp_draw(const VulkanWriterDraw &command,
                              VkCommandBuffer &command_buffer){
            
            if(command.indexed){
                vkCmdDrawIndexed(command_buffer, command.indexCount,
                        command.instaceCount, 
                        command.firstVertex, 
                        
                        //for instanced drawing. not for suballocated buffers.
                        //command.vertexBufferOffset, hardcoded 0.
                        0 
                        ,command.firstInstance);
            }else{
                vkCmdDraw(command_buffer, command.vertexCount, command.instaceCount,
                        command.firstVertex, command.firstInstance);
            }
        }; 
    };
};


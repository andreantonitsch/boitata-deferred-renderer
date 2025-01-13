#include "Vulkan.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>
#include <memory>

#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../types/Image.hpp"
#include "Window.hpp"

namespace bvk = boitatah::vk;

using boitatah::COLOR_SPACE;
using boitatah::COMMAND_BUFFER_LEVEL;
using boitatah::FORMAT;
using boitatah::FRAME_BUFFERING;
using boitatah::Image;
using boitatah::IMAGE_LAYOUT;
using boitatah::IMAGE_USAGE;
using boitatah::MEMORY_PROPERTY;
using boitatah::SAMPLES;
using boitatah::Shader;
using boitatah::ShaderDesc;
using boitatah::ShaderLayout;
using boitatah::ShaderLayoutDesc;

#pragma region Validationsupportjank
/// Validation Support Jank.
VkResult createDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance,
                              "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
};

void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance,
                              "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, debugMessenger, pAllocator);
    }
}

/// DEBUG
// Vulkan callback function.
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    // if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    //     && messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
    // {
    std::cerr << "Validation Layer : " << pCallbackData->pMessage << std::endl;
    //}

    return VK_FALSE;
}
#pragma endregion Validationsupportjank
/// End Validation Support Jank

#pragma region Initialization
bvk::Vulkan::Vulkan(VulkanOptions opts)
{
    options = opts;

    if (options.useValidationLayers)
    {
        validationLayers.emplace_back("VK_LAYER_KHRONOS_validation");
        instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    for (const auto &ext : options.extensions)
    {
        instanceExtensions.emplace_back(ext);
    }

    deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    initInstance();

}




// void bvk::Vulkan::createCommandBufferWriter()
// {
//     m_commandBufferWriter =  std::make_shared<VkCommandBufferWriter>(shared_from_this());;
// }

std::shared_ptr<bvk::Vulkan> boitatah::vk::Vulkan::create(VulkanOptions opts)
{
    std::shared_ptr<Vulkan> new_vulkan = std::make_shared<Vulkan>(opts);
    //new_vulkan->createCommandBufferWriter();
    return new_vulkan;
}

bvk::Vulkan::~Vulkan(void)
{

    vkDestroyCommandPool(device, commandPools.graphicsPool, nullptr);
    vkDestroyCommandPool(device, commandPools.transferPool, nullptr);
    vkDestroyCommandPool(device, commandPools.presentPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (options.useValidationLayers)
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    // vkDestroySurfaceKHR(instance, surface, nullptr); //done
    vkDestroyInstance(instance, nullptr);
}

VkInstance boitatah::vk::Vulkan::getInstance() const
{
    return instance;
}

VkDevice boitatah::vk::Vulkan::getDevice() const
{
    return device;
}

VkPhysicalDevice boitatah::vk::Vulkan::getPhysicalDevice()
{
    return physicalDevice;
}

VkQueue boitatah::vk::Vulkan::getTransferQueue()
{
    return queues.transferQueue;
}

VkQueue boitatah::vk::Vulkan::getGraphicsQueue()
{
    return queues.graphicsQueue;
}

VkQueue boitatah::vk::Vulkan::getPresentQueue()
{
    return queues.presentQueue;
}

void boitatah::vk::Vulkan::attachWindow(std::shared_ptr<WindowManager> window)
{
    this->window = window;
}

void boitatah::vk::Vulkan::initInstance()
{
    // App Info
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = options.appName,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3};

    // Extension Availability check
    // TODO For some MacOS sdks we need to add VK_KHR_PORTABILITY_subset.
    std::vector<VkExtensionProperties> availableExtensions = retrieveInstanceAvailableExtensions();

    for (const auto &ext : availableExtensions)
    {
        std::cout << ext.extensionName << std::endl;
    }

    if (options.useValidationLayers)
        for (const auto &ext : instanceExtensions)
        {
            std::cout << "required :: " << ext << std::endl;
        }

    bool hasAllExtensions = checkRequiredExtensions(availableExtensions,
                                                    instanceExtensions);

    if (!hasAllExtensions)
        throw std::runtime_error("Vulkan does not have all required extensions available.");

    // Create Info
    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = (uint32_t)(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data(),
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    if (options.useValidationLayers)
    {
        if (checkValidationLayerSupport(validationLayers))
        {

            populateMessenger(debugCreateInfo);
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        }
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan Instance.");
}

void boitatah::vk::Vulkan::completeInit()
{
    initializeDebugMessenger();

    initPhysicalDevice();
    initLogicalDeviceNQueues();
    setQueues();
    createCommandPools();

    familyIndices = findQueueFamilies(physicalDevice);
}

#pragma endregion Initialization

#pragma region Synchronization
void boitatah::vk::Vulkan::waitForFrame(RenderTargetCmdBuffers &bufferData)
{
    VkResult result = vkWaitForFences(device, 1, &bufferData.inFlightFen, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS)
        std::cout << "wait for fence failed " << result << std::endl;
    vkResetFences(device, 1, &bufferData.inFlightFen);
}

void boitatah::vk::Vulkan::waitIdle()
{
    vkDeviceWaitIdle(device);
}


void boitatah::vk::Vulkan::waitForFence(const VkFence &fence) const
{
    std::cout << " waiting for fence " << std::endl;
    VkResult result = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS)
        std::cout << "wait for fence failed " << result << std::endl;
    std::cout << " waited for fence " << std::endl;
    vkResetFences(device, 1, &fence);
}

bool boitatah::vk::Vulkan::checkFenceStatus(VkFence fence)
{
    VkResult result = vkGetFenceStatus(device, fence);
    return result == VK_SUCCESS;
}

#pragma endregion Synchronization

#pragma region PSO Building

VkRenderPass boitatah::vk::Vulkan::createRenderPass(const RenderPassDesc &desc)
{
    std::vector<VkAttachmentDescription> colorAttachments;
    std::vector<VkAttachmentReference> colorAttachmentRefs;

    for (const auto &attDesc : desc.attachments)
    {
        colorAttachments.push_back(createAttachmentDescription(attDesc));

        colorAttachmentRefs.push_back({
            .attachment = attDesc.index,
            .layout = castEnum<VkImageLayout>(attDesc.layout),
        });
    }

    VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT};

    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size()),
        .pColorAttachments = colorAttachmentRefs.data(),
    };

    VkRenderPassCreateInfo renderPassCreate{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(colorAttachments.size()),
        .pAttachments = colorAttachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency};
    VkRenderPass pass;
    if (vkCreateRenderPass(device, &renderPassCreate, nullptr, &pass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Render Pass");
    }

    return pass;
}

Image boitatah::vk::Vulkan::createImage(const ImageDesc &desc)
{

    Image image;

    VkImageCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = castEnum<VkFormat>(desc.format),
        .extent = {
            .width = desc.dimensions.x,
            .height = desc.dimensions.y,
            .depth = 1,
        },
        .mipLevels = desc.mipLevels,
        .arrayLayers = 1,
        .samples = castEnum<VkSampleCountFlagBits>(desc.samples),
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = castEnum<VkImageUsageFlagBits>(desc.usage),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE, // TODO SHARING MODE change later.
        .initialLayout = castEnum<VkImageLayout>(desc.initialLayout),
    };

    VkImage vkImage;

    if (vkCreateImage(device, &createInfo, nullptr, &vkImage) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to Create Image");
    }

    image.dimensions = desc.dimensions;
    image.image = vkImage;

    VkMemoryRequirements reqs;
    vkGetImageMemoryRequirements(device, vkImage, &reqs);

    image.memory = allocateMemory({
        .size = reqs.size,
        .type = MEMORY_PROPERTY::DEVICE_LOCAL,
        .typeBits = reqs.memoryTypeBits,
    });

    bindImageMemory(image.memory, image.image);

    return image;
}

VkImageView boitatah::vk::Vulkan::createImageView(VkImage image, const ImageDesc &desc)
{

    VkImageView view;

    VkImageViewCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = castEnum<VkFormat>(desc.format),
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
    };

    if (vkCreateImageView(device, &createInfo, nullptr, &view) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Swapchain image view");
    }

    return view;
}

uint32_t boitatah::vk::Vulkan::findMemoryIndex(const MemoryDesc &props) const
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        auto flags = castEnum<VkMemoryPropertyFlagBits>(props.type);
        // If the memory has all required properties.
        if ((props.typeBits & (i << i)) &&
            ((memProperties.memoryTypes[i].propertyFlags &
              flags) == flags))
        {
            return i;
        }
    }
    throw std::runtime_error("Failed to find memory");
    return 0;
}

uint32_t boitatah::vk::Vulkan::getAlignmentForBuffer(const VkBuffer buffer) const
{
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, buffer, &memReqs);
    return memReqs.alignment;
}

VkCommandBuffer boitatah::vk::Vulkan::allocateCommandBuffer(const CommandBufferDesc &desc)
{
    VkCommandBufferAllocateInfo allocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = castEnum<VkCommandBufferLevel>(desc.level),
        .commandBufferCount = desc.count};

    switch (desc.type)
    {
    case COMMAND_BUFFER_TYPE::GRAPHICS:
        allocateInfo.commandPool = commandPools.graphicsPool;
        break;
    case COMMAND_BUFFER_TYPE::TRANSFER:
        allocateInfo.commandPool = commandPools.transferPool;
        break;
    case COMMAND_BUFFER_TYPE::PRESENT:
        allocateInfo.commandPool = commandPools.presentPool;
        break;
    }

    VkCommandBuffer buffer;
    if (vkAllocateCommandBuffers(device, &allocateInfo, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Command Buffer");
    }

    return buffer;
}

void boitatah::vk::Vulkan::beginCmdBuffer(const BeginCommandVk &command)
{
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr};
    if (vkBeginCommandBuffer(command.commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to initialize buffer");
    }
}

void boitatah::vk::Vulkan::beginRenderpassCommand(const BeginRenderpassCommandVk &command)
{
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

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
    vkCmdSetViewport(command.commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(command.commandBuffer, 0, 1, &scissor);

    VkRenderPassBeginInfo passInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = command.pass,
        .framebuffer = command.frameBuffer,
        .renderArea = scissor,
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };
    vkCmdBeginRenderPass(command.commandBuffer, &passInfo, VK_SUBPASS_CONTENTS_INLINE);

}

void boitatah::vk::Vulkan::endRenderpassCommand(const EndRenderpassCommandVk &command)
{
    vkCmdEndRenderPass(command.commandBuffer);
    // if (vkEndCommandBuffer(command.commandBuffer) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("Failed to record a draw command buffer");
    // }
}

void boitatah::vk::Vulkan::recordDrawCommand(const DrawCommandVk &command)
{
    vkCmdBindPipeline(command.drawBuffer,
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      command.pipeline);


    if (command.vertexBuffer != VK_NULL_HANDLE)
    {
        VkBuffer vertexBuffers[] = {command.vertexBuffer};
        VkDeviceSize offsets[] = {command.vertexBufferOffset};
        vkCmdBindVertexBuffers(command.drawBuffer, 0, 1, vertexBuffers, offsets);
    }

    if(command.indexBuffer != VK_NULL_HANDLE){
        vkCmdBindIndexBuffer(command.drawBuffer, command.indexBuffer,
        command.indexBufferOffset, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(command.drawBuffer, command.indexCount,
                command.instaceCount, command.firstVertex, 
                command.vertexBufferOffset, command.firstInstance);
    }else{

        vkCmdDraw(command.drawBuffer, command.vertexCount, command.instaceCount,
                command.firstVertex, command.firstInstance);
    }


}

void boitatah::vk::Vulkan::resetCmdBuffer(const VkCommandBuffer buffer)
{
    vkResetCommandBuffer(buffer, 0);
}

void boitatah::vk::Vulkan::submitDrawCmdBuffer(const SubmitDrawCommandVk &command)
{
    vkResetFences(device, 1, &(command.fence));
    vkEndCommandBuffer(command.commandBuffer);
    // TODO buffer available.
    // std::vector<VkSemaphore> semaphores{SemImageAvailable};
    std::vector<VkPipelineStageFlags> stageFlags{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    // std::vector<VkSemaphore> signals{SemRenderFinished};
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        //        .waitSemaphoreCount = 1,
        //        .pWaitSemaphores = semaphores.data(),
        .pWaitDstStageMask = stageFlags.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &command.commandBuffer,
        //.signalSemaphoreCount = 1,
        //.pSignalSemaphores = signals.data()
    };
    
    if (vkQueueSubmit(queues.graphicsQueue, 1, &submitInfo, command.fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit graphics queue");
    }
}


void boitatah::vk::Vulkan::submitCmdBuffer(const SubmitCommandVk &command)
{
    vkResetFences(device, 1, &(command.fence));
    vkEndCommandBuffer(command.commandBuffer);

    VkSubmitInfo submit{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command.commandBuffer,
    };

    if (command.signal != VK_NULL_HANDLE)
    {
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &command.signal;
    }

    std::vector<VkPipelineStageFlags> stages{};
    VkQueue queue;
    if (command.submitType == COMMAND_BUFFER_TYPE::TRANSFER)
    {
        stages.push_back(VK_PIPELINE_STAGE_TRANSFER_BIT);
        queue = queues.transferQueue;
    }

    if (command.wait != VK_NULL_HANDLE)
    {
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &command.wait;
        submit.pWaitDstStageMask = stages.data();
    }

    vkQueueSubmit(queue, 1, &submit, command.fence);
}

bool boitatah::vk::Vulkan::presentFrame(Image &image,
                                        Image &swapchainImage,
                                        VkSwapchainKHR &swapchain,
                                        uint32_t &scIndex,
                                        const PresentCommandVk &command)
{
    auto transferBuffer = command.commandBuffer;

    // // Finished Transfer
    beginCommands(transferBuffer);
    CmdCopyImage({
        .buffer = transferBuffer,
        .srcImage = image.image,
        .srcImgLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .dstImage = swapchainImage.image,
        .dstImgLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .extent = image.dimensions,
    });

    endCommands(transferBuffer,
                queues.transferQueue,
                // command.bufferData.schainAcqSem,
                command.waitSemaphore,
                // command.bufferData.transferSem,
                command.signalSemaphore,
                // command.bufferData.inFlightFen);
                command.fence);

    //  Present
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &command.signalSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &scIndex,
        .pResults = nullptr};

    VkResult result = vkQueuePresentKHR(queues.presentQueue, &presentInfo);

    // swapchain is too small or too large
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        return false;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to present image to swapchain");

    return true;
}

void boitatah::vk::Vulkan::CmdCopyImage(const CopyImageCommandVk &command)
{

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
        .extent = {command.extent.x, command.extent.y, 1},
    };

    // transition source image to
    // transfer source layout.
    CmdTransitionLayout({
        .buffer = command.buffer,
        .src = command.srcImgLayout,
        .dst = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .image = command.srcImage,
        .srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        .dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .srcAccess = 0,
        .dstAccess = VK_ACCESS_TRANSFER_READ_BIT,
    });

    // transition destination image to
    // transfer destination layout
    CmdTransitionLayout({
        .buffer = command.buffer,
        .src = VK_IMAGE_LAYOUT_UNDEFINED,
        .dst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .image = command.dstImage,
        .srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        .dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .srcAccess = 0,
        .dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT,
    });

    vkCmdCopyImage(command.buffer,
                   command.srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   command.dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &copy);

    // transition destination image back
    // to its original layout
    CmdTransitionLayout({
        .buffer = command.buffer,
        .src = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .dst = command.dstImgLayout,
        .image = command.dstImage,

        .srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccess = 0,
    });

    // transition source image back
    //  to its original layout
    CmdTransitionLayout({
        .buffer = command.buffer,
        .src = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .dst = command.srcImgLayout,
        .image = command.srcImage,

        .srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccess = 0,
    });
}

void boitatah::vk::Vulkan::CmdCopyBuffer(const CopyBufferCommandVk &command) const
{
    VkBufferCopy copy{
        .srcOffset = command.srcOffset,
        .dstOffset = command.dstOffset,
        .size = command.size};
    vkCmdCopyBuffer(command.commandBuffer,
                    command.srcBuffer,
                    command.dstBuffer,
                    1,
                    &copy);
}


void boitatah::vk::Vulkan::beginCommands(const VkCommandBuffer &buffer)
{
    vkResetCommandBuffer(buffer, 0);

    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(buffer, &beginInfo);
}

void boitatah::vk::Vulkan::endCommands(const VkCommandBuffer &buffer,
                                       const VkQueue &queue,
                                       const VkSemaphore &wait,
                                       const VkSemaphore &signal,
                                       const VkFence &fence)
{
    vkEndCommandBuffer(buffer);

    VkSubmitInfo submit{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &buffer,
    };

    if (signal != VK_NULL_HANDLE)
    {
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &signal;
    }

    std::vector<VkPipelineStageFlags> stages{VK_PIPELINE_STAGE_TRANSFER_BIT};
    if (wait != VK_NULL_HANDLE)
    {
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &wait;
        submit.pWaitDstStageMask = stages.data();
    }

    vkQueueSubmit(queue, 1, &submit, fence);
}

VkDeviceMemory boitatah::vk::Vulkan::allocateMemory(const MemoryDesc &desc)
{
    VkMemoryAllocateInfo allocateInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = desc.size,
        .memoryTypeIndex = findMemoryIndex(desc),
    };

    VkDeviceMemory memory;

    if (vkAllocateMemory(device, &allocateInfo, nullptr, &memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Memory");
    }

    return memory;
}

void *boitatah::vk::Vulkan::mapMemory(const MapMemoryVk &desc) const
{
    // auto memoryUnmapper = [&, this](){
    //     unmapMemory({.memory = desc.memory});
    // };
    void *mappedTarget;
    if(vkMapMemory(device, desc.memory, desc.offset, desc.size, 0, &mappedTarget) != VK_SUCCESS){
        return nullptr;
    }
    std::cout<< "Vulkan mapped memory" << std::endl;
    //std::unique_ptr<void> mapped_pointer = std::make_unique<void>(mappedTarget);
    return mappedTarget;
}

void boitatah::vk::Vulkan::unmapMemory(const UnmapMemoryVk &desc) const
{
    vkUnmapMemory(device, desc.memory);
}

void boitatah::vk::Vulkan::bindImageMemory(VkDeviceMemory memory, VkImage image)
{
    // binds from start.
    // currently using one buffer per image.
    vkBindImageMemory(device, image, memory, 0);
}

void boitatah::vk::Vulkan::mapDataAndCopyToBuffer(CopyToBufferVk op)
{
    //maps a pointer to mapped memory, offset from its start.
    // size is the memory range to be mapped.   
    void * mapMem = mapMemory({.memory = op.memory, .offset = op.offset, .size = op.size});

    std::byte* byte_source = static_cast<std::byte*>(op.data);
    std::copy(byte_source, byte_source + op.size, static_cast<std::byte*>(mapMem));
    unmapMemory({.memory = op.memory});
}

void boitatah::vk::Vulkan::copyToMappedMemory(const CopyMappedMemoryVk &op) const
{
    std::byte* start = static_cast<std::byte*>(op.data) + op.offset;
    std::byte* end = start + op.elementSize * op.elementCount;
    
    std::cout << "Vulkan copy data " << start << " " << end << op.map << std::endl;

    std::copy(
        start,
        end,
        static_cast<std::byte*>(op.map));
}

VkPipelineLayout boitatah::vk::Vulkan::createShaderLayout(const ShaderLayoutDescVk &desc)
{
    VkPipelineLayout layout;
    
    //  add camera layout
    // add shader custom layout
    
    //add model push constants

    VkPipelineLayoutCreateInfo layoutCreate{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    if (vkCreatePipelineLayout(device, &layoutCreate, nullptr, &layout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create Pipeline Layout");
    }

    return layout;
}

VkDescriptorSetLayout boitatah::vk::Vulkan::createDescriptorLayout(const DescriptorSetLayoutDesc &desc)
{
    VkDescriptorSetLayout layout;

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    uint32_t binding_index = 0;
    for(const auto& bindingDesc : desc.bindingDescriptors){
        VkDescriptorSetLayoutBinding binding{
            .binding = binding_index,
            .descriptorType = castEnum<VkDescriptorType>(bindingDesc.type),
            .descriptorCount = binding.descriptorCount,
            .stageFlags = castEnum<VkShaderStageFlags>(bindingDesc.stages)
        };
        binding_index++;
    }

    VkDescriptorSetLayoutCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    if(vkCreateDescriptorSetLayout(device, &info, nullptr, &layout) != VK_SUCCESS){
        throw std::runtime_error("Failed to create Descriptor Set Layout");
    }

    return layout;
}

VkFence boitatah::vk::Vulkan::createFence(bool signaled)
{
    VkFenceCreateInfo fenceInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    if (signaled)
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence;
    if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Fence");
    std::cout << "created fence" << std::endl;
    return fence;
}

VkSemaphore boitatah::vk::Vulkan::createSemaphore() const
{
    VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkSemaphore semaphore;

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore))
        throw std::runtime_error("Failed to create Semaphore");

    return semaphore;
}

boitatah::vk::BufferVkData boitatah::vk::Vulkan::createBuffer(const BufferDescVk &desc) const
{

    const std::vector<uint32_t> indexes = {familyIndices.graphicsFamily.value(),
                                           familyIndices.transferFamily.value()};

    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = desc.size,
        .usage = castEnum<VkBufferUsageFlags>(desc.usage),
        .sharingMode = castEnum<VkSharingMode>(desc.sharing),
    };

    if (desc.sharing == SHARING_MODE::CONCURRENT)
    {
        bufferInfo.queueFamilyIndexCount = 2;
        bufferInfo.pQueueFamilyIndices = indexes.data();
    }

    VkBuffer buffer;
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a buffer");
        ;
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, buffer, &memReqs);

    VkDeviceMemory memory;
    VkMemoryAllocateInfo memInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = findMemoryIndex({
            .size = memReqs.size,
            .type = MEMORY_PROPERTY::HOST_VISIBLE_COHERENT,
            .typeBits = memReqs.memoryTypeBits,
        })};

    if (vkAllocateMemory(device, &memInfo, nullptr, &memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to  allocate buffer memory");
    }

    vkBindBufferMemory(device, buffer, memory, 0);

    return {.buffer = buffer, .memory = memory, .alignment = memReqs.alignment, .actualSize =memReqs.size };
}

boitatah::vk::BufferVkData boitatah::vk::Vulkan::getBufferAlignmentMemoryType(const BufferDescVk &desc) const
{
    const std::vector<uint32_t> indexes = {familyIndices.graphicsFamily.value(),
                                           familyIndices.transferFamily.value()};

    VkBufferCreateInfo dummyCreate{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = desc.size,
        .usage = castEnum<VkBufferUsageFlags>(desc.usage),
        .sharingMode = castEnum<VkSharingMode>(desc.sharing),
    };

    if (desc.sharing == SHARING_MODE::CONCURRENT)
    {
        dummyCreate.queueFamilyIndexCount = 2;
        dummyCreate.pQueueFamilyIndices = indexes.data();
    }

    VkBuffer dummyBuffer;
    if (vkCreateBuffer(device, &dummyCreate, nullptr, &dummyBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a buffer");
        ;
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, dummyBuffer, &memReqs);

    vkDestroyBuffer(device, dummyBuffer, nullptr);

    return BufferVkData{
        .alignment = memReqs.alignment,
        .memoryTypeBits = memReqs.memoryTypeBits};
}

void boitatah::vk::Vulkan::buildShader(const ShaderDescVk &desc, Shader &shader)
{
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()};

    // TOOD add to shader description.

    uint32_t bindingCount = static_cast<uint32_t>(desc.bindings.size());
    uint32_t attributeCount = static_cast<uint32_t>(desc.attributes.size());

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = bindingCount,
        .pVertexBindingDescriptions = bindingCount != 0 ? desc.bindings.data() : nullptr,
        .vertexAttributeDescriptionCount = attributeCount,
        .pVertexAttributeDescriptions = attributeCount != 0 ? desc.attributes.data() : nullptr};

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    // Dynamic state
    VkPipelineViewportStateCreateInfo dynamicViewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1};

    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE, // if true rasterizes discards geometry.
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,

        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,

        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };

    VkPipelineShaderStageCreateInfo stages[] = {
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_VERTEX_BIT,
         .module = shader.vert.shaderModule,
         .pName = desc.vert.entryFunction.c_str()},
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
         .module = shader.frag.shaderModule,
         .pName = desc.frag.entryFunction.c_str()}};

    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = stages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &dynamicViewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = desc.layout,
        .renderPass = desc.renderpass,
        .subpass = 0,

        // for deriving pipelines.
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    if (vkCreateGraphicsPipelines(device,
                                  VK_NULL_HANDLE,
                                  1,
                                  &pipelineInfo,
                                  nullptr,
                                  &(shader.pipeline)) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline");
    }
}

VkShaderModule boitatah::vk::Vulkan::createShaderModule(const std::vector<char> &bytecode)
{
    VkShaderModuleCreateInfo shaderCreate{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = bytecode.size(),
        .pCode = reinterpret_cast<const uint32_t *>(bytecode.data()),
    };

    VkShaderModule module;
    if (vkCreateShaderModule(device, &shaderCreate, nullptr, &module) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Shader Module");
    }
    return module;
}

VkFramebuffer boitatah::vk::Vulkan::createFramebuffer(const FramebufferDescVk &desc)
{
    VkFramebuffer buffer;
    VkFramebufferCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = desc.pass,
        .attachmentCount = static_cast<uint32_t>(desc.views.size()),
        .pAttachments = desc.views.data(),
        .width = desc.dimensions.x,
        .height = desc.dimensions.y,
        .layers = 1};

    VkResult r = vkCreateFramebuffer(device, &createInfo, nullptr, &buffer);
    if (r != VK_SUCCESS)
    {
        std::cout << "\n#### " << r << "\n " << std::endl;
        throw std::runtime_error("failed to create framebuffer");
    }

    return buffer;
}

VkAttachmentDescription boitatah::vk::Vulkan::createAttachmentDescription(const AttachmentDesc &attDesc)
{
    return VkAttachmentDescription{.format = castEnum<VkFormat>(attDesc.format),
                                   .samples = castEnum<VkSampleCountFlagBits>(attDesc.samples),
                                   .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                   .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                   .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                   .initialLayout = castEnum<VkImageLayout>(attDesc.initialLayout),
                                   .finalLayout = castEnum<VkImageLayout>(attDesc.finalLayout)};
}
#pragma endregion PSO Building

#pragma region Object Destructions

void boitatah::vk::Vulkan::destroyShader(Shader &shader)
{
    vkDestroyShaderModule(device, shader.vert.shaderModule, nullptr);
    vkDestroyShaderModule(device, shader.frag.shaderModule, nullptr);
    vkDestroyPipeline(device, shader.pipeline, nullptr);
}

void boitatah::vk::Vulkan::destroyRenderpass(RenderPass &pass)
{
    vkDestroyRenderPass(device, pass.renderPass, nullptr);
}

void boitatah::vk::Vulkan::destroyFramebuffer(RenderTarget &framebuffer)
{
    vkDestroyFramebuffer(device, framebuffer.buffer, nullptr);
}

void boitatah::vk::Vulkan::destroyImage(Image image)
{
    vkDestroyImageView(device, image.view, nullptr);
    if (!image.swapchain)
    {
        vkDestroyImage(device, image.image, nullptr);
        vkFreeMemory(device, image.memory, nullptr);
    }
}

void boitatah::vk::Vulkan::destroyPipelineLayout(ShaderLayout &layout)
{
    vkDestroyPipelineLayout(device, layout.layout, nullptr);
}

void boitatah::vk::Vulkan::destroyRenderTargetCmdData(const RenderTargetCmdBuffers &sync)
{
    vkFreeCommandBuffers(device, commandPools.graphicsPool, 1, &(sync.drawBuffer.buffer));
    vkFreeCommandBuffers(device, commandPools.transferPool, 1, &(sync.transferBuffer.buffer));

    vkDestroyFence(device, sync.inFlightFen, nullptr);
    vkDestroySemaphore(device, sync.schainAcqSem, nullptr);
    vkDestroySemaphore(device, sync.transferSem, nullptr);
}

void boitatah::vk::Vulkan::destroyBuffer(BufferVkData buffer) const
{
    vkDestroyBuffer(device, buffer.buffer, nullptr);
    vkFreeMemory(device, buffer.memory, nullptr);
}

void boitatah::vk::Vulkan::destroyFence(VkFence fence)
{
    vkDestroyFence(device, fence, nullptr);
}

void boitatah::vk::Vulkan::destroyDescriptorPool(VkDescriptorPool pool)
{
    vkDestroyDescriptorPool(device, pool, nullptr);
}

void boitatah::vk::Vulkan::destroyDescriptorSetLayout(VkDescriptorSetLayout &layout)
{
    vkDestroyDescriptorSetLayout(device, layout, nullptr);
}

#pragma endregion Object Destructions

#pragma region QUEUE_SETUP

void boitatah::vk::Vulkan::initLogicalDeviceNQueues()
{
    QueueFamilyIndices familyIndices = findQueueFamilies(physicalDevice);

    float queuePriority = 1.0;
    VkDeviceQueueCreateInfo graphicsQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = familyIndices.graphicsFamily.value(),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority};

    VkDeviceQueueCreateInfo presentQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = familyIndices.presentFamily.value(),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority};

    VkPhysicalDeviceFeatures deviceFeatures{};

    std::vector<VkDeviceQueueCreateInfo> queueCreation{
        graphicsQueueCreateInfo, presentQueueCreateInfo};

    VkDeviceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 2,
        .pQueueCreateInfos = queueCreation.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };

    if (options.useValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to initialize a Logical Device");
    }
}

void boitatah::vk::Vulkan::setQueues()
{
    QueueFamilyIndices familyIndices = findQueueFamilies(physicalDevice);
    vkGetDeviceQueue(device, familyIndices.graphicsFamily.value(), 0, &queues.graphicsQueue);
    vkGetDeviceQueue(device, familyIndices.presentFamily.value(), 0, &queues.presentQueue);
    vkGetDeviceQueue(device, familyIndices.transferFamily.value(), 0, &queues.transferQueue);
}

void boitatah::vk::Vulkan::CmdTransitionLayout(const TransitionLayoutCmdVk &command)
{
    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = command.srcAccess,
        .dstAccessMask = command.dstAccess,
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

    vkCmdPipelineBarrier(
        command.buffer,
        command.srcStage /*TODO*/, command.dstStage, /*TODO*/
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

void boitatah::vk::Vulkan::createCommandPools()
{
    VkCommandPoolCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        // two types of pools. The other is VK_COMMNAD_POOL_CREATE_TRANSIENT_BIT
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = findQueueFamilies(physicalDevice).graphicsFamily.value(),
    };

    if (vkCreateCommandPool(device, &info, nullptr, &commandPools.graphicsPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Graphics Command Pool");
    }

    info.queueFamilyIndex = findQueueFamilies(physicalDevice).transferFamily.value();
    if (vkCreateCommandPool(device, &info, nullptr, &commandPools.transferPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Transfer Command Pool");
    }

    info.queueFamilyIndex = findQueueFamilies(physicalDevice).presentFamily.value();
    if (vkCreateCommandPool(device, &info, nullptr, &commandPools.presentPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Present Command Pool");
    }
}

#pragma endregion QUEUE_SETUP

#pragma region DEVICE_SETUP

void boitatah::vk::Vulkan::initPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        std::runtime_error("No available GPUs with Vulkan Support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto &device : devices)
    {
        int value = evaluatePhysicalDevice(device);
        candidates.insert(std::make_pair(value, device));
    }

    if (candidates.rbegin()->first > 0)
    {
        physicalDevice = candidates.rbegin()->second;

        // DEBUG MESSAGE
        if (options.debugMessages)
        {
            VkPhysicalDeviceProperties deviceProps;
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProps);
            std::cout << "###DEBUG Physical Device### \n"
                      << "Picked physcal device " << deviceProps.deviceName << "\n"
                      << "Device type " << deviceProps.deviceType << "\n"
                      << "Device ID " << deviceProps.deviceID << "\n"
                      << "###DEBUG Physical Device END###" << std::endl;
        }
    }
    if (physicalDevice == VK_NULL_HANDLE)
        std::runtime_error("No available GPUs with Vulkan Support");
}

boitatah::vk::QueueFamilyIndices boitatah::vk::Vulkan::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices queueFamilies;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> families(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, families.data());

    int i = 0;
    for (const auto &family : families)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, window->getSurface(), &presentSupport);

        if (family.queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            queueFamilies.transferFamily = i;
        }

        if (presentSupport)
        {
            queueFamilies.presentFamily = i;
        }

        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamilies.graphicsFamily = i;
        }
        i++;
    }
    return queueFamilies;
}

#pragma endregion DEVICE_SETUP

#pragma region EXTENSIONS
// TODO this can be better
//  Compares required extensions and available extensions
std::vector<VkExtensionProperties> bvk::Vulkan::retrieveInstanceAvailableExtensions()
{

    uint32_t extensionCount = 0;
    // call once to determine extension count
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    // create a vector and populate it with the extension properties.
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    return extensions;
}

boitatah::vk::SwapchainSupport boitatah::vk::Vulkan::getSwapchainSupport(VkPhysicalDevice physicalDevice)
{
    SwapchainSupport support;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, window->getSurface(), &support.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, window->getSurface(), &formatCount, nullptr);

    if (options.useValidationLayers)
        std::cout << " AVAILABLE SURFACE FORMATS " << std::endl;

    if (formatCount != 0)
    {
        support.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
                                             window->getSurface(),
                                             &formatCount,
                                             support.formats.data());

        for (const auto &format : support.formats)
        {
            std::cout << " \tFORMAT :: " << format.format << std::endl;
        }
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, window->getSurface(), &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        support.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
                                                  window->getSurface(),
                                                  &presentModeCount,
                                                  support.presentModes.data());
    }

    return support;
}

bool bvk::Vulkan::checkRequiredExtensions(const std::vector<VkExtensionProperties> &available,
                                          const std::vector<const char *> &required)
{
    for (const auto &ext : required)
    {
        bool found = false;
        for (const auto &property : available)
        {
            if (std::strcmp(ext, property.extensionName) == 0)
            {
                found = true;
            }
        }
        if (!found)
            return false;
    }

    return true;
}
#pragma endregion EXTENSIONS

#pragma region VALIDATION

void boitatah::vk::Vulkan::initializeDebugMessenger()
{
    if (!options.useValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateMessenger(createInfo);
    if (createDebugUtilsMessengerEXT(instance,
                                     &createInfo,
                                     nullptr,
                                     &debugMessenger) != VK_SUCCESS)
    {
    }
}

bool boitatah::vk::Vulkan::checkValidationLayerSupport(const std::vector<const char *> &required)
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);

    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto &req : required)
    {
        std::cout << req << "\n";
        bool found = false;
        for (const auto &available : availableLayers)
        {
            std::cout << "\t" << available.layerName << "\n";
            if (std::strcmp(req, available.layerName) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    return true;
}

void boitatah::vk::Vulkan::populateMessenger(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // optional
    createInfo.pNext = nullptr;
}

int boitatah::vk::Vulkan::evaluatePhysicalDevice(VkPhysicalDevice device)
{
\

    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(device, &deviceProps);

    VkPhysicalDeviceFeatures deviceFeats;
    vkGetPhysicalDeviceFeatures(device, &deviceFeats);

    bool swapChainSupported = true;
    if (checkDeviceExtensionSupport(device))
    {
        SwapchainSupport support = getSwapchainSupport(device);
        swapChainSupported = !support.formats.empty() && !support.presentModes.empty();
    }

    return ((deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) * 1000 +
            deviceProps.limits.maxImageDimension2D) *
           (deviceFeats.geometryShader != 0);                     // *
    (findQueueFamilies(device).graphicsFamily.has_value() != 0) * // has graphics family queues
        (swapChainSupported);
}

bool boitatah::vk::Vulkan::checkDeviceExtensionSupport(VkPhysicalDevice device)
{

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

    std::set<std::string> required(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : extensions)
    {
        required.erase(extension.extensionName);
    }

    return required.empty();
}

#pragma endregion VALIDATION

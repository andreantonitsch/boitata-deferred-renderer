#include <boitatah/backend/vulkan/Vulkan.hpp>

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

#include <boitatah/backend/vulkan/Window.hpp>
#include <boitatah/utils/utils.hpp>

namespace bvk = boitatah::vk;

using boitatah::COLOR_SPACE;
using boitatah::COMMAND_BUFFER_LEVEL;
using boitatah::IMAGE_FORMAT;
using boitatah::FRAME_BUFFERING;
using boitatah::Image;
using boitatah::IMAGE_LAYOUT;
using boitatah::IMAGE_USAGE;
using boitatah::MEMORY_PROPERTY;
using boitatah::SAMPLES;
using boitatah::Shader;
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
bvk::VulkanInstance::VulkanInstance(VulkanOptions opts)
{
    m_options = opts;

    if (m_options.useValidationLayers)
    {
        m_validation_layers.emplace_back("VK_LAYER_KHRONOS_validation");
        m_instance_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    for (const auto &ext : m_options.extensions)
    {
        m_instance_extensions.emplace_back(ext);
    }

    m_device_extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    init_instance();

}

std::shared_ptr<bvk::VulkanInstance> boitatah::vk::VulkanInstance::create(VulkanOptions opts)
{
    std::shared_ptr<VulkanInstance> new_vulkan = std::shared_ptr<VulkanInstance>(new VulkanInstance(opts));
    return new_vulkan;
}

bvk::VulkanInstance::~VulkanInstance(void)
{

    vkDestroyCommandPool(m_device, m_command_pools.graphicsPool, nullptr);
    vkDestroyCommandPool(m_device, m_command_pools.transferPool, nullptr);
    vkDestroyCommandPool(m_device, m_command_pools.presentPool, nullptr);

    vkDestroyDevice(m_device, nullptr);

    if (m_options.useValidationLayers)
        destroyDebugUtilsMessengerEXT(instance, m_debug_messenger, nullptr);

    // vkDestroySurfaceKHR(instance, surface, nullptr); //done
    vkDestroyInstance(instance, nullptr);
}

VkInstance boitatah::vk::VulkanInstance::get_instance() const
{
    return instance;
}

VkDevice boitatah::vk::VulkanInstance::get_device() const
{
    return m_device;
}

VkPhysicalDevice boitatah::vk::VulkanInstance::get_physical_device() const
{
    return m_physical_device;
}

VkQueue boitatah::vk::VulkanInstance::get_transfer_queue() const
{
    return m_queues.transferQueue;
}

VkQueue boitatah::vk::VulkanInstance::get_graphics_queue() const
{
    return m_queues.graphicsQueue;
}

VkQueue boitatah::vk::VulkanInstance::get_present_queue() const
{
    return m_queues.presentQueue;
}

void boitatah::vk::VulkanInstance::attach_window(std::shared_ptr<WindowManager> window)
{
    this->window = window;
}

void boitatah::vk::VulkanInstance::init_instance()
{
    // App Info
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = m_options.appName,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3};

    // Extension Availability check
    // TODO For some MacOS sdks we need to add VK_KHR_PORTABILITY_subset.
    std::vector<VkExtensionProperties> availableExtensions = retrieve_available_extensions();

    for (const auto &ext : availableExtensions)
    {
        std::cout << ext.extensionName << std::endl;
    }

    if (m_options.useValidationLayers)
        for (const auto &ext : m_instance_extensions)
        {
            std::cout << "required :: " << ext << std::endl;
        }

    bool hasAllExtensions = check_required_extensions(availableExtensions,
                                                    m_instance_extensions);

    if (!hasAllExtensions)
        throw std::runtime_error("Vulkan does not have all required extensions available.");

    // Create Info
    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = (uint32_t)(m_instance_extensions.size()),
        .ppEnabledExtensionNames = m_instance_extensions.data(),
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    if (m_options.useValidationLayers)
    {
        if (check_validation_support(m_validation_layers))
        {

            populate_messenger(debugCreateInfo);
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
            createInfo.ppEnabledLayerNames = m_validation_layers.data();
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        }
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan Instance.");
}

void boitatah::vk::VulkanInstance::finish_initialization()
{
    init_debug_messenger();
    init_physical_device();
    init_logical_device_queues();
    set_queues();
    create_commandpools();
    m_queue_family_indices = find_queuefamilies(m_physical_device);
}

#pragma endregion Initialization

#pragma region Synchronization
void boitatah::vk::VulkanInstance::wait_for_frame(RenderTargetSync &bufferData)
{
    VkResult result = vkWaitForFences(m_device, 1, &bufferData.in_flight_fence, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS)
        std::cout << "wait for fence failed " << result << std::endl;
    vkResetFences(m_device, 1, &bufferData.in_flight_fence);
}

void boitatah::vk::VulkanInstance::wait_idle()
{
    vkDeviceWaitIdle(m_device);
}

void boitatah::vk::VulkanInstance::reset_fence(const VkFence &fence) const
{
    vkResetFences(m_device, 1, &fence);
}


void boitatah::vk::VulkanInstance::wait_for_fence(const VkFence &fence) const
{
    VkResult result = vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS)
        std::cout << "wait for fence failed " << result << std::endl;
}

bool boitatah::vk::VulkanInstance::check_fence_status(VkFence fence)
{
    VkResult result = vkGetFenceStatus(m_device, fence);
    return result == VK_SUCCESS; //fence is signaled
}

#pragma endregion Synchronization

#pragma region PSO Building

VkRenderPass boitatah::vk::VulkanInstance::create_renderpass(const RenderPassDesc &desc) const
{
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colorAttachmentRefs;    
    
    VkAttachmentReference depthStencilAttachmentRef;
    
    for (const auto &attDesc : desc.color_attachments)
    {
        attachments.push_back(create_attachmentdescription(attDesc));

        colorAttachmentRefs.push_back({
            .attachment = static_cast<uint32_t>(attDesc.index),
            .layout = castEnum<VkImageLayout>(attDesc.layout),
        });
    }

    //subdependency for layout transistions.
    VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask =  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        };

    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size()),
        .pColorAttachments = colorAttachmentRefs.data(),
        

    };

    if(desc.use_depthStencil){
            attachments.push_back(create_attachmentdescription(desc.depth_attachment));
            depthStencilAttachmentRef = {.attachment = static_cast<uint32_t>(desc.depth_attachment.index),
                                         .layout = castEnum<VkImageLayout>(desc.depth_attachment.layout)};
            subpass.pDepthStencilAttachment = &depthStencilAttachmentRef;

    }

    VkRenderPassCreateInfo renderPassCreate{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency};

    VkRenderPass pass;
    if (vkCreateRenderPass(m_device, &renderPassCreate, nullptr, &pass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Render Pass");
    }

    return pass;
}

Image boitatah::vk::VulkanInstance::create_image(const ImageDesc &desc) const
{

    Image image;
    if(desc.usage == IMAGE_USAGE::TRANSFER_DST_SAMPLED)
        std::cout << "creating sampled texture" << std::endl;
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
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = castEnum<VkImageLayout>(desc.initialLayout),
    };

    VkImage vkImage;

    if (vkCreateImage(m_device, &createInfo, nullptr, &vkImage) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to Create Image");
    }

    image.dimensions = desc.dimensions;
    image.image = vkImage;

    VkMemoryRequirements reqs;
    vkGetImageMemoryRequirements(m_device, vkImage, &reqs);

    image.memory = allocate_memory({
        .size = reqs.size,
        .type = MEMORY_PROPERTY::DEVICE_LOCAL,
        .typeBits = reqs.memoryTypeBits,
    });

    bind_image_memory(image.memory, image.image);

    return image;
}

VkImageView boitatah::vk::VulkanInstance::create_imageview(VkImage image, const ImageDesc &desc) const
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
            .a = VK_COMPONENT_SWIZZLE_IDENTITY},};
    if (desc.usage == IMAGE_USAGE::DEPTH_STENCIL ||
        desc.usage == IMAGE_USAGE::RENDER_GRAPH_DEPTH){
        createInfo.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                             .baseMipLevel = 0, .levelCount = 1, 
                             .baseArrayLayer = 0, .layerCount = 1};}
    else {
        createInfo.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                             .baseMipLevel = 0, .levelCount = 1, 
                             .baseArrayLayer = 0, .layerCount = 1};}

    if (vkCreateImageView(m_device, &createInfo, nullptr, &view) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Swapchain image view");
    }

    return view;
}

uint32_t boitatah::vk::VulkanInstance::find_memory_index(const MemoryDesc &props) const
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memProperties);

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

uint32_t boitatah::vk::VulkanInstance::get_buffer_alignment(const VkBuffer buffer) const
{
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_device, buffer, &memReqs);
    return memReqs.alignment;
}

boitatah::CommandBuffer boitatah::vk::VulkanInstance::allocate_commandbuffer(const CommandBufferDesc &desc)
{
    VkCommandBufferAllocateInfo allocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = castEnum<VkCommandBufferLevel>(desc.level),
        .commandBufferCount = desc.count};

    switch (desc.type)
    {
    case COMMAND_BUFFER_TYPE::GRAPHICS:
        allocateInfo.commandPool = m_command_pools.graphicsPool;
        break;
    case COMMAND_BUFFER_TYPE::TRANSFER:
        allocateInfo.commandPool = m_command_pools.transferPool;
        break;
    case COMMAND_BUFFER_TYPE::PRESENT:
        allocateInfo.commandPool = m_command_pools.presentPool;
        break;
    }

    VkCommandBuffer buffer;
    if (vkAllocateCommandBuffers(m_device, &allocateInfo, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Command Buffer");
    }

    return CommandBuffer{.buffer = buffer, .type = desc.type};
}



bool boitatah::vk::VulkanInstance::present(Image &swapchainImage,
                                        VkSwapchainKHR &swapchain,
                                        uint32_t &scIndex,
                                        const PresentCommandVk &command)
{
    //  Present
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = static_cast<uint32_t>(command.waitSemaphores.size()),
        .pWaitSemaphores = command.waitSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &scIndex,
        .pResults = nullptr};

    VkResult result = vkQueuePresentKHR(m_queues.presentQueue, &presentInfo);

    // swapchain is too small or too large
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        return false;
    
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to present image to swapchain");

    return true;
}


VkDeviceMemory boitatah::vk::VulkanInstance::allocate_memory(const MemoryDesc &desc) const
{
    VkMemoryAllocateInfo allocateInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = desc.size,
        .memoryTypeIndex = find_memory_index(desc),
    };

    VkDeviceMemory memory;

    if (vkAllocateMemory(m_device, &allocateInfo, nullptr, &memory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate Memory");
    

    return memory;
}

void *boitatah::vk::VulkanInstance::map_memory(const MapMemoryVk &desc) const
{
    void *mappedTarget;
    if(vkMapMemory(m_device, desc.memory, desc.offset, desc.size, 0, &mappedTarget) != VK_SUCCESS)
        return nullptr;

    return mappedTarget;
}

void boitatah::vk::VulkanInstance::unmap_memory(const UnmapMemoryVk &desc) const
{
    vkUnmapMemory(m_device, desc.memory);
}

void boitatah::vk::VulkanInstance::bind_image_memory(VkDeviceMemory memory, VkImage image) const
{
    // binds from start.
    // currently using one buffer per image.
    //TODO create a large memory region for images
    vkBindImageMemory(m_device, image, memory, 0);
}

void boitatah::vk::VulkanInstance::data_copy_to_buffer(CopyToBufferVk op) const
{
    //maps a pointer to mapped memory, offset from its start.
    // size is the memory range to be mapped.   
    void * mapMem = map_memory({.memory = op.memory, .offset = op.offset, .size = op.size});

    std::byte* byte_source = static_cast<std::byte*>(op.data);
    std::copy(byte_source, byte_source + op.size, static_cast<std::byte*>(mapMem));
    unmap_memory({.memory = op.memory});
}

void boitatah::vk::VulkanInstance::copy_to_mapped_memory(const CopyMappedMemoryVk &op) const
{
    std::byte* start = static_cast<std::byte*>(op.data) + op.offset;
    std::byte* end = start + op.elementSize * op.elementCount;
    
    std::copy(
        start,
        end,
        static_cast<std::byte*>(op.map));
}

VkPipelineLayout boitatah::vk::VulkanInstance::create_shaderlayout(const ShaderLayoutDescVk &desc) const
{
    VkPipelineLayout layout;
    
    //start with base layout layout
    std::vector<VkDescriptorSetLayout> descripLayouts(desc.materialLayouts);
    
    //add model push constants
    std::vector<VkPushConstantRange> ranges;

    for(auto& push_constant : desc.pushConstants){
        ranges.push_back(
        VkPushConstantRange{
          .stageFlags = castEnum<VkShaderStageFlags>(push_constant.stages), 
          .offset = push_constant.offset,
          .size = push_constant.size,
        });
    }
    
    VkPipelineLayoutCreateInfo layoutCreate{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = static_cast<uint32_t>(descripLayouts.size()),
        .pSetLayouts = descripLayouts.size() == 0 ? nullptr : descripLayouts.data(),
        .pushConstantRangeCount = static_cast<uint32_t>(ranges.size()),
        .pPushConstantRanges = ranges.data(),
    };
 
    if (vkCreatePipelineLayout(m_device, &layoutCreate, nullptr, &layout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create Pipeline Layout");
    }

    return layout;
}

VkDescriptorSetLayout boitatah::vk::VulkanInstance::create_descriptorlayout(const DescriptorSetLayoutDesc &desc) const
{
    VkDescriptorSetLayout layout;

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    uint32_t binding_index = 0;

    for(const auto& bindingDesc : desc.bindingDescriptors){
        VkDescriptorSetLayoutBinding binding{
            .binding = binding_index,
            .descriptorType = castEnum<VkDescriptorType>(bindingDesc.type),
            .descriptorCount = bindingDesc.descriptorCount,
            .stageFlags = castEnum<VkShaderStageFlags>(bindingDesc.stages),
        };
        bindings.push_back(binding);
        binding_index++;
    }

    VkDescriptorSetLayoutCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.size() == 0 ? 0 : bindings.data(),
    };

    if(vkCreateDescriptorSetLayout(m_device, &info, nullptr, &layout) != VK_SUCCESS){
        throw std::runtime_error("Failed to create Descriptor Set Layout");
    }

    return layout;
}

VkFence boitatah::vk::VulkanInstance::create_fence(bool signaled) const
{
    VkFenceCreateInfo fenceInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    if (signaled)
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence;
    if (vkCreateFence(m_device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Fence");
    std::cout << "created fence" << std::endl;
    return fence;
}

VkSemaphore boitatah::vk::VulkanInstance::create_semaphore() const
{
    VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkSemaphore semaphore;

    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &semaphore))
        throw std::runtime_error("Failed to create Semaphore");

    return semaphore;
}

boitatah::vk::BufferVkData boitatah::vk::VulkanInstance::create_buffer(const BufferDescVk &desc) const
{

    const std::vector<uint32_t> indexes = {m_queue_family_indices.graphicsFamily.value(),
                                           m_queue_family_indices.transferFamily.value()};

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
    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a buffer");
        ;
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_device, buffer, &memReqs);

    VkDeviceMemory memory;
    VkMemoryAllocateInfo memInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = find_memory_index({
            .size = memReqs.size,
            .type = MEMORY_PROPERTY::HOST_VISIBLE_COHERENT,
            .typeBits = memReqs.memoryTypeBits,
        })};

    if (vkAllocateMemory(m_device, &memInfo, nullptr, &memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to  allocate buffer memory");
    }

    vkBindBufferMemory(m_device, buffer, memory, 0);

    return {.buffer = buffer, .memory = memory, .alignment = memReqs.alignment, .actualSize =memReqs.size };
}

VkSampler boitatah::vk::VulkanInstance::create_sampler(const SamplerData &data) const
{
    VkSamplerCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    
    info.addressModeU = castEnum<VkSamplerAddressMode>(data.u_tiling);
    info.addressModeV = castEnum<VkSamplerAddressMode>(data.v_tiling);
    info.addressModeW = castEnum<VkSamplerAddressMode>(SAMPLER_TILE_MODE::REPEAT);
    info.anisotropyEnable = data.anisotropy;
    info.maxAnisotropy = std::min(m_device_properties.limits.maxSamplerAnisotropy, 
                                  data.maxAnisotropy);
    info.unnormalizedCoordinates = data.normalized ? VK_FALSE : VK_TRUE;
    info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    info.compareEnable = VK_FALSE;
    info.compareOp = VK_COMPARE_OP_ALWAYS;
    
    info.magFilter = castEnum<VkFilter>(data.magFilter);
    info.minFilter = castEnum<VkFilter>(data.minFilter);

    info.mipmapMode = castEnum<VkSamplerMipmapMode>(data.mipmap);
    info.mipLodBias = data.lodBias;
    info.minLod = 0.0f;
    info.maxLod = 0.0f;

    VkSampler sampler;
    VkResult result = vkCreateSampler(m_device, &info, nullptr, &sampler);

    if(result != VK_SUCCESS)
        std::runtime_error("Failed to create sampler");

    return sampler;
}

boitatah::vk::BufferVkData boitatah::vk::VulkanInstance::get_buffer_alignment_memorytype(const BufferDescVk &desc) const
{
    const std::vector<uint32_t> indexes = {m_queue_family_indices.graphicsFamily.value(),
                                           m_queue_family_indices.transferFamily.value()};

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
    if (vkCreateBuffer(m_device, &dummyCreate, nullptr, &dummyBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a buffer");
        ;
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_device, dummyBuffer, &memReqs);

    vkDestroyBuffer(m_device, dummyBuffer, nullptr);

    return BufferVkData{
        .alignment = memReqs.alignment,
        .memoryTypeBits = memReqs.memoryTypeBits};
}

boitatah::RenderTargetSync boitatah::vk::VulkanInstance::allocate_render_sync_structures()
{
    RenderTargetSync sync{
            .draw_buffer = allocate_commandbuffer({.count = 1,
                                                 .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                                                 .type = COMMAND_BUFFER_TYPE::GRAPHICS}),
            .present_buffer = allocate_commandbuffer({.count = 1,
                                                     .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                                                     .type = COMMAND_BUFFER_TYPE::TRANSFER}),
            .draw_semaphore = create_semaphore(),
            .sc_aquired_semaphore = create_semaphore(),
            .transfer_semaphore = create_semaphore(),
            .in_flight_fence = create_fence(true),
        };
    return sync;
}

void boitatah::vk::VulkanInstance::build_shader(const ShaderDescVk &desc, Shader &shader)
{
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()};


    uint32_t bindingCount = static_cast<uint32_t>(desc.bindings.size());
    uint32_t attributeCount = static_cast<uint32_t>(desc.attributes.size());

    VkPipelineDepthStencilStateCreateInfo depthStencil{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_GREATER,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

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

    //TODO temp workaround
    std::vector<VkPipelineColorBlendAttachmentState> blend_attachments;
    for(auto& blend : desc.colorBlends)
        blend_attachments.push_back(colorBlendAttachment);

    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = static_cast<uint32_t>(blend_attachments.size()),
        .pAttachments = blend_attachments.data(),
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };

    VkPipelineShaderStageCreateInfo stages[] = {
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_VERTEX_BIT,
         .module = shader.vert.shaderModule,
         .pName = shader.vert.entryFunction.c_str()},
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
         .module = shader.frag.shaderModule,
         .pName = shader.frag.entryFunction.c_str()}};

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
    if(desc.use_depth)
        pipelineInfo.pDepthStencilState = &depthStencil;
    if (vkCreateGraphicsPipelines(m_device,
                                  VK_NULL_HANDLE,
                                  1,
                                  &pipelineInfo,
                                  nullptr,
                                  &(shader.pipeline)) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline");
    }
}

VkShaderModule boitatah::vk::VulkanInstance::create_shadermodule(const std::vector<char> &bytecode) const
{
    VkShaderModuleCreateInfo shaderCreate{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = bytecode.size(),
        .pCode = reinterpret_cast<const uint32_t *>(bytecode.data()),
    };

    VkShaderModule module;
    if (vkCreateShaderModule(m_device, &shaderCreate, nullptr, &module) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Shader Module");
    }
    return module;
}

VkFramebuffer boitatah::vk::VulkanInstance::create_framebuffer(const FramebufferDescVk &desc) const
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

    VkResult r = vkCreateFramebuffer(m_device, &createInfo, nullptr, &buffer);
    if (r != VK_SUCCESS)
    {
        std::cout << "\n#### " << r << "\n " << std::endl;
        throw std::runtime_error("failed to create framebuffer");
    }

    return buffer;
}

VkAttachmentDescription boitatah::vk::VulkanInstance::create_attachmentdescription(const AttachmentDesc &attDesc) const
{   
    VkAttachmentLoadOp load_op;
    if(attDesc.clear)
        load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
    else
        load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
    return VkAttachmentDescription{.format = castEnum<VkFormat>(attDesc.format),
                                   .samples = castEnum<VkSampleCountFlagBits>(attDesc.samples),
                                   .loadOp = load_op,
                                   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                   .stencilLoadOp = load_op,
                                   .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                   .initialLayout = castEnum<VkImageLayout>(attDesc.initialLayout),
                                   .finalLayout = castEnum<VkImageLayout>(attDesc.finalLayout)};
}
#pragma endregion PSO Building

#pragma region Object Destructions

void boitatah::vk::VulkanInstance::destroy_shader(Shader &shader)
{
    vkDestroyShaderModule(m_device, shader.vert.shaderModule, nullptr);
    vkDestroyShaderModule(m_device, shader.frag.shaderModule, nullptr);
    vkDestroyPipeline(m_device, shader.pipeline, nullptr);
}

void boitatah::vk::VulkanInstance::destroy_renderpass(Renderpass &pass)
{
    vkDestroyRenderPass(m_device, pass.renderPass, nullptr);
}

void boitatah::vk::VulkanInstance::destroy_framebuffer(RenderTarget &framebuffer)
{
    
    vkDestroyFramebuffer(m_device, framebuffer.buffer, nullptr);
}

void boitatah::vk::VulkanInstance::destroy_image(Image image)
{
    vkDestroyImageView(m_device, image.view, nullptr);
    if (!image.swapchain)
    {
        vkDestroyImage(m_device, image.image, nullptr);
        vkFreeMemory(m_device, image.memory, nullptr);
    }
}

void boitatah::vk::VulkanInstance::destroy_pipelinelayout(ShaderLayout &layout)
{
    vkDestroyPipelineLayout(m_device, layout.pipeline, nullptr);
}

void boitatah::vk::VulkanInstance::destroy_rendertarget_sync(const RenderTargetSync &sync)
{
    vkFreeCommandBuffers(m_device, m_command_pools.graphicsPool, 1, &(sync.draw_buffer.buffer));
    vkFreeCommandBuffers(m_device, m_command_pools.transferPool, 1, &(sync.present_buffer.buffer));

    vkDestroyFence(m_device, sync.in_flight_fence, nullptr);
    vkDestroySemaphore(m_device, sync.sc_aquired_semaphore, nullptr);
    vkDestroySemaphore(m_device, sync.draw_semaphore, nullptr);
    vkDestroySemaphore(m_device, sync.transfer_semaphore, nullptr);
}

void boitatah::vk::VulkanInstance::destroy_buffer(BufferVkData buffer) const
{
    vkDestroyBuffer(m_device, buffer.buffer, nullptr);
    vkFreeMemory(m_device, buffer.memory, nullptr);
}

void boitatah::vk::VulkanInstance::destroy_fence(VkFence fence)
{
    vkDestroyFence(m_device, fence, nullptr);
}

void boitatah::vk::VulkanInstance::destroy_semaphore(VkSemaphore semaphore)
{
    vkDestroySemaphore(m_device, semaphore, nullptr);
}

void boitatah::vk::VulkanInstance::destroy_descriptorpool(VkDescriptorPool pool)
{
    vkDestroyDescriptorPool(m_device, pool, nullptr);
}

void boitatah::vk::VulkanInstance::destroy_descriptorset_layout(VkDescriptorSetLayout &layout)
{
    vkDestroyDescriptorSetLayout(m_device, layout, nullptr);
}

void boitatah::vk::VulkanInstance::destroy_sampler(VkSampler &sampler)
{  
    vkDestroySampler(m_device, sampler, nullptr);
}

#pragma endregion Object Destructions

#pragma region QUEUE_SETUP

void boitatah::vk::VulkanInstance::init_logical_device_queues()
{
    QueueFamilyIndices familyIndices = find_queuefamilies(m_physical_device);

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
        .enabledExtensionCount = static_cast<uint32_t>(m_device_extensions.size()),
        .ppEnabledExtensionNames = m_device_extensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };

    if (m_options.useValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
        createInfo.ppEnabledLayerNames = m_validation_layers.data();
    }

    if (vkCreateDevice(m_physical_device, &createInfo, nullptr, &m_device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to initialize a Logical Device");
    }
}

void boitatah::vk::VulkanInstance::set_queues()
{
    QueueFamilyIndices familyIndices = find_queuefamilies(m_physical_device);
    vkGetDeviceQueue(m_device, familyIndices.graphicsFamily.value(), 0, &m_queues.graphicsQueue);
    vkGetDeviceQueue(m_device, familyIndices.presentFamily.value(), 0, &m_queues.presentQueue);
    vkGetDeviceQueue(m_device, familyIndices.transferFamily.value(), 0, &m_queues.transferQueue);
}

void boitatah::vk::VulkanInstance::create_commandpools()
{
    VkCommandPoolCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        // two types of pools. The other is VK_COMMNAD_POOL_CREATE_TRANSIENT_BIT
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = find_queuefamilies(m_physical_device).graphicsFamily.value(),
    };

    if (vkCreateCommandPool(m_device, &info, nullptr, &m_command_pools.graphicsPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Graphics Command Pool");
    }

    info.queueFamilyIndex = find_queuefamilies(m_physical_device).transferFamily.value();
    if (vkCreateCommandPool(m_device, &info, nullptr, &m_command_pools.transferPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Transfer Command Pool");
    }

    info.queueFamilyIndex = find_queuefamilies(m_physical_device).presentFamily.value();
    if (vkCreateCommandPool(m_device, &info, nullptr, &m_command_pools.presentPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Present Command Pool");
    }
}

#pragma endregion QUEUE_SETUP

#pragma region DEVICE_SETUP

void boitatah::vk::VulkanInstance::init_physical_device()
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
        int value = eval_physical_device(device);
        candidates.insert(std::make_pair(value, device));
    }

    if (candidates.rbegin()->first > 0)
    {
        m_physical_device = candidates.rbegin()->second;

        // DEBUG MESSAGE
        if (m_options.debugMessages)
        {
            VkPhysicalDeviceProperties deviceProps;
            vkGetPhysicalDeviceProperties(m_physical_device, &deviceProps);
            std::cout << "###DEBUG Physical Device### \n"
                      << "Picked physcal device " << deviceProps.deviceName << "\n"
                      << "Device type " << deviceProps.deviceType << "\n"
                      << "Device ID " << deviceProps.deviceID << "\n"
                      << "###DEBUG Physical Device END###" << std::endl;
        }
    }
    if (m_physical_device == VK_NULL_HANDLE)
        std::runtime_error("No available GPUs with Vulkan Support");


    //set the Vulkan member properties.
    vkGetPhysicalDeviceProperties(m_physical_device, &m_device_properties);

}

const boitatah::vk::QueueFamilyIndices boitatah::vk::VulkanInstance::get_queuefamily_indices() const
{
    return m_queue_family_indices;
}

boitatah::vk::QueueFamilyIndices boitatah::vk::VulkanInstance::find_queuefamilies(VkPhysicalDevice device) const
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
std::vector<VkExtensionProperties> bvk::VulkanInstance::retrieve_available_extensions()
{

    uint32_t extensionCount = 0;
    // call once to determine extension count
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    // create a vector and populate it with the extension properties.
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    return extensions;
}

boitatah::vk::SwapchainSupport boitatah::vk::VulkanInstance::get_swapchain_support(VkPhysicalDevice physicalDevice)
{
    SwapchainSupport support;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, window->getSurface(), &support.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, window->getSurface(), &formatCount, nullptr);

    if (m_options.useValidationLayers)
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

bool bvk::VulkanInstance::check_required_extensions(const std::vector<VkExtensionProperties> &available,
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

void boitatah::vk::VulkanInstance::init_debug_messenger()
{
    if (!m_options.useValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populate_messenger(createInfo);
    if (createDebugUtilsMessengerEXT(instance,
                                     &createInfo,
                                     nullptr,
                                     &m_debug_messenger) != VK_SUCCESS)
    {
    }
}

bool boitatah::vk::VulkanInstance::check_validation_support(const std::vector<const char *> &required)
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

void boitatah::vk::VulkanInstance::populate_messenger(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
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

int boitatah::vk::VulkanInstance::eval_physical_device(VkPhysicalDevice device)
{
\

    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(device, &deviceProps);

    VkPhysicalDeviceFeatures deviceFeats;
    vkGetPhysicalDeviceFeatures(device, &deviceFeats);

    bool swapChainSupported = true;
    if (check_device_ext_support(device))
    {
        SwapchainSupport support = get_swapchain_support(device);
        swapChainSupported = !support.formats.empty() && !support.presentModes.empty();
    }

    return ((deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) * 1000 +
            deviceProps.limits.maxImageDimension2D) *
           (deviceFeats.geometryShader != 0);                     // *
    (find_queuefamilies(device).graphicsFamily.has_value() != 0) * // has graphics family queues
        (swapChainSupported);
}

bool boitatah::vk::VulkanInstance::check_device_ext_support(VkPhysicalDevice device)
{

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

    std::set<std::string> required(m_device_extensions.begin(), m_device_extensions.end());

    for (const auto &extension : extensions)
    {
        required.erase(extension.extensionName);
    }

    return required.empty();
}

#pragma endregion VALIDATION

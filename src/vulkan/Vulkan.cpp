#include "Vulkan.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>

#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"

namespace bvk = boitatah::vk;

using boitatah::COLOR_SPACE;
using boitatah::FORMAT;
using boitatah::FRAME_BUFFERING;
using boitatah::Shader;
using boitatah::ShaderDesc;

#pragma region Enum Specializations

template <>
inline VkFormat boitatah::vk::Vulkan::castEnum(FORMAT format)
{
    switch (format)
    {
    case RGBA_8_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case BGRA_8_SRGB:
        return VK_FORMAT_B8G8R8A8_SRGB;
    case RGBA_8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case BGRA_8_UNORM:
        return VK_FORMAT_B8G8R8A8_UNORM;
    default:
        return VK_FORMAT_UNDEFINED;
    }
}
template VkFormat boitatah::vk::Vulkan::castEnum<FORMAT, VkFormat>(FORMAT);

template <>
inline VkColorSpaceKHR boitatah::vk::Vulkan::castEnum(COLOR_SPACE colorSpace)
{
    switch (colorSpace)
    {
    case SRGB_NON_LINEAR:
        return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    default:
        return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
}
template VkColorSpaceKHR boitatah::vk::Vulkan::castEnum<COLOR_SPACE, VkColorSpaceKHR>(COLOR_SPACE);

template <>
inline VkPresentModeKHR boitatah::vk::Vulkan::castEnum(FRAME_BUFFERING buffering_mode)
{
    switch (buffering_mode)
    {
    case NO_BUFFER:
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    case VSYNC:
        return VK_PRESENT_MODE_FIFO_KHR;
    case TRIPLE_BUFFER:
        return VK_PRESENT_MODE_MAILBOX_KHR;
    default:
        return VK_PRESENT_MODE_FIFO_KHR;
    }
}
template VkPresentModeKHR boitatah::vk::Vulkan::castEnum<FRAME_BUFFERING, VkPresentModeKHR>(FRAME_BUFFERING);

#pragma endregion Enum Specializations

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
    createSurface(opts.window);
    initializeDebugMessenger();

    initPhysicalDevice();
    initLogicalDeviceNQueues();
    setQueues();
}

bvk::Vulkan::~Vulkan(void)
{

    clearSwapchainViews();

    vkDestroySwapchainKHR(device, swapchain, nullptr);

    vkDestroyDevice(device, nullptr);

    if (options.useValidationLayers)
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
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
        .apiVersion = VK_API_VERSION_1_0};

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

#pragma region PSO Building

void boitatah::vk::Vulkan::buildShader(const ShaderDesc &desc, Shader &shader)
{
    injectPipelineLayout(desc, shader);
    injectRenderPass(desc, shader);
    injectPSO(desc, shader);
}

void boitatah::vk::Vulkan::injectRenderPass(const ShaderDesc &desc, Shader &shader)
{
    VkAttachmentDescription colorAttachment{
        .format = swapchainFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    //Directly related to layout(location = 0) in shader.
    VkAttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
    };

    VkRenderPassCreateInfo renderPassCreate{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass
    };

    if(vkCreateRenderPass(device, &renderPassCreate, nullptr, &(shader.renderPass))!= VK_SUCCESS){
        throw std::runtime_error("Failed to create Render Pass");
    }

}

void boitatah::vk::Vulkan::injectPipelineLayout(const ShaderDesc &desc, Shader &shader)
{
    VkPipelineLayoutCreateInfo layoutCreate{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr, 
    };

    if(vkCreatePipelineLayout(device, &layoutCreate, nullptr, &(shader.layout)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create Pipeline Layout");
    } 
}

void boitatah::vk::Vulkan::injectPSO(const ShaderDesc &desc, Shader &shader)
{
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr

    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    //Dynamic state
    VkPipelineViewportStateCreateInfo dynamicViewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };

    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE, //if true rasterizes discards geometry.
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
        .alphaToCoverageEnable= VK_FALSE,
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
        .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f},
    };

    VkPipelineShaderStageCreateInfo stages[] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = shader.vert,
            .pName = desc.vert.entryFunction.c_str()
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = shader.frag,
            .pName = desc.frag.entryFunction.c_str()
        }
    };

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
        .layout = shader.layout,
        .renderPass = shader.renderPass,
        .subpass = 0,

        // for deriving pipelines.
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    if(vkCreateGraphicsPipelines(device, 
    VK_NULL_HANDLE, 
    1, 
    &pipelineInfo, 
    nullptr,
    &(shader.PSO) )!= VK_SUCCESS){
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

void boitatah::vk::Vulkan::destroyShaderModule(VkShaderModule module)
{
    vkDestroyShaderModule(device, module, nullptr);
}

#pragma endregion PSO Building

#pragma region SWAPCHAIN

void boitatah::vk::Vulkan::clearSwapchainViews()
{
    for (auto view : swapchainViews)
    {
        vkDestroyImageView(device, view, nullptr);
    }
}

// does 3 main things.
// chooses, format (e.g. rgba8 srgb), present mode (e.g. vsync) and extent(i.e. resolution
// buildSwapchain();
void boitatah::vk::Vulkan::buildSwapchain(FORMAT scFormat)
{
    clearSwapchainViews();
    createSwapchain(scFormat);
    createSwapchainViews(scFormat);
}


boitatah::vk::SwapchainSupport boitatah::vk::Vulkan::getSwapchainSupport(VkPhysicalDevice device)
{
    SwapchainSupport support;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &support.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (options.useValidationLayers)
        std::cout << " AVAILABLE SURFACE FORMATS " << std::endl;

    if (formatCount != 0)
    {
        support.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device,
                                             surface,
                                             &formatCount,
                                             support.formats.data());

        for (const auto &format : support.formats)
        {
            std::cout << " \tFORMAT :: " << format.format << std::endl;
        }
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        support.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                                  surface,
                                                  &presentModeCount,
                                                  support.presentModes.data());
    }

    return support;
}

VkSurfaceFormatKHR boitatah::vk::Vulkan::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats,
    FORMAT scFormat,
    COLOR_SPACE scColorSpace)
{
    for (const auto &surfaceFormat : availableFormats)
    {
        // GPUs usually display in BGRA [citation needed]
        if (surfaceFormat.format == castEnum<FORMAT, VkFormat>(scFormat) &&
            surfaceFormat.colorSpace == castEnum<COLOR_SPACE, VkColorSpaceKHR>(scColorSpace))
        {
            return surfaceFormat;
        }
    }
    throw std::runtime_error("Unable to select Swapchain format");
}

VkPresentModeKHR boitatah::vk::Vulkan::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availableModes)
{
    for (const auto &mode : availableModes)
    {
        if (mode == castEnum<FRAME_BUFFERING, VkPresentModeKHR>(FRAME_BUFFERING::TRIPLE_BUFFER))
        {
            return mode;
        }
    }

    return castEnum<FRAME_BUFFERING, VkPresentModeKHR>(FRAME_BUFFERING::VSYNC);
}

VkExtent2D boitatah::vk::Vulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(options.window, &width, &height);

        VkExtent2D extent{
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height)};

        extent.width = std::clamp(extent.width,
                                  capabilities.minImageExtent.width,
                                  capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height,
                                   capabilities.minImageExtent.height,
                                   capabilities.maxImageExtent.height);
        return extent;
    }
}

void boitatah::vk::Vulkan::createSwapchain(FORMAT scFormat)
{
    SwapchainSupport support = getSwapchainSupport(physicalDevice);

    VkSurfaceFormatKHR format = chooseSwapSurfaceFormat(support.formats,
                                                        FORMAT::BGRA_8_SRGB,
                                                        COLOR_SPACE::SRGB_NON_LINEAR);
    VkPresentModeKHR mode = chooseSwapPresentMode(support.presentModes);
    VkExtent2D extent = chooseSwapExtent(support.capabilities);

    uint32_t imageCount = support.capabilities.minImageCount;
    imageCount += 1;

    // 0 is a special value that mean no limit
    if (support.capabilities.maxImageCount > 0 &&
        imageCount > support.capabilities.maxImageCount)
    {
        imageCount = support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1, // 2 for stereoscopic aplications (nothing to do with deffered rendering)
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    // if queues are going to share burrers or not
    if (indices.graphicsFamily != indices.presentFamily)
    {
        if (options.useValidationLayers)
            std::cout << "FAMILIES :: USING TWO QUEUE FAMILIES: " << indices.graphicsFamily.value() << " and " << indices.presentFamily.value() << std::endl;
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        if (options.useValidationLayers)
            std::cout << "FAMILIES :: USING ONE QUEUE FAMILIES" << std::endl;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    // for pre-transformations, e.g. rotating 90o
    // current transform is no tranform.
    createInfo.preTransform = support.capabilities.currentTransform;

    // use alpha to blend with other windows.
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = mode;
    createInfo.clipped = VK_TRUE; // for obscured pixels in the window system.

    // for replacing the swap chain when resizing windows.
    createInfo.oldSwapchain = swapchain;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to Build a Swapchain");
    }

    swapchainFormat = format.format;
    swapchainExtent = extent;

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());
}

void boitatah::vk::Vulkan::createSwapchainViews(FORMAT scFormat)
{
    swapchainViews.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainViews.size(); i++)
    {
        VkImageViewCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = castEnum<FORMAT, VkFormat>(scFormat),
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
        };

        if (vkCreateImageView(device, &createInfo, nullptr, &swapchainViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create Swapchain image view");
        }
    }
}

#pragma endregion SWAPCHAIN

#pragma region QUEUE_SETUP

void boitatah::vk::Vulkan::initLogicalDeviceNQueues()
{
    QueueFamilyIndices familyIndices = findQueueFamilies(physicalDevice);

    float queuePriority = 1.0;
    VkDeviceQueueCreateInfo queueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = familyIndices.graphicsFamily.value(),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority};

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
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
    vkGetDeviceQueue(device, familyIndices.graphicsFamily.value(), 0, &graphicsQueue);
}

#pragma endregion QUEUE_SETUP

#pragma region DEVICE_SETUP
void boitatah::vk::Vulkan::createSurface(GLFWwindow *window)
{
    // TODO not ideal
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Window Surface");
}

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
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
            queueFamilies.presentFamily = i;

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

bool bvk::Vulkan::checkRequiredExtensions(const std::vector<VkExtensionProperties> &available,
                                          const std::vector<const char *> &required)
{
    for (const auto &ext : required)
    {
        bool found = false;
        for (const auto &property : available)
        {
            if (strcmp(ext, property.extensionName) == 0)
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
            if (strcmp(req, available.layerName) == 0)
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

    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(device, &deviceProps);

    VkPhysicalDeviceFeatures deviceFeats;
    vkGetPhysicalDeviceFeatures(device, &deviceFeats);

    bool swapChainSupported = false;
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

// bvk::Vulkan &bvk::Vulkan::operator=(const Vulkan &v)
// {
//     return *this;
// }

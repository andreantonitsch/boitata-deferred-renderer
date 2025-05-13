#include "Swapchain.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>


namespace boitatah
{
    Swapchain::Swapchain(SwapchainOptions options)
    {
        this->options = options;
        swapchainFormat = boitatah::castEnum<VkFormat>(options.format);
    }

    Swapchain::~Swapchain(void)
    {
        clearSwapchainViews();
        vkDestroySwapchainKHR(vulkan->get_device(), swapchain, nullptr);
    }

    SwapchainImage Swapchain::getNext(VkSemaphore &semaphore)
    {
        uint32_t index = UINT32_MAX;

        VkResult result = vkAcquireNextImageKHR(vulkan->get_device(),
                                                swapchain, UINT64_MAX,
                                                semaphore, VK_NULL_HANDLE,
                                                &index);
        //std::cout << static_cast<int>(result) << std::endl;;
        if(result == VK_ERROR_OUT_OF_DATE_KHR){
            //createSwapchain();
            return { .index = index, .sc = swapchain};
        }
        
        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            return { .index = index -1u, .sc = swapchain};

        currentIndex = index;
        return {.image = getSwapchainImages()[index],
                .index = currentIndex,
                .sc = swapchain};
    }

    SwapchainImage Swapchain::getCurrent()
    {
        return {.image = getSwapchainImages()[currentIndex],
                .index = currentIndex,
                .sc = swapchain};
    }

    void Swapchain::attach(std::shared_ptr<VulkanInstance> vulkan, std::shared_ptr<WindowManager> window)
    {
        this->vulkan = vulkan;
        this->window = window;
    }

    void Swapchain::createSwapchain()
    {
        clearSwapchainViews();
        swapchainImageCache.clear();
        vkDestroySwapchainKHR(vulkan->get_device(), swapchain, nullptr);
        createVkSwapchain();
        createViews();
    }

    void Swapchain::createVkSwapchain()
    {
        SwapchainSupport support = getSwapchainSupport(vulkan->get_physical_device());

        VkSurfaceFormatKHR format = chooseSwapSurfaceFormat(support.formats,
                                                            IMAGE_FORMAT::BGRA_8_SRGB,
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
            .surface = window->getSurface(),
            .minImageCount = imageCount,
            .imageFormat = format.format,
            .imageColorSpace = format.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1, // 2 for stereoscopic aplications (nothing to do with deffered rendering)
            .imageUsage = boitatah::castEnum<VkImageUsageFlagBits>(IMAGE_USAGE::COLOR_ATT_TRANSFER_DST),
            .oldSwapchain = VK_NULL_HANDLE
            };

        QueueFamilyIndices indices = vulkan->find_queuefamilies(vulkan->get_physical_device());
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        // if queues are going to share burrers or not
        if (indices.graphicsFamily != indices.presentFamily)
        {
 //           if (options.useValidationLayers)
//               std::cout << "FAMILIES :: USING TWO QUEUE FAMILIES: " << indices.graphicsFamily.value() << " and " << indices.presentFamily.value() << std::endl;
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
//            if (options.useValidationLayers)
 //               std::cout << "FAMILIES :: USING ONE QUEUE FAMILIES" << std::endl;
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
        //createInfo.oldSwapchain = swapchain;

        if (vkCreateSwapchainKHR(vulkan->get_device(), &createInfo, nullptr, &swapchain) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to Build a Swapchain");
        }

        swapchainFormat = format.format;
        swapchainExtent = extent;

        vkGetSwapchainImagesKHR(vulkan->get_device(), swapchain, &imageCount, nullptr);
        swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(vulkan->get_device(), swapchain, &imageCount, swapchainImages.data());
    }

    void Swapchain::createViews()
    {
        swapchainViews.resize(swapchainImages.size());

        for (std::size_t i = 0; i < swapchainViews.size(); i++)
        {
            VkImageViewCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swapchainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapchainFormat,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY},
                .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
            };

            if (vkCreateImageView(vulkan->get_device(), &createInfo, nullptr, &swapchainViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create Swapchain image view");
            }
        }
    }

    std::vector<Image> Swapchain::getSwapchainImages()
    {
        if (swapchainImageCache.size() == 0)
        {
            std::vector<Image> swapimages(swapchainImages.size());

            for (int i = 0; i < swapimages.size(); i++)
            {
                swapimages[i] = {
                    .image = swapchainImages[i],
                    .view = swapchainViews[i],
                    .dimensions = {swapchainExtent.width, swapchainExtent.height},
                    .swapchain = true};
            }
            swapchainImageCache = swapimages;
        }

        return swapchainImageCache;
    }

    void Swapchain::clearSwapchainViews()
    {
        for (auto view : swapchainViews)
            vkDestroyImageView(vulkan->get_device(), view, nullptr);

        swapchainViews.clear();
    }

    boitatah::SwapchainSupport Swapchain::getSwapchainSupport(VkPhysicalDevice physicalDevice)
    {
        SwapchainSupport support;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan->get_physical_device(), window->getSurface(), &support.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan->get_physical_device(), window->getSurface(), &formatCount, nullptr);

        if (formatCount != 0)
        {
            support.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan->get_physical_device(),
                                                 window->getSurface(),
                                                 &formatCount,
                                                 support.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan->get_physical_device(), window->getSurface(), &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            support.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan->get_physical_device(),
                                                      window->getSurface(),
                                                      &presentModeCount,
                                                      support.presentModes.data());
        }

        return support;
    }

    VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats, IMAGE_FORMAT scFormat, COLOR_SPACE scColorSpace)
    {
        for (const auto &surfaceFormat : availableFormats)
        {
            // GPUs usually display in BGRA [citation needed]
            if (surfaceFormat.format == boitatah::castEnum<VkFormat>(scFormat) &&
                surfaceFormat.colorSpace == boitatah::castEnum<VkColorSpaceKHR>(scColorSpace))
            {
                return surfaceFormat;
            }
        }
        throw std::runtime_error("Unable to select Swapchain format");
    }

    VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(window->window, &width, &height);

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
    VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availableModes)
    {
        for (const auto &mode : availableModes)
        {
            if (mode == boitatah::castEnum<VkPresentModeKHR>(FRAME_BUFFERING::TRIPLE_BUFFER))
            {
                return mode;
            }
        }

        return boitatah::castEnum<VkPresentModeKHR>(FRAME_BUFFERING::VSYNC);
    }
}
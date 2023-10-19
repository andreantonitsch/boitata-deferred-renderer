#include "Vulkan.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <algorithm>

namespace bvk = boitatah::vk;

bvk::Vulkan::Vulkan()
{
}

bvk::Vulkan::Vulkan(VulkanOptions opts)
{
    options = opts;
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
    std::vector<VkExtensionProperties> availableExtensions = retrieveAvailableExtensions();
    bool hasAllExtensions = checkRequiredExtensions(availableExtensions,
                                                    options.extensions,
                                                    options.extensionsCount);

    if (!hasAllExtensions)
        throw std::runtime_error("Vulkan does not have all required extensions available.");

    // Create Info
    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = options.extensionsCount,
        .ppEnabledExtensionNames = options.extensions,
    };

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan Instance.");
}

bvk::Vulkan::~Vulkan(void)
{
    vkDestroyInstance(instance, nullptr);
}

/// EXTENSIONS BLOCK
// TODO this can be better
//  Compares required extensions and available extensions
std::vector<VkExtensionProperties> bvk::Vulkan::retrieveAvailableExtensions()
{

    uint32_t extensionCount = 0;
    // call once to determine extension count
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    // create a vector and populate it with the extension properties.
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    return extensions;
}

// TODO slow comparison
bool bvk::Vulkan::checkRequiredExtensions(std::vector<VkExtensionProperties> available,
                                          const char **required,
                                          uint32_t requiredCount)
{
    std::vector<std::string> requiredExts(required, required + requiredCount);
    std::vector<std::string> availableExts;

    std::cout << requiredCount;

    for (size_t i = 0; i < (size_t)available.size(); i++)
    {
        availableExts.emplace_back(std::string(available[i].extensionName));
    }

    // Jank
    for (const auto& s : requiredExts)
    {
        
        if (std::find(availableExts.begin(),
                      availableExts.end(),
                      s) == availableExts.end())
            return false;
    }

    return true;
}

/// END EXTENSIONS BLOCK

// bvk::Vulkan &bvk::Vulkan::operator=(const Vulkan &v)
// {
//     return *this;
// }

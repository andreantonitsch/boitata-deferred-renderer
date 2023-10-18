#include "Vulkan.hpp"
#include <iostream>

namespace bvk = boitatah::vulkan;

bvk::Vulkan::Vulkan()
{
}

bvk::Vulkan::Vulkan(VulkanOptions options)
{

    //App Info
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = options.appName,
        .applicationVersion = VK_MAKE_VERSION(1,0,0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1,0,0),
        .apiVersion = VK_API_VERSION_1_0
        };

    //Create Info
    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = options.extensionsCount,
        .ppEnabledExtensionNames = options.extensions,
    };

    vkCreateInstance(&createInfo, nullptr, &instance);
}

bvk::Vulkan::~Vulkan(void)
{
    vkDestroyInstance(instance, nullptr);
}

// bvk::Vulkan &bvk::Vulkan::operator=(const Vulkan &v)
// {
//     return *this;
// }

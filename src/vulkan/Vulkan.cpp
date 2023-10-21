#include "Vulkan.hpp"
#include <iostream>
#include <stdexcept>
// #include <string>
#include <cstring>
#include <algorithm>
#include <map>
#include <optional>
#include <set>

namespace bvk = boitatah::vk;

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

/// DEBUG STUFF
// Vulkan callback function.
// DEBUG SEVERITY
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
    }

    initVkInstance();
    createSurface(opts.window);
    initPhysicalDevice();
    initLogicalDeviceNQueues();
}

bvk::Vulkan::~Vulkan(void)
{

    vkDestroyDevice(device, nullptr);

    if (options.useValidationLayers)
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

#pragma region QUEUE_SETUP
void boitatah::vk::Vulkan::createSurface(GLFWwindow *window)
{
    // TODO not ideal
    if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Window Surface");
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
            std::cout << "###DEBUG### \n"
                      << "Picked physcal device " << deviceProps.deviceName << "\n"
                      << "Device type " << deviceProps.deviceType << "\n"
                      << "Device ID " << deviceProps.deviceID << "\n"
                      << "###DEBUG###" << std::endl;
        }
    }
    else
    {
        std::runtime_error("No available GPUs with Vulkan Support");
    }
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

        if(presentSupport)
            queueFamilies.presentFamily = i;

        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamilies.graphicsFamily = i;
        }
        i++;
    }
    return queueFamilies;
}

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
        .enabledExtensionCount = 0,
        .pEnabledFeatures = &deviceFeatures};

    if (options.useValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to initialize a Logical Device");
    }

    vkGetDeviceQueue(device, familyIndices.graphicsFamily.value(), 0, &graphicsQueue);
}

#pragma endregion DEVICE_SETUP

#pragma region EXTENSIONS
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

void boitatah::vk::Vulkan::initVkInstance()
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
    std::vector<VkExtensionProperties> availableExtensions = retrieveAvailableExtensions();
    bool hasAllExtensions = checkRequiredExtensions(availableExtensions,
                                                    options.extensions);

    if (!hasAllExtensions)
        throw std::runtime_error("Vulkan does not have all required extensions available.");

    // Create Info
    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = (uint32_t)(options.extensions.size()),
        .ppEnabledExtensionNames = options.extensions.data(),
    };

    // const std::vector<const char *> validationLayers = {
    //     "VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    // Not happy about this vector here.
    //  checks for availability and initializes.
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
    if (options.useValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateMessenger(createInfo);

    if (createDebugUtilsMessengerEXT(instance,
                                     &createInfo,
                                     nullptr,
                                     &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Debug Messenger for validation Layers");
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
}

int boitatah::vk::Vulkan::evaluatePhysicalDevice(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(device, &deviceProps);

    VkPhysicalDeviceFeatures deviceFeats;
    vkGetPhysicalDeviceFeatures(device, &deviceFeats);

    return ((deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) * 1000 +
            deviceProps.limits.maxImageDimension2D) *
           (deviceFeats.geometryShader != 0) *
           (findQueueFamilies(device).graphicsFamily.has_value() != 0) // has graphics family queues
        ;
}

#pragma endregion VALIDATION

// bvk::Vulkan &bvk::Vulkan::operator=(const Vulkan &v)
// {
//     return *this;
// }

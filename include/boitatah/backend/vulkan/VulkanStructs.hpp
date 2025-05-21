#pragma once
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <cstring>
#include <optional>
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>

#include <boitatah/BoitatahEnums.hpp>

#include "CommandsVk.hpp"


namespace boitatah::vk{

    struct BufferDescVk
    {
        // uint32_t alignment;
        uint32_t size;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };

    struct BufferVkData{
        VkBuffer buffer;
        VkDeviceMemory memory;
        uint64_t alignment;
        uint64_t actualSize;

        uint32_t memoryTypeBits;
    };

    struct CommandPools
    {
        VkCommandPool graphicsPool;
        VkCommandPool transferPool;
        VkCommandPool presentPool;
    };

    struct CommandQueues
    {
        VkQueue graphicsQueue;
        VkQueue transferQueue;
        VkQueue presentQueue;
    };

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;

        bool hasFullSupport()
        {
            return graphicsFamily.has_value() &&
                   presentFamily.has_value() &&
                   transferFamily.has_value();
        }
    };

    struct SwapchainSupport
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct VulkanOptions
    {
        char *appName = nullptr;
        std::vector<const char *> extensions;
        bool useValidationLayers = false;
        bool debugMessages = false;

        // TODO glfw requires instance to create a surface.
        // I would like to keep both things separate.
        GLFWwindow *window;
    };
};
   
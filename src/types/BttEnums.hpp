#ifndef BOITATAH_BTT_ENUMS_HPP
#define BOITATAH_BTT_ENUMS_HPP

#include <vulkan/vulkan.h>
namespace boitatah
{

    enum class COLOR_SPACE
    {
        SRGB_NON_LINEAR = 1,
        LINEAR = 2,
    };

    enum class FORMAT
    {
        RGBA_8_SRGB = 1,
        BGRA_8_SRGB = 2,
        RGBA_8_UNORM = 3,
        BGRA_8_UNORM = 4,
    };

    enum class FRAME_BUFFERING
    {
        NO_BUFFER = 1,
        VSYNC = 2,
        TRIPLE_BUFFER = 3,
    };

    enum class SAMPLES
    {
        SAMPLES_1 = 1,
        SAMPLES_2 = 2,
        SAMPLES_4 = 3,
        SAMPLES_8 = 4,
        SAMPLES_16 = 5
    };

    enum class IMAGE_LAYOUT
    {
        UNDEFINED = 0,
        COLOR_ATT_OPTIMAL = 1,
        PRESENT_SRC = 2,
    };

    enum class USAGE
    {
        TRANSFER_SRC = 1,
        TRANSFER_DST = 2,
        COLOR_ATT = 3,
        DEPTH_STENCIL = 4,
        SAMPLED = 5,
        TRANSFER_DST_SAMPLED = 6,
        COLOR_ATT_TRANSFER_DST = 7,
        COLOR_ATT_TRANSFER_SRC = 8
    };

    enum class MEMORY_PROPERTY
    {
        DEVICE_LOCAL = 1,
        HOST_VISIBLE = 2,
        HOST_COHERENT = 3,
        HOST_CACHED = 4,
        LAZY_ALLOCATE = 5,
        HOST_VISIBLE_COHERENT = 6,

    };

    enum class COMMAND_BUFFER_LEVEL
    {
        PRIMARY = 1,
        SECONDARY = 2
    };

    enum class COMMAND_BUFFER_TYPE
    {
        GRAPHICS,
        TRANSFER,
        PRESENT
    };

    enum class ATTACHMENT_TYPE
    {
        COLOR,
        DEPTH_STENCIL,
        NORMAL,
    };

    template <typename From, typename To>
    static To castEnum(From from);

#pragma region Enum Specializations

    template <>
    inline VkFormat boitatah::castEnum(FORMAT format)
    {
        switch (format)
        {
        case FORMAT::RGBA_8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case FORMAT::BGRA_8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case FORMAT::RGBA_8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case FORMAT::BGRA_8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        default:
            return VK_FORMAT_UNDEFINED;
        }
    }
    template VkFormat boitatah::castEnum<FORMAT, VkFormat>(FORMAT);

    template <>
    inline VkColorSpaceKHR boitatah::castEnum(COLOR_SPACE colorSpace)
    {
        switch (colorSpace)
        {
        case COLOR_SPACE::SRGB_NON_LINEAR:
            return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        default:
            return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
    }
    template VkColorSpaceKHR boitatah::castEnum<COLOR_SPACE, VkColorSpaceKHR>(COLOR_SPACE);

    template <>
    inline VkPresentModeKHR boitatah::castEnum(FRAME_BUFFERING buffering_mode)
    {
        switch (buffering_mode)
        {
        case FRAME_BUFFERING::NO_BUFFER:
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case FRAME_BUFFERING::VSYNC:
            return VK_PRESENT_MODE_FIFO_KHR;
        case FRAME_BUFFERING::TRIPLE_BUFFER:
            return VK_PRESENT_MODE_MAILBOX_KHR;
        default:
            return VK_PRESENT_MODE_FIFO_KHR;
        }
    }
    template VkPresentModeKHR boitatah::castEnum<FRAME_BUFFERING, VkPresentModeKHR>(FRAME_BUFFERING);

    template <>
    inline VkImageLayout boitatah::castEnum(IMAGE_LAYOUT imageLayout)
    {
        switch (imageLayout)
        {
        case IMAGE_LAYOUT::COLOR_ATT_OPTIMAL:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case IMAGE_LAYOUT::PRESENT_SRC:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case IMAGE_LAYOUT::UNDEFINED:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        default:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }
    template VkImageLayout boitatah::castEnum<IMAGE_LAYOUT, VkImageLayout>(IMAGE_LAYOUT);

    template <>
    inline VkSampleCountFlagBits boitatah::castEnum(SAMPLES samples)
    {
        switch (samples)
        {
        case SAMPLES::SAMPLES_1:
            return VK_SAMPLE_COUNT_1_BIT;
        case SAMPLES::SAMPLES_2:
            return VK_SAMPLE_COUNT_2_BIT;
        case SAMPLES::SAMPLES_4:
            return VK_SAMPLE_COUNT_4_BIT;
        case SAMPLES::SAMPLES_8:
            return VK_SAMPLE_COUNT_8_BIT;
        case SAMPLES::SAMPLES_16:
            return VK_SAMPLE_COUNT_16_BIT;
        default:
            return VK_SAMPLE_COUNT_1_BIT;
        }
    }
    template VkSampleCountFlagBits boitatah::castEnum<SAMPLES, VkSampleCountFlagBits>(SAMPLES);

    template <>
    inline VkImageUsageFlagBits boitatah::castEnum(USAGE samples)
    {
        switch (samples)
        {
        case USAGE::TRANSFER_SRC:
            return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        case USAGE::TRANSFER_DST:
            return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        case USAGE::COLOR_ATT:
            return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        case USAGE::TRANSFER_DST_SAMPLED:
            return (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        case USAGE::SAMPLED:
            return VK_IMAGE_USAGE_SAMPLED_BIT;
        case USAGE::COLOR_ATT_TRANSFER_DST:
            return (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        case USAGE::COLOR_ATT_TRANSFER_SRC:
            return (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        default:
            return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }
    template VkImageUsageFlagBits boitatah::castEnum<USAGE, VkImageUsageFlagBits>(USAGE);

    template <>
    inline VkMemoryPropertyFlagBits boitatah::castEnum(MEMORY_PROPERTY properties)
    {
        switch (properties)
        {
        case MEMORY_PROPERTY::DEVICE_LOCAL:
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        case MEMORY_PROPERTY::HOST_VISIBLE:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        case MEMORY_PROPERTY::HOST_COHERENT:
            return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        case MEMORY_PROPERTY::HOST_CACHED:
            return VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        case MEMORY_PROPERTY::LAZY_ALLOCATE:
            return VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
        case MEMORY_PROPERTY::HOST_VISIBLE_COHERENT:
            return (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        default:
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        }
    }
    template VkMemoryPropertyFlagBits boitatah::castEnum<MEMORY_PROPERTY, VkMemoryPropertyFlagBits>(MEMORY_PROPERTY);

    template <>
    inline VkCommandBufferLevel boitatah::castEnum(COMMAND_BUFFER_LEVEL properties)
    {
        switch (properties)
        {
        case COMMAND_BUFFER_LEVEL::PRIMARY:
            return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        case COMMAND_BUFFER_LEVEL::SECONDARY:
            return VK_COMMAND_BUFFER_LEVEL_SECONDARY;

        default:
            return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        }
    }
    template VkCommandBufferLevel boitatah::castEnum<COMMAND_BUFFER_LEVEL, VkCommandBufferLevel>(COMMAND_BUFFER_LEVEL);

#pragma endregion Enum Specializations

}

#endif // BOITATAH_BTT_ENUMS_HPP
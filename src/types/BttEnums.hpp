#ifndef BOITATAH_BTT_ENUMS_HPP
#define BOITATAH_BTT_ENUMS_HPP

#include <vulkan/vulkan.h>
namespace boitatah
{

/// CONVERSION BETWEEN BOITATAH ENUMS AND VK ENUMS


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
        R_32_SFLOAT = 5,
        RG_32_SFLOAT = 6,
        RGB_32_SFLOAT = 7,
        RGBA_32_SFLOAT = 8,
        R_32_SINT = 9,
        RG_32_SINT = 10,
        RGB_32_SINT = 11,
        RGBA_32_SINT = 12,
        R_32_UINT = 13,
        RG_32_UINT = 14,
        RGB_32_UINT = 15,
        RGBA_32_UINT = 16,
        R_64_SFLOAT = 17,
        RG_64_SFLOAT = 18,
        RGB_64_SFLOAT = 19,
        RGBA_64_SFLOAT = 20,
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

    enum class IMAGE_USAGE
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

    enum class BUFFER_USAGE
    {
        VERTEX = 1,
        INDEX = 2,
        TRANSFER_SRC = 3,
        TRANSFER_DST = 4,
        TRANSFER_DST_VERTEX = 5,
        TRANSFER_DST_INDEX = 6,

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

    enum class SHARING_MODE
    {
        EXCLUSIVE = 1,
        CONCURRENT = 2,
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
        case FORMAT::R_32_SFLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case FORMAT::RG_32_SFLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case FORMAT::RGB_32_SFLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case FORMAT::RGBA_32_SFLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case FORMAT::R_32_SINT:
            return VK_FORMAT_R32_SINT;
        case FORMAT::RG_32_SINT:
            return VK_FORMAT_R32G32_SINT;
        case FORMAT::RGB_32_SINT:
            return VK_FORMAT_R32G32B32_SINT;
        case FORMAT::RGBA_32_SINT:
            return VK_FORMAT_R32G32B32A32_SINT;
        case FORMAT::R_32_UINT:
            return VK_FORMAT_R32_UINT;
        case FORMAT::RG_32_UINT:
            return VK_FORMAT_R32G32_UINT;
        case FORMAT::RGB_32_UINT:
            return VK_FORMAT_R32G32B32_UINT;
        case FORMAT::RGBA_32_UINT:
            return VK_FORMAT_R32G32B32A32_UINT;
        case FORMAT::R_64_SFLOAT:
            return VK_FORMAT_R64_SFLOAT;
        case FORMAT::RG_64_SFLOAT:
            return VK_FORMAT_R64G64_SFLOAT;
        case FORMAT::RGB_64_SFLOAT:
            return VK_FORMAT_R64G64B64_SFLOAT;
        case FORMAT::RGBA_64_SFLOAT:
            return VK_FORMAT_R64G64B64A64_SFLOAT;
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
    inline VkImageUsageFlagBits boitatah::castEnum(IMAGE_USAGE samples)
    {
        switch (samples)
        {
        case IMAGE_USAGE::TRANSFER_SRC:
            return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        case IMAGE_USAGE::TRANSFER_DST:
            return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        case IMAGE_USAGE::COLOR_ATT:
            return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        case IMAGE_USAGE::TRANSFER_DST_SAMPLED:
            return (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        case IMAGE_USAGE::SAMPLED:
            return VK_IMAGE_USAGE_SAMPLED_BIT;
        case IMAGE_USAGE::COLOR_ATT_TRANSFER_DST:
            return (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        case IMAGE_USAGE::COLOR_ATT_TRANSFER_SRC:
            return (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        default:
            return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }
    template VkImageUsageFlagBits boitatah::castEnum<IMAGE_USAGE, VkImageUsageFlagBits>(IMAGE_USAGE);

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

    template <>
    inline VkSharingMode boitatah::castEnum(SHARING_MODE properties)
    {
        switch (properties)
        {
        case SHARING_MODE::EXCLUSIVE:
            return VK_SHARING_MODE_EXCLUSIVE;
        case SHARING_MODE::CONCURRENT:
            return VK_SHARING_MODE_CONCURRENT;

        default:
            return VK_SHARING_MODE_EXCLUSIVE;
        }
    }
    template VkSharingMode boitatah::castEnum<SHARING_MODE, VkSharingMode>(SHARING_MODE);

    template <>
    inline VkBufferUsageFlags boitatah::castEnum(BUFFER_USAGE usage)
    {
        switch (usage)
        {
        case BUFFER_USAGE::VERTEX:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BUFFER_USAGE::INDEX:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BUFFER_USAGE::TRANSFER_DST:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        case BUFFER_USAGE::TRANSFER_SRC:
            return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        case BUFFER_USAGE::TRANSFER_DST_VERTEX:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BUFFER_USAGE::TRANSFER_DST_INDEX:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        default:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
    }
    template VkBufferUsageFlags boitatah::castEnum<BUFFER_USAGE, VkBufferUsageFlags>(BUFFER_USAGE MODE);

#pragma endregion Enum Specializations

    static uint32_t formatSize(FORMAT format);
    inline uint32_t formatSize(FORMAT format)
    {
        switch (format)
        {
        case FORMAT::RGBA_8_SRGB:
            return 4;
        case FORMAT::BGRA_8_SRGB:
            return 4;
        case FORMAT::RGBA_8_UNORM:
            return 4;
        case FORMAT::BGRA_8_UNORM:
            return 4;
        case FORMAT::R_32_SFLOAT:
            return 4;
        case FORMAT::RG_32_SFLOAT:
            return 8;
        case FORMAT::RGB_32_SFLOAT:
            return 12;
        case FORMAT::RGBA_32_SFLOAT:
            return 16;
        case FORMAT::R_32_SINT:
            return 4;
        case FORMAT::RG_32_SINT:
            return 8;
        case FORMAT::RGB_32_SINT:
            return 12;
        case FORMAT::RGBA_32_SINT:
            return 16;
        case FORMAT::R_32_UINT:
            return 4;
        case FORMAT::RG_32_UINT:
            return 8;
        case FORMAT::RGB_32_UINT:
            return 12;
        case FORMAT::RGBA_32_UINT:
            return 16;
        case FORMAT::R_64_SFLOAT:
            return 8;
        case FORMAT::RG_64_SFLOAT:
            return 16;
        case FORMAT::RGB_64_SFLOAT:
            return 24;
        case FORMAT::RGBA_64_SFLOAT:
            return 32;
        default:
            return 0;
        }
    }
}

#endif // BOITATAH_BTT_ENUMS_HPP
#pragma once

#include <vulkan/vulkan.h>
namespace boitatah
{

    template<class T>
        T& as_lvalue(T&& t){
        return static_cast<T&>(t);
    }

    /// CONVERSION BETWEEN BOITATAH ENUMS AND VK ENUMS

    enum class COLOR_SPACE
    {
        SRGB_NON_LINEAR = 1,
        LINEAR          = 2,
    };

    enum class IMAGE_FORMAT
    {
        RGBA_8_SRGB         = 1,
        BGRA_8_SRGB         = 2,
        RGBA_8_UNORM        = 3,
        BGRA_8_UNORM        = 4,
        R_32_SFLOAT         = 5,
        RG_32_SFLOAT        = 6,
        RGB_32_SFLOAT       = 7,
        RGBA_32_SFLOAT      = 8,
        R_32_SINT           = 9,
        RG_32_SINT          = 10,
        RGB_32_SINT         = 11,
        RGBA_32_SINT        = 12,
        R_32_UINT           = 13,
        RG_32_UINT          = 14,
        RGB_32_UINT         = 15,
        RGBA_32_UINT        = 16,
        R_64_SFLOAT         = 17,
        RG_64_SFLOAT        = 18,
        RGB_64_SFLOAT       = 19,
        RGBA_64_SFLOAT      = 20,
        DEPTH_32_SFLOAT     = 21,
        DEPTH_32_SFLOAT_UINT_STENCIL    = 22,
        DEPTH_24_UNORM_UINT_STENCIL     = 23,

    };

    enum class FRAME_BUFFERING
    {
        NO_BUFFER       = 1,
        VSYNC           = 2,
        TRIPLE_BUFFER   = 3,
    };

    enum class SAMPLES
    {
        SAMPLES_1       = 1,
        SAMPLES_2       = 2,
        SAMPLES_4       = 3,
        SAMPLES_8       = 4,
        SAMPLES_16      = 5
    };

    enum class IMAGE_LAYOUT
    {
        UNDEFINED           = 0,
        COLOR_ATT           = 1,
        PRESENT_SRC         = 2,
        WRITE               = 3,
        READ                = 4,
        TRANSFER_DST        = 5,
        DEPTH_STENCIL_ATT   = 6,
    };

    enum class IMAGE_USAGE
    {
        TRANSFER_SRC            = 1,
        TRANSFER_DST            = 2,
        COLOR_ATT               = 3,
        DEPTH_STENCIL           = 4,
        SAMPLED                 = 5,
        TRANSFER_DST_SAMPLED    = 6,
        COLOR_ATT_TRANSFER_DST  = 7,
        COLOR_ATT_TRANSFER_SRC  = 8,
        RENDER_GRAPH_COLOR      = 9,
        RENDER_GRAPH_DEPTH      = 10,

    };

    enum class BUFFER_USAGE
    {
        VERTEX                  = 1,
        INDEX                   = 2,
        TRANSFER_SRC            = 3,
        TRANSFER_DST            = 4,
        UNIFORM_BUFFER          = 5,

    };

    enum class MEMORY_PROPERTY
    {
        DEVICE_LOCAL            = 1,
        HOST_VISIBLE            = 2,
        HOST_COHERENT           = 3,
        HOST_CACHED             = 4,
        LAZY_ALLOCATE           = 5,
        HOST_VISIBLE_COHERENT   = 6,

    };

    enum class COMMAND_BUFFER_LEVEL
    {
        PRIMARY     = 1,
        SECONDARY   = 2
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
        POSITION,
        NORMAL,
        DEPTH_STENCIL,
    };

    enum class SHARING_MODE 
    {
        EXCLUSIVE   = 1,
        CONCURRENT  = 2,
    };

    //TODO standardize
    enum class DESCRIPTOR_TYPE : uint32_t
    {
        UNIFORM_BUFFER          = 0U,
        IMAGE                   = 1U,
        SAMPLER                 = 2U,
        COMBINED_IMAGE_SAMPLER  = 3U,
    };

    enum class STAGE_FLAG
    {
        VERTEX          = 1,
        FRAGMENT        = 2,
        VERTEX_FRAGMENT = 3,
        ALL_GRAPHICS    = 4
    };

    enum class SAMPLER_TILE_MODE{
        REPEAT,
        MIRRORED_REPEAT,
        CLAMP_EDGE,
        MIRRORED_CLAMP_EDGE,
        CLAMP_BORDER
    };

    enum class SAMPLER_MIPMAP_MODE{
        NEAREST,
        LINEAR
    };

    enum class FILTER{
        NEAREST,
        LINEAR,
        CUBIC,
    };

    template <typename To, typename From>
    static To castEnum(From from);

#pragma region Enum Specializations

    template<>
    inline VkFilter boitatah::castEnum(FILTER filter){
        switch(filter){
            case FILTER::NEAREST: return VK_FILTER_NEAREST;
            case FILTER::LINEAR: return VK_FILTER_LINEAR;
            case FILTER::CUBIC: return VK_FILTER_CUBIC_IMG;
            default: return VK_FILTER_NEAREST;
        }
    }
    template VkFilter boitatah::castEnum<VkFilter, FILTER>(FILTER);
    
    template<>
    inline VkSamplerMipmapMode boitatah::castEnum(SAMPLER_MIPMAP_MODE mode){
        switch(mode){
            case SAMPLER_MIPMAP_MODE::LINEAR:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            case SAMPLER_MIPMAP_MODE::NEAREST:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;
            default:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
    }
    template VkSamplerMipmapMode boitatah::castEnum<VkSamplerMipmapMode, SAMPLER_MIPMAP_MODE>(SAMPLER_MIPMAP_MODE);

    template<>
    inline VkSamplerAddressMode boitatah::castEnum(SAMPLER_TILE_MODE mode){
        switch(mode){
                case SAMPLER_TILE_MODE::REPEAT:
                     return VK_SAMPLER_ADDRESS_MODE_REPEAT;

                case SAMPLER_TILE_MODE::MIRRORED_REPEAT:
                     return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

                case SAMPLER_TILE_MODE::CLAMP_EDGE:
                     return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

                case SAMPLER_TILE_MODE::MIRRORED_CLAMP_EDGE:
                     return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
                     
                case SAMPLER_TILE_MODE::CLAMP_BORDER:
                     return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                default:
                    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }
    template VkSamplerAddressMode boitatah::castEnum<VkSamplerAddressMode, SAMPLER_TILE_MODE>(SAMPLER_TILE_MODE);


    template <>
    inline VkDescriptorType boitatah::castEnum(DESCRIPTOR_TYPE format)
    {
        switch (format)
        {
        case DESCRIPTOR_TYPE::UNIFORM_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DESCRIPTOR_TYPE::IMAGE:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DESCRIPTOR_TYPE::SAMPLER:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DESCRIPTOR_TYPE::COMBINED_IMAGE_SAMPLER:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        default:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
    }
    template VkDescriptorType boitatah::castEnum<VkDescriptorType, DESCRIPTOR_TYPE>(DESCRIPTOR_TYPE);

    template <>
    inline VkFormat boitatah::castEnum(IMAGE_FORMAT format)
    {
        switch (format)
        {
        case IMAGE_FORMAT::RGBA_8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;

        case IMAGE_FORMAT::BGRA_8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;

        case IMAGE_FORMAT::RGBA_8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;

        case IMAGE_FORMAT::BGRA_8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;

        case IMAGE_FORMAT::R_32_SFLOAT:
            return VK_FORMAT_R32_SFLOAT;
            
        case IMAGE_FORMAT::RG_32_SFLOAT:
            return VK_FORMAT_R32G32_SFLOAT;

        case IMAGE_FORMAT::RGB_32_SFLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;

        case IMAGE_FORMAT::RGBA_32_SFLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;

        case IMAGE_FORMAT::R_32_SINT:
            return VK_FORMAT_R32_SINT;

        case IMAGE_FORMAT::RG_32_SINT:
            return VK_FORMAT_R32G32_SINT;

        case IMAGE_FORMAT::RGB_32_SINT:
            return VK_FORMAT_R32G32B32_SINT;

        case IMAGE_FORMAT::RGBA_32_SINT:
            return VK_FORMAT_R32G32B32A32_SINT;

        case IMAGE_FORMAT::R_32_UINT:
            return VK_FORMAT_R32_UINT;

        case IMAGE_FORMAT::RG_32_UINT:
            return VK_FORMAT_R32G32_UINT;

        case IMAGE_FORMAT::RGB_32_UINT:
            return VK_FORMAT_R32G32B32_UINT;

        case IMAGE_FORMAT::RGBA_32_UINT:
            return VK_FORMAT_R32G32B32A32_UINT;

        case IMAGE_FORMAT::R_64_SFLOAT:
            return VK_FORMAT_R64_SFLOAT;

        case IMAGE_FORMAT::RG_64_SFLOAT:
            return VK_FORMAT_R64G64_SFLOAT;

        case IMAGE_FORMAT::RGB_64_SFLOAT:
            return VK_FORMAT_R64G64B64_SFLOAT;

        case IMAGE_FORMAT::RGBA_64_SFLOAT:
            return VK_FORMAT_R64G64B64A64_SFLOAT;
        
        case IMAGE_FORMAT::DEPTH_32_SFLOAT:
            return VK_FORMAT_D32_SFLOAT;

        case IMAGE_FORMAT::DEPTH_32_SFLOAT_UINT_STENCIL:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;

        case IMAGE_FORMAT::DEPTH_24_UNORM_UINT_STENCIL:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        
        default:
            return VK_FORMAT_UNDEFINED;
        }
    }
    template VkFormat boitatah::castEnum<VkFormat, IMAGE_FORMAT>(IMAGE_FORMAT);

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
    template VkColorSpaceKHR boitatah::castEnum<VkColorSpaceKHR, COLOR_SPACE>(COLOR_SPACE);

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
    template VkPresentModeKHR boitatah::castEnum<VkPresentModeKHR, FRAME_BUFFERING>(FRAME_BUFFERING);

    template <>
    inline VkImageLayout boitatah::castEnum(IMAGE_LAYOUT imageLayout)
    {
        switch (imageLayout)
        {
        case IMAGE_LAYOUT::COLOR_ATT:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case IMAGE_LAYOUT::PRESENT_SRC:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case IMAGE_LAYOUT::WRITE:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; 
        case IMAGE_LAYOUT::READ:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case IMAGE_LAYOUT::UNDEFINED:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case IMAGE_LAYOUT::DEPTH_STENCIL_ATT:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        default:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }
    template VkImageLayout boitatah::castEnum<VkImageLayout, IMAGE_LAYOUT>(IMAGE_LAYOUT);

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
    template VkSampleCountFlagBits boitatah::castEnum<VkSampleCountFlagBits, SAMPLES>(SAMPLES);

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
        
        case IMAGE_USAGE::DEPTH_STENCIL:
            return (VkImageUsageFlagBits) VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        
        case IMAGE_USAGE::RENDER_GRAPH_COLOR:
            return(VkImageUsageFlagBits)(   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                            VK_IMAGE_USAGE_SAMPLED_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

        case IMAGE_USAGE::RENDER_GRAPH_DEPTH:
            return(VkImageUsageFlagBits)(VK_IMAGE_USAGE_SAMPLED_BIT |
                                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

        default:
            return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }
    template VkImageUsageFlagBits boitatah::castEnum<VkImageUsageFlagBits, IMAGE_USAGE>(IMAGE_USAGE);

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
    template VkMemoryPropertyFlagBits boitatah::castEnum<VkMemoryPropertyFlagBits, MEMORY_PROPERTY>(MEMORY_PROPERTY);

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
    template VkCommandBufferLevel boitatah::castEnum<VkCommandBufferLevel, COMMAND_BUFFER_LEVEL>(COMMAND_BUFFER_LEVEL);

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
    template VkSharingMode boitatah::castEnum<VkSharingMode, SHARING_MODE>(SHARING_MODE);

    template <>
    inline VkBufferUsageFlags boitatah::castEnum(BUFFER_USAGE usage)
    {
        switch (usage)
        {
        case BUFFER_USAGE::TRANSFER_DST:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        case BUFFER_USAGE::TRANSFER_SRC:
            return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        case BUFFER_USAGE::VERTEX:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BUFFER_USAGE::INDEX:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BUFFER_USAGE::UNIFORM_BUFFER:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        default:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }
    }
    template VkBufferUsageFlags boitatah::castEnum<VkBufferUsageFlags, BUFFER_USAGE>(BUFFER_USAGE MODE);

    template <>
    inline VkShaderStageFlags boitatah::castEnum(STAGE_FLAG stages)
    {
        switch (stages)
        {
        case STAGE_FLAG::VERTEX:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case STAGE_FLAG::FRAGMENT:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case STAGE_FLAG::VERTEX_FRAGMENT:
            return VK_SHADER_STAGE_VERTEX_BIT ||VK_SHADER_STAGE_FRAGMENT_BIT;
        case STAGE_FLAG::ALL_GRAPHICS:
            return VK_SHADER_STAGE_ALL_GRAPHICS;
        default:
            return VK_SHADER_STAGE_ALL_GRAPHICS;
        }
    }
    template VkShaderStageFlags boitatah::castEnum<VkShaderStageFlags, STAGE_FLAG>(STAGE_FLAG MODE);


#pragma endregion Enum Specializations

    static uint32_t formatSize(IMAGE_FORMAT format);
    inline uint32_t formatSize(IMAGE_FORMAT format)
    {
        switch (format)
        {
        case IMAGE_FORMAT::RGBA_8_SRGB:
            return 4;
        case IMAGE_FORMAT::BGRA_8_SRGB:
            return 4;
        case IMAGE_FORMAT::RGBA_8_UNORM:
            return 4;
        case IMAGE_FORMAT::BGRA_8_UNORM:
            return 4;
        case IMAGE_FORMAT::R_32_SFLOAT:
            return 4;
        case IMAGE_FORMAT::RG_32_SFLOAT:
            return 8;
        case IMAGE_FORMAT::RGB_32_SFLOAT:
            return 12;
        case IMAGE_FORMAT::RGBA_32_SFLOAT:
            return 16;
        case IMAGE_FORMAT::R_32_SINT:
            return 4;
        case IMAGE_FORMAT::RG_32_SINT:
            return 8;
        case IMAGE_FORMAT::RGB_32_SINT:
            return 12;
        case IMAGE_FORMAT::RGBA_32_SINT:
            return 16;
        case IMAGE_FORMAT::R_32_UINT:
            return 4;
        case IMAGE_FORMAT::RG_32_UINT:
            return 8;
        case IMAGE_FORMAT::RGB_32_UINT:
            return 12;
        case IMAGE_FORMAT::RGBA_32_UINT:
            return 16;
        case IMAGE_FORMAT::R_64_SFLOAT:
            return 8;
        case IMAGE_FORMAT::RG_64_SFLOAT:
            return 16;
        case IMAGE_FORMAT::RGB_64_SFLOAT:
            return 24;
        case IMAGE_FORMAT::RGBA_64_SFLOAT:
            return 32;
        case IMAGE_FORMAT::DEPTH_32_SFLOAT:
            return 4;
        case IMAGE_FORMAT::DEPTH_32_SFLOAT_UINT_STENCIL:
            return 8;
        case IMAGE_FORMAT::DEPTH_24_UNORM_UINT_STENCIL:
            return 4;
        default:
            return 0;
        }
    }

}

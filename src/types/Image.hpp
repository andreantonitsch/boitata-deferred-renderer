#pragma once

#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include "BttEnums.hpp"
#include "../collections/Pool.hpp"

namespace boitatah
{

    //structs
    struct SamplerData{

        FILTER magFilter = FILTER::LINEAR;
        FILTER minFilter = FILTER::LINEAR;
        SAMPLER_MIPMAP_MODE mipmap = SAMPLER_MIPMAP_MODE::LINEAR;
        SAMPLER_TILE_MODE u_tiling = SAMPLER_TILE_MODE::REPEAT;
        SAMPLER_TILE_MODE v_tiling = SAMPLER_TILE_MODE::REPEAT;
        float lodBias = 0;
        bool normalized = true;
        bool anisotropy = false;
        float maxAnisotropy = 0.0f;
        uint32_t mipLevels = 1;
    };


    struct Sampler{
        SamplerData data;
        VkSampler sampler;
    };

    struct ImageDesc
    {
        IMAGE_FORMAT format;
        glm::u32vec2 dimensions;
        uint32_t mipLevels = 1;
        IMAGE_LAYOUT initialLayout;
        IMAGE_USAGE usage;
        SAMPLES samples = SAMPLES::SAMPLES_1;
        bool skip_view = false;
    };

    struct Image
    {
        VkImage image;
        VkImageView view;
        glm::u32vec2 dimensions;
        bool swapchain = false;
        VkDeviceMemory memory;
    };

    struct ImageAccessData{
        VkImageView view;
    };

    struct SamplerAccessData{
        VkSampler sampler;
    };

}


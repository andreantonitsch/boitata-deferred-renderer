#pragma once

#include <vector>
#include <span>

#include <boitatah/BoitatahEnums.hpp>
#include <boitatah/buffers/BufferStructs.hpp>
#include <boitatah/resources/TextureStructs.hpp>
#include <boitatah/types/Image.hpp>

namespace boitatah
{
    using namespace boitatah::buffer;

    struct BindingDesc{
        //uint32_t binding = 0;
        DESCRIPTOR_TYPE type;
        SHADER_STAGE stages;
        uint32_t descriptorCount = 1;
    };
    
    struct DescriptorSetLayoutDesc{
        std::vector<BindingDesc> bindingDescriptors;
    };

    struct DescriptorSetRatio{
        DESCRIPTOR_TYPE type;
        int16_t quantity;
    };

    struct DescriptorSetLayout
    {
        std::vector<DescriptorSetRatio> ratios;
        DescriptorSetLayoutDesc description;
        VkDescriptorSetLayout layout;
    };

    // TODO hide all this.
    struct DescriptorSet 
    {
        VkDescriptorSet descriptorSet;
    };


    struct DescriptorSetRequest{
        std::span<DescriptorSetRatio> layout;
    };
 
    struct BindBindingDesc{
        //uint32_t set;
        uint32_t binding;
        DESCRIPTOR_TYPE type;
        union{
            BufferAccessData bufferData;
            TextureAccessData textureData;
            ImageAccessData imageData;
            SamplerAccessData samplerData;
        }access;
    };

};




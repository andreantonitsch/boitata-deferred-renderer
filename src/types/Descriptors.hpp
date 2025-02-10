#pragma once

#include "BttEnums.hpp"
#include <vector>
#include <span>
//#include <renderer/resources/GPUBuffer.hpp>

namespace boitatah
{
    class GPUBuffer;

    struct BindingDesc{
        //uint32_t binding = 0;
        DESCRIPTOR_TYPE type;
        STAGE_FLAG stages;
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
        Handle<GPUBuffer> buffer;
    };

};




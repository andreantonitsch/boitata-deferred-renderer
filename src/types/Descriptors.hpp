#pragma once

#include "BttEnums.hpp"
#include <vector>
#include <span>

namespace boitatah
{
    struct BindingDesc{
        //uint32_t binding = 0;
        DESCRIPTOR_TYPE type;
        STAGE_FLAG stages;
        uint32_t descriptorCount = 1;
    };
    
    struct DescriptorSetLayoutDesc{
        uint32_t m_set = 1;
        std::vector<BindingDesc> bindingDescriptors;
    };

    struct DescriptorSetRatio{
        DESCRIPTOR_TYPE type;
        int16_t quantity;
    };

    struct DescriptorSetLayout
    {
        std::span<DescriptorSetRatio> ratios;
        VkDescriptorSetLayout layout;
    };



    // TODO hide all this.
    struct DescriptorSet 
    {
        VkDescriptorSet descriptorSet;
    };


    struct DescriptorSetRequest{
        DescriptorSetLayout layout;
    };
 
};




#pragma once

#include "../../vulkan/Vulkan.hpp"
#include "../../collections/Pool.hpp"
#include <vector>
#include <span>
#include "../../types/Descriptors.hpp"

namespace boitatah
{

    using std::size_t;
    using namespace vk;

    struct DescriptorSetPool
    {
        uint16_t bufferCapacity;
        uint16_t imageCapacity;
        uint16_t bufferOccupation;
        uint16_t imageOccupation;
        VkDescriptorPool pools[2];

        bool fits(const DescriptorSetLayout &request)
        {
            bool fit = true;

            for (auto ratio : request.ratios)
            {
                switch (ratio.type)
                {
                case DESCRIPTOR_TYPE::UNIFORM_BUFFER:
                    fit &= (bufferCapacity % ratio.quantity) == 0;
                    break;
                }
            }
            return fit;
        };

        VkDescriptorSet allocate(const DescriptorSetLayout &request, const uint32_t poolIndex, const Vulkan *vk)
        {
            VkDescriptorSet set;

            VkDescriptorSetAllocateInfo info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = pools[poolIndex],
                .descriptorSetCount = static_cast<uint32_t>(1),
                //.
            };
            return set;
        };

        void reset(const size_t poolIndex, const Vulkan *vk)
        {
            vkResetDescriptorPool(vk->getDevice(), pools[poolIndex], 0);
        };

        void release(Vulkan *vk)
        {
            vk->destroyDescriptorPool(pools[0]);
            vk->destroyDescriptorPool(pools[1]);
        };

        void initialize(const uint32_t maxSets, const std::span<DescriptorSetRatio> ratios, const Vulkan *vk)
        {

            // TODO move to vulkan class
            std::vector<VkDescriptorPoolSize> sizes;
            sizes.resize(ratios.size());

            for (size_t i = 0; i < sizes.size(); i++)
            {
                sizes[i].type = castEnum<VkDescriptorType>(ratios[i].type);
                sizes[i].descriptorCount = ratios[i].quantity * maxSets;
            }

            VkDescriptorPoolCreateInfo poolInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .maxSets = maxSets,
                .poolSizeCount = static_cast<uint32_t>(sizes.size()),
                .pPoolSizes = sizes.data()};

            for (size_t i = 0; i < 3; i++)
            {
                if (vkCreateDescriptorPool(vk->getDevice(), &poolInfo, nullptr, &(pools[i])) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to create descriptor pool");
                }
            }
        };
    };

    class DescriptorPoolManager
    {

    public:
        DescriptorPoolManager(Vulkan *vulkan, uint32_t maximumSets);
        ~DescriptorPoolManager();
        Handle<DescriptorSet> getSet(DescriptorSetLayout &request);

    private:
        Vulkan *m_vk;
        uint32_t maxSets = 4095;
        std::vector<DescriptorSetPool> currentPools;
        void createPool(DescriptorSetLayout &request);
        size_t findPool(DescriptorSetLayout &request);
        void releasePool(size_t index);

        // Members
        std::vector<DescriptorSetPool> m_pools;
    };
}


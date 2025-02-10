#pragma once

#include <vulkan/Vulkan.hpp>
#include <collections/Pool.hpp>
#include <vector>
#include <span>
#include <memory>
#include <types/Descriptors.hpp>
#include <algorithm>
#include <string>
#include <renderer/modules/GPUResourceManager.hpp>

namespace boitatah::vk
{

    using std::size_t;

    struct DescriptorSetPool
    {   
        template<typename T, int length, int width>
        using mat = std::array<std::array<T, length>, width>;
        private:
            mat<uint32_t, 10, 2> used_descriptors; // TODO number of descriptor types
            std::array<uint32_t, 10> set_capacity;
            std::array<VkDescriptorPool, 2> pools;

        public:

            DescriptorSetPool(const uint32_t maxSets, const std::vector<DescriptorSetRatio> ratios, std::shared_ptr<Vulkan> vk){
                // TODO move to vulkan class
                std::vector<VkDescriptorPoolSize> sizes;
                sizes.resize(ratios.size());
                std::cout << " creating pool with " << sizes.size() << " sizes" << std::endl;
                for (size_t i = 0; i < sizes.size(); i++)
                {
                    sizes[i].type = castEnum<VkDescriptorType>(ratios[i].type);
                    sizes[i].descriptorCount = ratios[i].quantity * maxSets;
                    auto capacity_idx = static_cast<uint32_t>(ratios[i].type);
                    std::cout << capacity_idx << std::endl;
                    set_capacity[capacity_idx] = ratios[i].quantity * maxSets;

                    used_descriptors[0][capacity_idx] = 0; 
                    used_descriptors[1][capacity_idx] = 0;
                } 
                std::cout << "created sizes vector" << std::endl;
                VkDescriptorPoolCreateInfo poolInfo{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                    .pNext = nullptr,
                    .maxSets = maxSets,
                    .poolSizeCount = static_cast<uint32_t>(sizes.size()),
                    .pPoolSizes = sizes.data(),
                    };
                
                std::cout << "created pool with " << ratios.size() << " ratio types" << std::endl;

                for (size_t i = 0; i  < 2; i++)
                {
                    
                    auto result = vkCreateDescriptorPool(vk->getDevice(), &poolInfo, nullptr, &(pools[i]));
                    if (result != VK_SUCCESS)
                    {
                        throw std::runtime_error("failed to create descriptor pool" + std::to_string(static_cast<int>(result)));
                    }
                }
            }

            bool fits(const DescriptorSetLayout &request, uint32_t pool_index)
            {
                bool fit = true;
                for (auto ratio : request.ratios)
                {
                    int type_idx = static_cast<uint32_t>(ratio.type);
                    fit &= (set_capacity[type_idx] % ratio.quantity) == 0;
                    int remaining = set_capacity[type_idx] - used_descriptors[pool_index%2][type_idx];
                    fit &= remaining > ratio.quantity;
                }
                return fit;
            };

            VkDescriptorSet allocate(const DescriptorSetLayout &request, const uint32_t poolIndex,std::shared_ptr<Vulkan> vk)
            {
                VkDescriptorSet set;

                VkDescriptorSetAllocateInfo info{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                    .pNext = nullptr,
                    .descriptorPool = pools[poolIndex%2],
                    .descriptorSetCount = static_cast<uint32_t>(1),
                    .pSetLayouts = &(request.layout),
                };

                for (auto ratio : request.ratios)
                {
                    used_descriptors[poolIndex%2][static_cast<uint32_t>(ratio.type)] +=  ratio.quantity;
                }
                auto result = vkAllocateDescriptorSets(vk->getDevice(), &info, &set);
                if( result != VK_SUCCESS)
                    throw std::runtime_error("failed to allocate descriptor set for Pool " + std::to_string(static_cast<int>(result)));
                
                return set;
            };

            void reset(const size_t poolIndex, std::shared_ptr<Vulkan> vk)
            {
                vkResetDescriptorPool(vk->getDevice(), pools[poolIndex%2], 0);
                used_descriptors[poolIndex%2].fill(0);
            };

            void release(std::shared_ptr<Vulkan> vk)
            {
                vk->destroyDescriptorPool(pools[0]);
                vk->destroyDescriptorPool(pools[1]);
            };

    };

    class DescriptorPoolManager
    {

    public:
        DescriptorPoolManager(std::shared_ptr<Vulkan> vulkan, std::shared_ptr<GPUResourceManager> resourceManager, uint32_t maximumSets);
        ~DescriptorPoolManager();
        DescriptorSet getSet(const DescriptorSetLayout &request, uint32_t frame_index);
        void writeSet(const std::span<const BindBindingDesc> &bindings, 
                      const DescriptorSet& set,
                      uint32_t frame_index);
        void bindSet(const CommandBuffer drawBuffer,
                     const ShaderLayout& layout,
                     const DescriptorSet& set,
                     uint32_t set_index, uint32_t frame_index);
        void resetPools(uint32_t frame_index);

    private:
        // Members
        std::shared_ptr<Vulkan> m_vk;
        std::shared_ptr<GPUResourceManager> m_resourceManager;
        uint32_t maxSets = 4096;
        std::vector<DescriptorSetPool> m_pools;

        size_t createPool(const DescriptorSetLayout &request);
        size_t findPool(const DescriptorSetLayout &request, uint32_t frame_index);
        DescriptorSetPool& findCreatePool(const DescriptorSetLayout &request, uint32_t frame_index);
        void releasePool(size_t index);

    };
}


#pragma once

#include <vector>
#include <span>
#include <memory>
#include <algorithm>
#include <string>

#include <boitatah/backend/vulkan/Vulkan.hpp>
#include <boitatah/collections.hpp>
#include <boitatah/types/Descriptors.hpp>
namespace boitatah::vk::descriptor_sets{ class DescriptorSetTree; }

namespace boitatah::vk
{
    using std::size_t;

    template<uint32_t FRAMES>
    struct DescriptorSetPool
    {   
        template<typename T, int length, int width>
        using mat = std::array<std::array<T, length>, width>;
        private:
            mat<uint32_t, 10, FRAMES> used_descriptors; // TODO number of descriptor types
            std::array<uint32_t, 10> set_capacity;
            std::array<VkDescriptorPool, FRAMES> pools;

        public:

            DescriptorSetPool(const uint32_t maxSets, const std::vector<DescriptorSetRatio> ratios, std::shared_ptr<VulkanInstance> vk){
                // TODO move to vulkan class
                std::vector<VkDescriptorPoolSize> sizes;
                sizes.resize(ratios.size());
                for (size_t i = 0; i < sizes.size(); i++)
                {
                    sizes[i].type = castEnum<VkDescriptorType>(ratios[i].type);
                    sizes[i].descriptorCount = ratios[i].quantity * maxSets;
                    auto capacity_idx = static_cast<uint32_t>(ratios[i].type);
                    std::cout << capacity_idx << std::endl;
                    set_capacity[capacity_idx] = ratios[i].quantity * maxSets;
                    
                    for(uint32_t i = 0; i < FRAMES; i++)
                        used_descriptors[i][capacity_idx] = 0;
                } 
                //std::cout << "created sizes vector" << std::endl;
                VkDescriptorPoolCreateInfo poolInfo{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                    .pNext = nullptr,
                    .maxSets = maxSets,
                    .poolSizeCount = static_cast<uint32_t>(sizes.size()),
                    .pPoolSizes = sizes.data(),
                    };
                
                //std::cout << "created pool with " << ratios.size() << " ratio types" << std::endl;

                for (size_t i = 0; i  < FRAMES; i++)
                {
                    
                    auto result = vkCreateDescriptorPool(vk->get_device(), &poolInfo, nullptr, &(pools[i]));
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
                    int remaining = set_capacity[type_idx] - used_descriptors[pool_index % FRAMES][type_idx];
                    fit &= remaining > ratio.quantity;
                }
                return fit;
            };

            VkDescriptorSet allocate(const DescriptorSetLayout &request, const uint32_t poolIndex,std::shared_ptr<VulkanInstance> vk)
            {
                VkDescriptorSet set;

                VkDescriptorSetAllocateInfo info{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                    .pNext = nullptr,
                    .descriptorPool = pools[poolIndex % FRAMES],
                    .descriptorSetCount = static_cast<uint32_t>(1),
                    .pSetLayouts = &(request.layout),
                };

                for (auto ratio : request.ratios)
                {
                    used_descriptors[poolIndex % FRAMES][static_cast<uint32_t>(ratio.type)] +=  ratio.quantity;
                }
                auto result = vkAllocateDescriptorSets(vk->get_device(), &info, &set);
                if( result != VK_SUCCESS)
                    throw std::runtime_error("failed to allocate descriptor set for Pool " + std::to_string(static_cast<int>(result)));
                
                return set;
            };

            void reset(const size_t poolIndex, std::shared_ptr<VulkanInstance> vk)
            {
                vkResetDescriptorPool(vk->get_device(), pools[poolIndex % FRAMES], 0);
                used_descriptors[poolIndex % FRAMES].fill(0);
            };

            void release(std::shared_ptr<VulkanInstance> vk)
            {                
                for(uint32_t i; i < FRAMES; i++)
                    vk->destroy_descriptorpool(pools[i]);
            };

    };

    class DescriptorSetManager
    {

    public:
        DescriptorSetManager(std::shared_ptr<VulkanInstance> vulkan, uint32_t maximumSets);
        ~DescriptorSetManager();
        Handle<DescriptorSetLayout> getLayout(const DescriptorSetLayoutDesc& description);
        DescriptorSetLayout& getLayoutContent(const Handle<DescriptorSetLayout>& handle);
        DescriptorSet getSet(const DescriptorSetLayout &request, uint32_t frame_index);
        void writeSet(const std::span<const BindBindingDesc> &bindings, 
                      const DescriptorSet& set,
                      uint32_t frame_index);
        void bindSet(const CommandBuffer drawBuffer,
                     const ShaderLayout& layout,
                     const DescriptorSet& set,
                     uint32_t set_index);
        void resetPools(uint32_t frame_index);

    private:
        // Members
        std::shared_ptr<VulkanInstance> m_vk;
        uint32_t maxSets = 4096;
        std::vector<DescriptorSetPool<3>> m_pools;
        std::unique_ptr<descriptor_sets::DescriptorSetTree> m_descriptorTree;

        // Handle<DescriptorSetLayout> createLayout(const DescriptorSetLayoutDesc& description);
        // DescriptorSetLayout findCreateLayout(const DescriptorSetLayoutDesc& description);
        
        size_t createPool(const DescriptorSetLayout &request);
        size_t findPool(const DescriptorSetLayout &request, uint32_t frame_index);
        DescriptorSetPool<3>& findCreatePool(const DescriptorSetLayout &request, uint32_t frame_index);
        void releasePool(size_t index);

    };
}


#include "DescriptorPoolManager.hpp"

namespace boitatah::vk {

    DescriptorPoolManager::DescriptorPoolManager(std::shared_ptr<Vulkan> vulkan, uint32_t maximumSets)
    : m_vk(vulkan), maxSets(maximumSets){};

    DescriptorPoolManager::~DescriptorPoolManager(){
        //release all pools
        while(m_pools.size() > 0){
            releasePool(m_pools.size()-1);
            m_pools.pop_back();
        }
    }

    DescriptorSet DescriptorPoolManager::getSet(const DescriptorSetLayout &request, uint32_t frame_index)
    {
        DescriptorSetPool pool = findCreatePool(request, frame_index);
        DescriptorSet set;
        set.descriptorSet =  pool.allocate(request, frame_index, m_vk);

        return set;
    }

    void DescriptorPoolManager::writeSet(const std::span<const BindBindingDesc> &bindings, 
                                        const DescriptorSet &set,
                                        uint32_t frame_index)
    {   
        std::vector<VkWriteDescriptorSet> writes;
        std::vector<VkDescriptorImageInfo> images;
        std::vector<VkDescriptorBufferInfo> buffers;
        std::vector<VkBufferView> views;
        
        for(auto& binding : bindings){
            VkWriteDescriptorSet write;
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.pNext = nullptr;
            write.descriptorCount = 1;
            write.dstSet = set.descriptorSet;
            write.dstBinding = binding.binding;
            write.descriptorType = castEnum<VkDescriptorType>(binding.type);
            write.dstArrayElement = 0U;
            switch(binding.type){
                case DESCRIPTOR_TYPE::UNIFORM_BUFFER:{
                    VkDescriptorBufferInfo info;
                    auto bufferAccess =  binding.bufferData;
                    info.buffer = bufferAccess.buffer->getBuffer();
                    info.offset = bufferAccess.offset;
                    info.range = bufferAccess.size;
                    buffers.push_back(info);
                    write.pBufferInfo = &buffers.back();
                    break;}
                default:
                    std::cout << "Trying to bind invalid descriptor type" << std::endl;
                    break;
            }

            writes.push_back(write);
        }

        vkUpdateDescriptorSets(m_vk->getDevice(), writes.size(), writes.data(), 0, nullptr);
    }

    void DescriptorPoolManager::bindSet(const CommandBuffer drawBuffer,
                                        const ShaderLayout &layout,
                                        const DescriptorSet &set, 
                                        uint32_t set_index, 
                                        uint32_t frame_index)
    {
        vkCmdBindDescriptorSets(drawBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                layout.pipeline, set_index, 1, &set.descriptorSet, 0, nullptr);
    }

    // void DescriptorPoolManager::bindSet(const const DescriptorSet &set, uint32_t set_index, uint32_t frame_index)
    // {
    //     vkCmdBindDescriptorSets()
    // }

    void DescriptorPoolManager::resetPools(uint32_t frame_index)
    {
        for(auto& pool : m_pools){
            pool.reset(frame_index, m_vk);
        }
    }

    size_t DescriptorPoolManager::createPool(const DescriptorSetLayout &request)
    {
        DescriptorSetPool pool(maxSets, request.ratios, m_vk);
        m_pools.push_back(pool);
        std::cout<< request.ratios.size() << std::endl;
        std::cout << "created set pool" << std::endl;
        return m_pools.size()-1;
    }

    size_t DescriptorPoolManager::findPool(const DescriptorSetLayout &request, uint32_t frame_index)
    {
        for(size_t i = 0; i < m_pools.size(); i++){
            if(m_pools[i].fits(request, frame_index))
                return i;
        }

        return UINT32_MAX;
    }
    
    DescriptorSetPool& DescriptorPoolManager::findCreatePool(const DescriptorSetLayout &request, uint32_t frame_index)
    {
        uint32_t pool_idx = findPool(request, frame_index);

        if(pool_idx == UINT32_MAX){
            createPool(request);
            pool_idx = findPool(request, frame_index);
        }
        return m_pools[pool_idx];
    }

    void DescriptorPoolManager::releasePool(size_t index)
    {
        m_pools[index].release(m_vk);  
    };

};

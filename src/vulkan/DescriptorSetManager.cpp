#include "DescriptorSetManager.hpp"

namespace boitatah::vk {

    DescriptorSetManager::DescriptorSetManager(std::shared_ptr<Vulkan> vulkan, uint32_t maximumSets)
    : m_vk(vulkan), maxSets(maximumSets), m_descriptorTree(std::make_unique<descriptor_sets::DescriptorSetTree>(vulkan)){};

    DescriptorSetManager::~DescriptorSetManager(){
        //release all pools
        while(m_pools.size() > 0){
            releasePool(m_pools.size()-1);
            m_pools.pop_back();
        }
    }

    Handle<DescriptorSetLayout> DescriptorSetManager::getLayout(const DescriptorSetLayoutDesc &description)
    {
        return m_descriptorTree->getSetLayout(description);
    }

    DescriptorSetLayout& DescriptorSetManager::getLayoutContent(const Handle<DescriptorSetLayout> &handle)
    {
        return m_descriptorTree->getSetLayoutData(handle);
    }

    DescriptorSet DescriptorSetManager::getSet(const DescriptorSetLayout &request, uint32_t frame_index)
    {
        DescriptorSetPool pool = findCreatePool(request, frame_index);
        DescriptorSet set;
        set.descriptorSet =  pool.allocate(request, frame_index, m_vk);

        return set;
    }

    void DescriptorSetManager::writeSet(const std::span<const BindBindingDesc> &bindings, 
                                        const DescriptorSet &set,
                                        uint32_t frame_index)
    {   
        //TODO adjust with std::any or something.
        // this will waste memory but preserve the pointers up to 10
        std::vector<VkWriteDescriptorSet> writes{};
        std::vector<VkDescriptorImageInfo> images(10);
        std::vector<VkDescriptorBufferInfo> buffers(10);
        std::vector<VkBufferView> views(10);
        uint32_t count = 0;
        for(auto& binding : bindings){
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorCount = 1;
            write.dstSet = set.descriptorSet;
            write.dstBinding = binding.binding;
            write.descriptorType = castEnum<VkDescriptorType>(binding.type);
            switch(binding.type){
                case DESCRIPTOR_TYPE::UNIFORM_BUFFER:{
                    VkDescriptorBufferInfo info{};
                    auto bufferAccess =  binding.access.bufferData;
                    info.buffer = bufferAccess.buffer->getBuffer();
                    info.offset = bufferAccess.offset;
                    info.range = bufferAccess.size;
                    buffers.push_back(info);
                    write.pBufferInfo = &buffers.back();
                    break;}

                case DESCRIPTOR_TYPE::COMBINED_IMAGE_SAMPLER:{
                    VkDescriptorImageInfo info4{};
                    auto textureAccess =  binding.access.textureData;
                    info4.imageLayout = textureAccess.layout;
                    info4.sampler = textureAccess.sampler;
                    info4.imageView = textureAccess.view;
                    images.push_back(info4);
                    write.pImageInfo = &images.back();
                    break;}

                case DESCRIPTOR_TYPE::IMAGE:{
                    VkDescriptorImageInfo info2{};
                    auto imageAccess =  binding.access.imageData;
                    info2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    info2.imageView = imageAccess.view;
                    images.push_back(info2);
                    write.pImageInfo = &(images.back());
                    std::cout << (write.pImageInfo)->imageLayout << std::endl;
                    break;}

                case DESCRIPTOR_TYPE::SAMPLER:{
                    VkDescriptorImageInfo info3{};
                    auto samplerAccess =  binding.access.samplerData;
                    info3.sampler = samplerAccess.sampler;
                    info3.imageView = VK_NULL_HANDLE;
                    images.push_back(info3);
                    write.pImageInfo = &images.back();
                    break;}

                default:
                    std::cout << "Trying to bind invalid descriptor type" << std::endl;
                    break;
            }
            writes.push_back(write);
        }


        vkUpdateDescriptorSets(m_vk->getDevice(), writes.size(), writes.data(), 0, nullptr);
    }

    void DescriptorSetManager::bindSet(const CommandBuffer drawBuffer,
                                        const ShaderLayout &layout,
                                        const DescriptorSet &set, 
                                        uint32_t set_index)
    {
        vkCmdBindDescriptorSets(drawBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                layout.pipeline, set_index, 1, &set.descriptorSet, 0, nullptr);
    }

    void DescriptorSetManager::resetPools(uint32_t frame_index)
    {
        for(auto& pool : m_pools){
            pool.reset(frame_index, m_vk);
        }
    }

    size_t DescriptorSetManager::createPool(const DescriptorSetLayout &request)
    {
        DescriptorSetPool<3> pool(maxSets, request.ratios, m_vk);
        m_pools.push_back(pool);
        return m_pools.size()-1;
    }

    size_t DescriptorSetManager::findPool(const DescriptorSetLayout &request, uint32_t frame_index)
    {
        for(size_t i = 0; i < m_pools.size(); i++){
            if(m_pools[i].fits(request, frame_index))
                return i;
        }

        return UINT32_MAX;
    }
    
    DescriptorSetPool<3>& DescriptorSetManager::findCreatePool(const DescriptorSetLayout &request, uint32_t frame_index)
    {
        uint32_t pool_idx = findPool(request, frame_index);

        if(pool_idx == UINT32_MAX){
            createPool(request);
            pool_idx = findPool(request, frame_index);
        }
        return m_pools[pool_idx];
    }

    void DescriptorSetManager::releasePool(size_t index)
    {
        m_pools[index].release(m_vk);  
    };

};

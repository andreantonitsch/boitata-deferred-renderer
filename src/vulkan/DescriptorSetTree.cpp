#include "DescriptorSetTree.hpp"
#include <algorithm>
namespace boitatah::vk::descriptor_sets{
    DescriptorSetTree::DescriptorSetTree(std::shared_ptr<Vulkan> vulkan)
    {
        m_vk =  vulkan;
        m_layoutPool = std::unique_ptr<Pool<DescriptorSetLayout>>(
            new Pool<DescriptorSetLayout>(PoolOptions{
                .size = 100,
                .dynamic = true,
                .name = "descriptor layout pool"
            })
        );

        m_nodes = std::make_unique<DescriptorSetTreeNode>();
    }

    Handle<DescriptorSetLayout> DescriptorSetTree::createSetLayout(DescriptorSetLayoutDesc &description)
    {
        auto binds = std::vector<BindingDesc>(description.bindingDescriptors);
        std::reverse(binds.begin(), binds.end());

        auto& node = m_nodes->findNode(binds);
        
        DescriptorSetLayout layout;
        //std::vector<DescriptorSetRatio> ratios;
        std::vector<BindingDesc> bindingDesc;
        layout.layout = m_vk->createDescriptorLayout(description);

        for(auto& bindDesc : description.bindingDescriptors){
            for (std::size_t i = 0; i <= layout.ratios.size(); i++)
            {
                if(i == layout.ratios.size()){
                    DescriptorSetRatio ratio;
                    ratio.type = bindDesc.type;
                    ratio.quantity = bindDesc.descriptorCount;
                    layout.ratios.push_back(ratio);
                    break;
                }
                if(layout.ratios[i].type == bindDesc.type)
                {
                    layout.ratios[i].quantity += bindDesc.descriptorCount;
                    break;
                }
            }            
        }
        auto layout_handle = m_layoutPool->set(layout);
        return layout_handle;
    }

    Handle<DescriptorSetLayout> DescriptorSetTree::getSetLayout(DescriptorSetLayoutDesc &description, 
                                                                std::vector<BindingDesc> &binds)
    {
        auto& node = m_nodes->findNode(binds);
        return node.getSetLayout(description, *this);
    }

    Handle<DescriptorSetLayout> DescriptorSetTree::getSetLayout(DescriptorSetLayoutDesc &description)
    {
        auto binds = std::vector<BindingDesc>(description.bindingDescriptors);
        std::reverse(binds.begin(), binds.end());
        return getSetLayout(description, binds);
    }

    DescriptorSetLayout &DescriptorSetTree::getSetLayoutData(Handle<DescriptorSetLayout> &handle)
    {
        return m_layoutPool->get(handle);
    }

    Handle<DescriptorSetLayout> DescriptorSetTreeNode::getSetLayout(DescriptorSetLayoutDesc& description,
                                                                    DescriptorSetTree& tree)
    {
        if(!m_setLayout)
        {
            m_setLayout = tree.createSetLayout(description);
        }
        return m_setLayout;
    }

    DescriptorSetTreeNode::DescriptorSetTreeNode(DESCRIPTOR_TYPE type, uint32_t quant)
    :  m_descQuant(quant), m_descType(type){  }


    DescriptorSetTreeNode& DescriptorSetTreeNode::findNode(std::vector<BindingDesc> &binds)
    {
        if(binds.empty()){
            return *this;
        }
        auto bind = binds.back();

        for(auto& child : m_children){
            if(child->m_descType == bind.type &&
               child->m_descQuant == bind.descriptorCount)
                return *child;
        }

        auto& child = addChild(bind.type, bind.descriptorCount);
        return child.findNode(binds);
    }
    DescriptorSetTreeNode& DescriptorSetTreeNode::addChild(DESCRIPTOR_TYPE type, uint32_t quant)
    {
        auto child = std::make_shared<DescriptorSetTreeNode>(type, quant);
        m_children.push_back(child);
        return *child;
    }
}
#pragma once

#include <memory>
#include <vector>

#include <boitatah/backend/vulkan/Vulkan.hpp>
#include <boitatah/BoitatahEnums.hpp>

#include <boitatah/types/Descriptors.hpp>
#include <boitatah/collections/Pool.hpp>

namespace boitatah::vk::descriptor_sets{

    class DescriptorSetTree;
    /// @brief A type node can parent M quantity nodes
    /// a quantity node can parent N type nodes and contain 1 set.
    class DescriptorSetTreeNode{

        private:
            bool empty = false;
            DESCRIPTOR_TYPE m_descType;
            uint32_t m_descQuant;
            Handle<DescriptorSetLayout> m_setLayout;
            std::vector<std::shared_ptr<DescriptorSetTreeNode>> m_children;
        
        public:
            DescriptorSetTreeNode() : empty(true) {  };
            DescriptorSetTreeNode(DESCRIPTOR_TYPE type, uint32_t quant) :
                                  m_descQuant(quant), m_descType(type){  }

            Handle<DescriptorSetLayout> getSetLayout(const DescriptorSetLayoutDesc& description,
                                                     DescriptorSetTree& tree);
            DescriptorSetTreeNode& findNode(std::vector<BindingDesc>& binds);
            DescriptorSetTreeNode& addChild(DESCRIPTOR_TYPE type, uint32_t quant);
    };
    
    class DescriptorSetTree{
        friend class DescriptorSetTreeNode;
        private:
            std::shared_ptr<VulkanInstance> m_vk;
            std::unique_ptr<Pool<DescriptorSetLayout>> m_layoutPool;
            std::unique_ptr<DescriptorSetTreeNode> m_nodes;
            /// @brief special case empty set.
            //Handle<DescriptorSetLayout> m_emptySet; 


            Handle<DescriptorSetLayout> createSetLayout(const DescriptorSetLayoutDesc& description);
            Handle<DescriptorSetLayout> getSetLayout(const DescriptorSetLayoutDesc& description,
                                                     std::vector<BindingDesc>& binds);
        public:
            DescriptorSetTree(std::shared_ptr<VulkanInstance> vulkan);
            Handle<DescriptorSetLayout> getSetLayout(const DescriptorSetLayoutDesc& description);
            DescriptorSetLayout& getSetLayoutData(const Handle<DescriptorSetLayout>& handle);

    };
}
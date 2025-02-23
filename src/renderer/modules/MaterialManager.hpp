#pragma once

#include <types/Material.hpp>
#include <collections/Pool.hpp>
#include <types/Shader.hpp>
#include <vulkan/DescriptorSetManager.hpp>
#include <vulkan/DescriptorSetTree.hpp>
#include <types/BttEnums.hpp>
#include <unordered_map>
#include <unordered_set>
#include <memory>
namespace boitatah{

    class MaterialGraph{
        struct NODE{
            Handle<Material> handle;
            Handle<Material> parent;
            std::vector<Handle<Material>> children;
        };

        private:
            std::unordered_map<Handle<Material>, NODE, HandleHasher> m_nodes;
            std::vector<Handle<Material>> m_currentOrder;

            bool m_dirty = true;

            void orderGraph();
        public:
            void addMaterial(Handle<Material> mat, Handle<Material> parent);
            void removeMaterial(Handle<Material> mat);
            const std::vector<Handle<Material>>& getOrder();
    };


    class ShaderManager{
        private:
            std::shared_ptr<Vulkan> m_vk;
            std::vector<Handle<Shader>> m_currentShaders;
            std::unique_ptr<Pool<ShaderLayout>> m_layoutPool;
            std::unique_ptr<Pool<Shader>> m_shaderPool;

            ShaderModule compileShaderModule(const std::vector<char>& bytecode, std::string entryPoint);
            VkPipeline compileShader();
            void reflectShader();

        public:
            ShaderManager(std::shared_ptr<Vulkan> vulkan) : m_vk(vulkan) {};
            Handle<ShaderLayout> get(Handle<ShaderLayout>& handle);
            Shader& get(Handle<Shader>& handle);
            bool isValid(Handle<Shader>& handle);
            bool isValid(Handle<ShaderLayout>& handle);
            Handle<Shader> makeShader(const MakeShaderDesc &data); //a shader is a pipeline
            void destroyShader(Handle<Shader>& handle);
    };

    class MaterialManager{
        public:
            MaterialManager(std::shared_ptr<Vulkan> vulkan);
            ShaderManager& getShaderManager();
            Handle<Material> createMaterial(const MaterialCreate& description);
            void destroyMaterial(const Handle<Material>& handle);
            const std::vector<Handle<Material>>& orderMaterials();
            Material& getMaterialContent(const Handle<Material>& handle);
            Handle<MaterialBinding> createBinding(Handle<DescriptorSetLayout>& description);
            MaterialBinding& getBinding(Handle<MaterialBinding>& handle);
            

            bool BindMaterial(Handle<Material>& handle, CommandBuffer& buffer);
            bool BindPipeline(Handle<Shader>& handle, CommandBuffer& buffer);
            bool BindBinding(Handle<MaterialBinding>& handle, uint32_t set_index, CommandBuffer& buffer);
            void setBaseBindings(std::vector<Handle<MaterialBinding>>&& handles);
            void setBaseBindings(const std::vector<Handle<MaterialBinding>>& handles);
        
        private:
            std::shared_ptr<Vulkan> m_vk;        
            std::unique_ptr<ShaderManager> m_shaderManager;
            std::shared_ptr<DescriptorSetManager> m_descriptorManager;
            MaterialGraph m_materialGraph;
            std::unique_ptr<Pool<Material>> m_materialPool;
            std::unique_ptr<Pool<MaterialBinding>> m_bindingsPool;

            std::vector<Handle<MaterialBinding>> m_currentBindings;
            Handle<Shader> m_currentPipeline;

            std::vector<Handle<MaterialBinding>> m_baseBindings;

            void inheritMaterial();

    };
};
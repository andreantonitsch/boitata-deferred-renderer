#pragma once

#include <types/Material.hpp>
#include <collections/Pool.hpp>
#include <types/Shader.hpp>
#include <vulkan/DescriptorSetManager.hpp>
#include <vulkan/DescriptorSetTree.hpp>
#include <renderer/modules/RenderTargetManager.hpp>
#include <renderer/modules/GPUResourceManager.hpp>
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

            std::pair<Handle<Shader>, Handle<ShaderLayout>> unlit_shader;

            std::vector<Handle<Shader>> m_currentShaders;

            Handle<DescriptorSetLayout> m_baseLayout; 

            std::unique_ptr<Pool<ShaderLayout>> m_layoutPool;
            std::unique_ptr<Pool<Shader>> m_shaderPool;
            std::shared_ptr<RenderTargetManager> m_targetManager;
            std::shared_ptr<DescriptorSetManager> m_descriptorManager;
            
            ShaderModule compileShaderModule(const std::vector<char>& bytecode, std::string entryPoint);
            VkPipeline compileShader();
            void updateShadersForRenderPass(Handle<RenderPass> renderPass);
            void updateShadersForRenderTarget(Handle<RenderPass> renderPass);
            void updateShadersForRenderTarget(std::vector<Handle<Shader>>& shaders,
                                              Handle<RenderPass> renderPass);
            void reflectShader();

        public:
            ShaderManager(std::shared_ptr<Vulkan> vulkan, 
                         std::shared_ptr<RenderTargetManager> targetManager,
                         std::shared_ptr<DescriptorSetManager> descriptorManager) 
;
            
            Handle<Shader> getUnlitShader();

            void setBaseLayout(Handle<DescriptorSetLayout> handle);
            ShaderLayout& get(const Handle<ShaderLayout>& handle);
            Shader& get(const Handle<Shader>& handle);
            bool isValid(Handle<Shader>& handle);
            bool isValid(Handle<ShaderLayout>& handle);
            Handle<ShaderLayout> makeShaderLayout(const ShaderLayoutDesc& description);
            Handle<Shader> makeShader(const MakeShaderDesc &data); //a shader is a pipeline
            void destroy(Handle<Shader>& handle);
            void destroy(Handle<ShaderLayout>& handle);
    };

    class MaterialManager{
        public:
            MaterialManager(std::shared_ptr<Vulkan> vulkan, 
                            std::shared_ptr<RenderTargetManager> targetManager,
                            std::shared_ptr<DescriptorSetManager> setManager,
                            std::shared_ptr<GPUResourceManager> resourceManager);
            ShaderManager& getShaderManager();
            Handle<Material> createMaterial(const MaterialCreate& description);
            void destroyMaterial(const Handle<Material>& handle);
            const std::vector<Handle<Material>>& orderMaterials();
            Material& getMaterialContent(const Handle<Material>& handle);
            Handle<MaterialBinding> createBinding(Handle<DescriptorSetLayout>& description);
            MaterialBinding& getBinding(Handle<MaterialBinding>& handle);
            
            bool BindMaterial(  Handle<Material>                &handle,
                                uint32_t                        frame_index,
                                CommandBuffer                   &buffer);

            bool BindPipeline(Handle<Shader>& handle, CommandBuffer& buffer);
            bool BindBinding(
                            Handle<MaterialBinding>         &handle,
                            ShaderLayout                    &shaderLayout,
                            Handle<DescriptorSetLayout>     setLayout,
                            uint32_t                        set_index,
                            uint32_t                        frame_index,
                            CommandBuffer                   &buffer);
            void setBaseBindings(std::vector<Handle<MaterialBinding>>&& handles);
            void setBaseBindings(const std::vector<Handle<MaterialBinding>>& handles);
        
            Handle<Material> getUnlitMaterial(const MaterialCreate& description);
            void resetBindings();
        private:
            std::shared_ptr<Vulkan> m_vk;        
            std::shared_ptr<RenderTargetManager> m_targetManager;
            std::shared_ptr<DescriptorSetManager> m_descriptorManager;
            std::shared_ptr<GPUResourceManager> m_resourceManager;
            std::unique_ptr<ShaderManager> m_shaderManager;
            MaterialGraph m_materialGraph;
            std::unique_ptr<Pool<Material>> m_materialPool;
            std::unique_ptr<Pool<MaterialBinding>> m_bindingsPool;

            std::vector<Handle<MaterialBinding>> m_currentBindings;
            Handle<Shader> m_currentPipeline;

            std::vector<Handle<MaterialBinding>> m_baseBindings;

            void inheritMaterial();

    };
};
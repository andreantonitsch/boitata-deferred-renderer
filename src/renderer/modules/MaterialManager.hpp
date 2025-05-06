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


            std::vector<Handle<Shader>> m_currentShaders;

            std::unique_ptr<Pool<ShaderLayout>> m_layoutPool;
            std::unique_ptr<Pool<Shader>> m_shaderPool;
            std::shared_ptr<RenderTargetManager> m_targetManager;
            std::shared_ptr<DescriptorSetManager> m_descriptorManager;

            ShaderModule compileShaderModule(const std::vector<char>& bytecode, 
                                             std::string entryPoint);
            void updateShadersForRenderPass(Handle<RenderPass> renderPass);
            void updateShadersForRenderTarget(Handle<RenderPass> renderPass);
            void updateShadersForRenderTarget(std::vector<Handle<Shader>>& shaders,
                                              Handle<RenderPass> renderPass);
            void reflectShader();

        public:
            ShaderManager(std::shared_ptr<Vulkan> vulkan, 
                         std::shared_ptr<RenderTargetManager> targetManager,
                         std::shared_ptr<DescriptorSetManager> descriptorManager);

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
            
            const std::vector<Handle<Material>> orderMaterials();
            void printMaterial(Handle<Material> handle); 
            void printBindings(Handle<MaterialBinding> handle);
            void printVertexBuffers(Handle<Material> handle);

            Handle<Material> createMaterial(const MaterialCreate& description);
            Handle<Material> copyMaterial(const Handle<Material>& handle);
            Material& getMaterialContent(const Handle<Material>& handle);
            void destroyMaterial(const Handle<Material>& handle);

            //void setBaseMaterial(const Handle<Material>& handle);

            MaterialBinding& getBinding(Handle<MaterialBinding>& handle);
            Handle<MaterialBinding> createBinding(
                                    const Handle<DescriptorSetLayout> &description);
            Handle<MaterialBinding> createBinding(
                                    const std::vector<MaterialBindingAtt>& bindings);
            
            std::vector<Handle<MaterialBinding>> createBindings(
                                                 const Handle<ShaderLayout> &description);   
            Handle<DescriptorSetLayout> createBindingsSetLayout(Handle<MaterialBinding>& binding);

            // Creates bindings same as createBindings about. 
            // But overrides the first overrides.size() bindings with the bindings in overrides
            // this is useful for basing materials on other materials and avoiding double binding       
            std::vector<Handle<MaterialBinding>> createBindings(
                                                 const Handle<ShaderLayout> &description,
                                                 std::vector<Handle<MaterialBinding>> overrides);

            bool setTextureBindingAttribute (const Handle<Material> material,
                                             const Handle<RenderTexture>                &texture,
                                             const uint32_t                             set, 
                                             const uint32_t                             binding);
            bool setTextureBindingAttribute (const Handle<Material> material,
                                             const Handle<FixedTexture>                 &texture,
                                             const uint32_t                             set, 
                                             const uint32_t                             binding);
            bool setBufferBindingAttribute  (const Handle<Material> material,
                                             const Handle<GPUBuffer>                    &gpu_buffer,
                                             const uint32_t                             set, 
                                             const uint32_t                             binding);
            bool setImageBindingAttribute   (const Handle<Material> material,
                                             const Handle<Image>                        &image,
                                             const uint32_t                             set, 
                                             const uint32_t                             binding);
            bool setSamplerBindingAttribute (const Handle<Material> material,
                                             const Handle<Sampler>                      &sampler,
                                             const uint32_t                             set, 
                                             const uint32_t                             binding);

            //std::vector<Handle<MaterialBinding>> createUnlitMaterialBindings();
            
            template <typename BufferWriterType>
            bool BindMaterial(CommandBufferWriter<BufferWriterType> &writer,
                                            Handle<Material>  &handle, 
                                            uint32_t         frame_index)
            {
                auto& material = m_materialPool->get(handle);
                auto& shader = m_shaderManager->get(material.shader);

                m_currentBindings.resize(material.bindings.size());

                bool success = true;
                if(material.shader && m_currentPipeline != material.shader){
                    success &= BindPipeline(writer, material.shader);
                }else{
                    std::runtime_error("failed to bind pipeline");
                }
        
                for(uint32_t i = 0; i < material.bindings.size(); i++){
                    success &= BindBinding( writer,
                                            material.bindings[i],
                                            shader.layout,
                                            shader.layout.descriptorSets[i],
                                            i,
                                            frame_index);
                }
                return success;
            }

            template <typename BufferWriterType>
            bool BindPipeline(CommandBufferWriter<BufferWriterType> &writer,
                                       Handle<Shader> &handle)
            {
                if(!handle)
                    return false;

                if(!m_shaderManager->isValid(handle))
                    return false;

                if(m_currentPipeline == handle)
                    return true;

                m_currentPipeline = handle;

                auto pipeline = m_shaderManager->get(m_currentPipeline).pipeline;
                writer.bind_pipeline({.pipeline = pipeline,});
                return true;
            }

            template <typename BufferWriterType>
            bool BindBinding(CommandBufferWriter<BufferWriterType> &writer,
                                    Handle<MaterialBinding>         &handle,
                                    ShaderLayout                    &shaderLayout,
                                    Handle<DescriptorSetLayout>     setLayout,
                                    uint32_t                        set_index,
                                    uint32_t                        frame_index)
            {
                //invalid binding
                if(!m_bindingsPool->contains(handle)){
                    //std::cout << " skipped binding " << set_index << " invalid handle." << std::endl;
                    return false;
                }
                
                //already bound
                if(m_currentBindings[set_index] == handle){
                    //std::cout << " skipped binding " << set_index << " already bound." << std::endl;
                    return true;
                }

                m_currentBindings[set_index] = handle;
                auto& binding = getBinding(handle);


                auto& layoutContent = m_descriptorManager->getLayoutContent(setLayout);
                auto set = m_descriptorManager->getSet(layoutContent, frame_index);
                
                std::vector<BindBindingDesc> bindings;
                for(int i = 0; i < binding.bindings.size(); i++){
                    BindBindingDesc desc;
                    desc.binding = i;
                    desc.type = binding.bindings[i].type;
                    switch(desc.type){
                        case DESCRIPTOR_TYPE::UNIFORM_BUFFER:
                            desc.access.bufferData = m_resourceManager->
                                                            getCommitResourceAccessData(
                                                                binding.bindings[i].binding_handle.buffer,
                                                                frame_index);
                            break;
                            
                        case DESCRIPTOR_TYPE::COMBINED_IMAGE_SAMPLER:
                            desc.access.textureData = m_resourceManager->
                                                            getCommitResourceAccessData(
                                                                binding.bindings[i].binding_handle.renderTex,
                                                            frame_index);
                            break;

                        case DESCRIPTOR_TYPE::IMAGE:{
                            auto& image = m_resourceManager->getImageManager().
                                                            getImage(
                                                                binding.bindings[i].binding_handle.image);
                            desc.access.imageData = {.view = image.view};
                            break;} 

                        case DESCRIPTOR_TYPE::SAMPLER:{
                            auto& sampler = m_resourceManager->getImageManager().
                                                getSampler(binding.bindings[i].binding_handle.sampler);
                            desc.access.samplerData = {.sampler = sampler.sampler};
                            break; }

                    }

                    bindings.push_back(desc);
                }
                m_descriptorManager->writeSet(bindings,    
                                            set, 
                                            frame_index);

                writer.bind_set({   shaderLayout.pipeline,
                                    set.descriptorSet,
                                    set_index});


                return true;
            }
            
            void resetBindings();

            //void setupBaseMaterials(Handle<RenderPass> renderpass);
            void clearBaseMaterials();

        private:
            std::shared_ptr<Vulkan> m_vk;        
            std::shared_ptr<RenderTargetManager> m_targetManager;
            std::shared_ptr<DescriptorSetManager> m_descriptorManager;
            std::shared_ptr<GPUResourceManager> m_resourceManager;
            std::unique_ptr<ShaderManager> m_shaderManager;
            
            //MaterialGraph m_materialGraph;

            std::shared_ptr<Pool<Material>> m_materialPool;
            std::unique_ptr<Pool<MaterialBinding>> m_bindingsPool;

             std::vector<Handle<Material>> current_Materials;

            std::vector<Handle<MaterialBinding>> m_currentBindings;
            Handle<Shader> m_currentPipeline;

            //std::pair<Handle<Shader>, Handle<ShaderLayout>> unlit_shader;

            //Handle<Material> m_baseMaterial;


            //void setupUnlitMaterial(Handle<RenderPass> renderpass);

    };
};
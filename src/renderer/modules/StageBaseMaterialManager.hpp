#pragma once

#include <renderer/modules/BackBufferDesc.hpp>
#include <renderer/modules/MaterialManager.hpp>
#include <renderer/modules/Camera.hpp>
#include <unordered_map>
#include <types/Texture.hpp>
#include <renderer/modules/GPUResourceManager.hpp>
#include <renderer/modules/BackBuffer.hpp>

namespace boitatah{

    enum class ShaderType : uint32_t{
        Unlit      = 0,
        Lambert    = 1,
        PBR        = 1,
        ComposePBR = 2,
    };

    //This class frees BackBuffer from having to know aobut
    //the base Materials from each stage
    // it also removes the dependency between backbuffermanager an dmaterial managerb c
    class Materials{

        public:
            Materials(std::shared_ptr<MaterialManager> material_mngr,
                      std::shared_ptr<DescriptorSetManager> descriptor_manager,
                      std::shared_ptr<RenderTargetManager> target_manager,
                      std::shared_ptr<BackBufferManager> back_buffer) 
                      : m_material_mngr(material_mngr),
                        m_descriptor_mngr(descriptor_manager),
                        m_target_mngr(target_manager),
                        m_back_buffer(back_buffer){
                            GenerateStageBaseMaterials();
                            BuildShaderMap();
                        };
                        
            Handle<Material> getStageBaseMaterial(uint32_t stage_index);

            void GenerateStageBaseMaterials();
            void ClearStageBaseMaterials();


            Handle<Material> createUnlitMaterial(uint32_t                stage_mask,
                                                 uint32_t                priority,
                                                 Handle<RenderTexture>   texture);
            Handle<Material> createLambertMaterial(uint32_t                stage_mask,
                                                 uint32_t                priority,
                                                 Handle<RenderTexture>   texture);

            Handle<Material> createUnlitDeferredComposeMaterial(uint32_t     stage_mask,
                                                                uint32_t     priority);
            Handle<Material> createLambertDeferredComposeMaterial(uint32_t   stage_mask,
                                                                uint32_t     priority);
            Handle<Material> createPBRComposerMaterial(uint32_t priority, uint32_t stage_mask);

            Handle<MaterialBinding> getCameraStageBinding();
            Handle<MaterialBinding> getScreenQuadStageBinding();

        private:
            //an matrix of StageIndex x ShaderType
            using ShaderMap = std::array<
                                std::array<
                                    std::pair<Handle<Shader>, Handle<ShaderLayout>>,
                                    4>, 
                                10>;

            void BuildShaderMap();
            void BuildUnlitShader(uint32_t stage_index);
            void BuildLambertShader(uint32_t stage_index);

            Handle<Material> GenerateBaseCameraMaterial(Handle<RenderStage> stage_handle);
            Handle<Material> GenerateBaseScreenQuadMaterial(Handle<RenderStage> stage_handle);

            std::vector<Handle<Material>> base_materials;

            ShaderMap base_shaders;

            std::shared_ptr<MaterialManager> m_material_mngr;
            std::shared_ptr<DescriptorSetManager> m_descriptor_mngr;
            std::shared_ptr<RenderTargetManager> m_target_mngr;
            std::shared_ptr<BackBufferManager> m_back_buffer;
    };
 };
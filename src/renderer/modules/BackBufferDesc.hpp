#pragma once

#include <glm/vec2.hpp>
#include <vector>
#include <types/BttEnums.hpp>
#include <types/Material.hpp>

namespace boitatah{

    enum class StageType : uint32_t{
        CAMERA      = 0, //renders a SceneNode list
        SCREEN_QUAD = 1  //renders a screenquad (for compositing/post-processing)
    };
    struct PresentLink{
        //TargetLinkType type;
        uint32_t target_idx;
        uint32_t attach_idx;
    };

    //Repeats a render target on this
    struct TargetLink{
        uint32_t prev_target_idx = UINT32_MAX;
    };

    //uses the same attachment as an attachment
    // as a previous stage
    struct AttachToAttachLink{
        uint32_t prev_target_idx = UINT32_MAX;
        uint32_t prev_attach_idx = UINT32_MAX;
        uint32_t attachment_idx = UINT32_MAX;
    };

    //backbuffer links
    // Links previous render targets as SampledTextures on 
    // set Bindings.
    struct AttachToTextureLink{
        //TargetLinkType type;
        uint32_t prev_stage_idx;
        uint32_t prev_attach_idx;
        //uint32_t binding_idx;

        AttachToTextureLink(std::initializer_list<uint32_t> indices){
            std::vector<uint32_t> v(indices);
            prev_stage_idx = v[0];
            prev_attach_idx = v[1];
            //binding_idx = v[2];
        };
    };

    struct Attach_Texture_Link{

    };

    //links renderpasses together with a binding set
    struct BindingLinks{
        std::vector<AttachToTextureLink> attachToTexture;
        std::vector<AttachToAttachLink> attachToAttach;
        TargetLink targetLink;

    };


    /// @brief attachments.size() == attachmentFormats.size()
    struct BackBufferDesc{
        std::vector<ATTACHMENT_TYPE> attachments = {ATTACHMENT_TYPE::COLOR,
                                                    ATTACHMENT_TYPE::DEPTH_STENCIL};
        std::vector<IMAGE_FORMAT> attachmentFormats = {
            IMAGE_FORMAT::RGBA_8_SRGB,
            IMAGE_FORMAT::R_32_SFLOAT
            };
        SAMPLES samples = SAMPLES::SAMPLES_1;
        glm::u32vec2 dimensions = {0, 0};
    };


    struct RenderStageDesc{
        StageType type = StageType::CAMERA;
        //depth_stencil attachments currently have to be the last in the list.
        std::vector<ATTACHMENT_TYPE> attachments = {ATTACHMENT_TYPE::COLOR,
                                                    ATTACHMENT_TYPE::DEPTH_STENCIL};
        std::vector<IMAGE_FORMAT> attachmentFormats = {
            IMAGE_FORMAT::RGBA_8_SRGB,
            IMAGE_FORMAT::R_32_SFLOAT
            };
        bool clear = true;
        SAMPLES samples = SAMPLES::SAMPLES_1;
        //SamplerData sampler_desc = SamplerData{};
        BindingLinks links;
    };

    struct RenderStage{
        uint32_t stage_index;
        StageType type = StageType::CAMERA;
        bool clear = true;
        Handle<RenderTarget> target;
        //std::vector<Handle<RenderTexture>> attachment_copy_tex;
        //Handle<MaterialBinding> materialBinding;
        std::vector<Handle<RenderTargetSync>> wait_list;
        RenderStageDesc description;
    
    };

    struct BackBufferDesc2{
        glm::u32vec2 dimensions = {0, 0};
        std::vector<RenderStageDesc> render_stages;
        PresentLink present_link;
    };


}

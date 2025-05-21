#include <boitatah/modules/StageBaseMaterialManager.hpp>
#include <boitatah/utils/utils.hpp>

namespace boitatah{

    Handle<Material> Materials::getStageBaseMaterial(uint32_t stage_index){
        return base_materials[stage_index];
    }

    void Materials::GenerateStageBaseMaterials() {
        std::cout << "generating rendergraph base materials" << std::endl;
        for(auto& target_handle : m_back_buffer->getCurrent_Graph()){
            auto& stage = m_back_buffer->getStage(target_handle);
            std::cout << "creating base material for stage " << stage.stage_index << std::endl;
            switch(stage.type){
                case StageType::CAMERA:
                    base_materials.push_back(GenerateBaseCameraMaterial(target_handle));
                    break;
                case StageType::SCREEN_QUAD:
                    base_materials.push_back(GenerateBaseScreenQuadMaterial(target_handle));
                    break;
            }
            std::cout << "created base material for stage " << stage.stage_index << std::endl;

        }
        std::cout << "generating rendergraph base materials finished" << std::endl;
    }

    void Materials::ClearStageBaseMaterials() {
    
    }

    Handle<Material> Materials::createUnlitMaterial(uint32_t                stage_mask,
                                                    uint32_t                priority,
                                                    Handle<RenderTexture>   texture){

        auto compat_stage_handle = m_back_buffer->getCompatibleRenderStage( 1u << stage_mask);
        auto& compatible_stage = m_back_buffer->getStage(compat_stage_handle);


        uint32_t stage_index =  compatible_stage.stage_index;
        auto base_mat_handle = getStageBaseMaterial(stage_index);
        auto& base_mat = m_material_mngr->getMaterialContent(base_mat_handle);
        auto base_bindings = base_mat.bindings;

        // create unlit material bindings     
        auto shader = base_shaders[static_cast<uint32_t>(stage_index)]
                                  [static_cast<uint32_t>(ShaderType::Unlit)];
        auto bindings = m_material_mngr->createBindings(shader.second, base_bindings);

        auto material =  m_material_mngr->createMaterial({
            .stage_mask = stage_mask,
            .priority = priority,
            .shader = shader.first,
            .bindings = bindings,
            .vertexBufferBindings = {
                VERTEX_BUFFER_TYPE::POSITION,
                VERTEX_BUFFER_TYPE::UV, 
                VERTEX_BUFFER_TYPE::COLOR,
            },
            .name = "unlit material",
        });
        m_material_mngr->setTextureBindingAttribute(material, texture, 1, 0);
        return material;
    }

    Handle<Material> Materials::createLambertMaterial(uint32_t stage_mask, 
                                                      uint32_t priority, 
                                                      Handle<RenderTexture> texture)
    {
        auto compat_stage_handle = m_back_buffer->getCompatibleRenderStage( 1u << stage_mask);
        auto& compatible_stage = m_back_buffer->getStage(compat_stage_handle);


        uint32_t stage_index =  compatible_stage.stage_index;
        auto base_mat_handle = getStageBaseMaterial(stage_index);
        auto& base_mat = m_material_mngr->getMaterialContent(base_mat_handle);
        auto base_bindings = base_mat.bindings;

        // create unlit material bindings     
        auto shader = base_shaders[static_cast<uint32_t>(stage_index)]
                                  [static_cast<uint32_t>(ShaderType::Lambert)];
        auto bindings = m_material_mngr->createBindings(shader.second, base_bindings);

        auto material =  m_material_mngr->createMaterial({
            .stage_mask = stage_mask,
            .priority = priority,
            .shader = shader.first,
            .bindings = bindings,
            .vertexBufferBindings = {
                VERTEX_BUFFER_TYPE::POSITION,
                VERTEX_BUFFER_TYPE::UV, 
                VERTEX_BUFFER_TYPE::NORMAL,
                VERTEX_BUFFER_TYPE::COLOR,
            },
            .name = "lambert material",
        });
        m_material_mngr->setTextureBindingAttribute(material, texture, 1, 0);
        return material;
    }
    Handle<Material> Materials::createUnlitDeferredComposeMaterial(uint32_t stage_mask,
                                                                   uint32_t priority)
    {
        auto compat_stage_handle = m_back_buffer->getCompatibleRenderStage( 1u<< stage_mask);
        auto& compatible_stage = m_back_buffer->getStage(compat_stage_handle);

        uint32_t stage_index =  compatible_stage.stage_index;
        auto base_mat_handle = getStageBaseMaterial(stage_index);
        auto& base_mat = m_material_mngr->getMaterialContent(base_mat_handle);
        auto override_binding = base_mat.bindings;

        // create unlit material bindings     
        auto shader = base_shaders[static_cast<uint32_t>(stage_index)]
                                  [static_cast<uint32_t>(ShaderType::Unlit)];
        //auto bindings = m_material_mngr->createBindings(shader.second, override_binding);
        auto material =  m_material_mngr->createMaterial({
            .stage_mask = stage_mask,
            .priority = priority,
            .shader = shader.first,
            .bindings = override_binding,
            .vertexBufferBindings = {
                VERTEX_BUFFER_TYPE::POSITION,
                VERTEX_BUFFER_TYPE::UV, 
                VERTEX_BUFFER_TYPE::COLOR,
            },
            .name = "unlit compose material",
        });
        return material;
    }
    Handle<Material> Materials::createLambertDeferredComposeMaterial(uint32_t stage_mask,
                                                                     uint32_t priority)
    {
        auto compat_stage_handle = m_back_buffer->getCompatibleRenderStage( 1u<< stage_mask);
        auto& compatible_stage = m_back_buffer->getStage(compat_stage_handle);

        uint32_t stage_index =  compatible_stage.stage_index;
        auto base_mat_handle = getStageBaseMaterial(stage_index);
        auto& base_mat = m_material_mngr->getMaterialContent(base_mat_handle);
        auto override_binding = base_mat.bindings;

        // create unlit material bindings     
        auto shader = base_shaders[static_cast<uint32_t>(stage_index)]
                                  [static_cast<uint32_t>(ShaderType::Lambert)];
        auto bindings = m_material_mngr->createBindings(shader.second, override_binding);
        
        auto material =  m_material_mngr->createMaterial({
            .stage_mask = stage_mask,
            .priority = priority,
            .shader = shader.first,
            .bindings = bindings,
            .vertexBufferBindings = {
                VERTEX_BUFFER_TYPE::POSITION,
                VERTEX_BUFFER_TYPE::UV,
                VERTEX_BUFFER_TYPE::NORMAL,
                VERTEX_BUFFER_TYPE::COLOR,
            },
            .name = "lambert compose material",
        });
        return material;
    };

    void Materials::BuildShaderMap()
    {
        for(std::size_t i = 0; i < m_back_buffer->getStageCount(); ++i){
            BuildUnlitShader(i);
            BuildLambertShader(i);
        }
    }

    void Materials::BuildUnlitShader(uint32_t stage_index)
    {
        std::cout << "building unlit shader" << std::endl;
        auto stage = m_back_buffer->getStage(stage_index);
        auto base_material_handle = getStageBaseMaterial(stage_index);
        auto& base_material = m_material_mngr->getMaterialContent(base_material_handle);
        auto& base_shader = m_material_mngr->getShaderManager()
                                          .get(base_material.shader);

        //TODO make this less cumbersome.
        auto setLayouts = m_material_mngr->getShaderManager()
                                          .get(base_material.shader)
                                          .layout
                                          .descriptorSets;
    
        auto& pass = m_target_mngr->get(stage.target).renderpass;


        auto shader_desc = MakeShaderDesc{
                        .name = "unlit shader",
                        .renderPass = pass,
                        .vertexBindings = base_shader.description.vertexBindings,
                      };


        //TODO make this less hardcoded.
        switch(stage.type){
            case StageType::CAMERA:{
                //add a texture to the base camera
                auto texture_layout = m_descriptor_mngr->getLayout({
                                        .bindingDescriptors = {
                                        {//.binding = 0,
                                        .type = DESCRIPTOR_TYPE::COMBINED_IMAGE_SAMPLER,
                                        .stages = SHADER_STAGE::FRAGMENT,
                                        .descriptorCount= 1,
                                            }
                                        }
                                    });
                setLayouts.push_back(texture_layout);

                shader_desc.vert = {.byteCode =  utils::readFile(
                                            "./shaders/base_shaders/unlit_camera_mat.vert.spv"),
                                    .entryFunction = "main"};

                if(stage.description.attachments.size() == 2){
                    shader_desc.frag=  { .byteCode =  utils::readFile(
                                                "./shaders/base_shaders/unlit_material_forward.frag.spv"),
                                        .entryFunction = "main"};}

                if(stage.description.attachments.size() == 4){
                    std::cout << "building defered unlit camera shader" << std::endl;
                    shader_desc.frag=  { .byteCode =  utils::readFile(
                                                "./shaders/base_shaders/unlit_material_deferred.frag.spv"),
                                        .entryFunction = "main"};}
                break;
            }
            case StageType::SCREEN_QUAD:
                std::cout << "building defered unlit compose shader" << std::endl;
\
                shader_desc.vert = {.byteCode =  utils::readFile(
                                            "./shaders/base_shaders/base_screen_quad.vert.spv"),
                                    .entryFunction = "main"};
                shader_desc.frag = {.byteCode =  utils::readFile(
                                            "./shaders/base_shaders/unlit_deferred_compose.frag.spv"),
                                    .entryFunction = "main"};
                break;

        }

        auto shader_layout = m_material_mngr
                            ->getShaderManager()
                            .makeShaderLayout({.setLayouts = setLayouts});
        shader_desc.layout = shader_layout;

        //TODO temp workaround
        std::vector<ColorBlend> blends;
        uint32_t color_atts_count = stage.description.attachments.size();
        if(stage.description.attachments.back() == ATTACHMENT_TYPE::DEPTH_STENCIL)
            color_atts_count--;
        for(uint32_t i = 0; i < color_atts_count; i++)
            blends.push_back({});
        shader_desc.colorBlends = blends;

        auto shader = m_material_mngr
                      ->getShaderManager()
                      .makeShader(shader_desc);

        base_shaders[stage_index][static_cast<uint32_t>(ShaderType::Unlit)] = {
            shader, shader_layout
        };

    }

    void Materials::BuildLambertShader(uint32_t stage_index)
    {
             std::cout << "building lit shader" << std::endl;
        auto stage = m_back_buffer->getStage(stage_index);
        auto base_material_handle = getStageBaseMaterial(stage_index);
        auto& base_material = m_material_mngr->getMaterialContent(base_material_handle);
        auto& base_shader = m_material_mngr->getShaderManager()
                                          .get(base_material.shader);

        //TODO make this less cumbersome.
        auto setLayouts = m_material_mngr->getShaderManager()
                                          .get(base_material.shader)
                                          .layout
                                          .descriptorSets;
    
        auto& pass = m_target_mngr->get(stage.target).renderpass;

        auto shader_desc = MakeShaderDesc{
                        .name = "unlit shader",
                        .renderPass = pass,
                        .vertexBindings = base_shader.description.vertexBindings,
                      };

        auto lights_layout = m_descriptor_mngr->getLayout({
                                        .bindingDescriptors = {
                                        {//.binding = 0,
                                            .type = DESCRIPTOR_TYPE::UNIFORM_BUFFER,
                                            .stages = SHADER_STAGE::FRAGMENT,
                                            .descriptorCount= 1,
                                        },
                                        {//.binding = 0,
                                            .type = DESCRIPTOR_TYPE::UNIFORM_BUFFER,
                                            .stages = SHADER_STAGE::FRAGMENT,
                                            .descriptorCount= 1,
                                        }
                                        }
                                    });

        //TODO make this less hardcoded.
        switch(stage.type){
            case StageType::CAMERA:{
                //add a texture to the base camera
                auto texture_layout = m_descriptor_mngr->getLayout({
                                        .bindingDescriptors = {
                                        {//.binding = 0,
                                        .type = DESCRIPTOR_TYPE::COMBINED_IMAGE_SAMPLER,
                                        .stages = SHADER_STAGE::FRAGMENT,
                                        .descriptorCount= 1,
                                            }
                                        }
                                    });
                setLayouts.push_back(texture_layout);

                shader_desc.vert = {.byteCode =  utils::readFile(
                                            "./shaders/base_shaders/lit_camera_mat.vert.spv"),
                                    .entryFunction = "main"};

                if(stage.description.attachments.size() == 2){
                    shader_desc.frag=  { .byteCode =  utils::readFile(
                                                "./shaders/base_shaders/lambert_material_forward.frag.spv"),
                                        .entryFunction = "main"};}

                if(stage.description.attachments.size() == 4){
                    std::cout << "building defered unlit camera shader" << std::endl;
                    shader_desc.frag=  { .byteCode =  utils::readFile(
                                                "./shaders/base_shaders/lit_material_deferred.frag.spv"),
                                        .entryFunction = "main"};}
                break;
            }
            case StageType::SCREEN_QUAD:
                std::cout << "building defered unlit compose shader" << std::endl;
                setLayouts.push_back(lights_layout);
                shader_desc.vert = {.byteCode =  utils::readFile(
                                            "./shaders/base_shaders/base_screen_quad.vert.spv"),
                                    .entryFunction = "main"};
                shader_desc.frag = {.byteCode =  utils::readFile(
                                            "./shaders/base_shaders/lambert_deferred_compose.frag.spv"),
                                    .entryFunction = "main"};
                break;

        }

        auto shader_layout = m_material_mngr
                            ->getShaderManager()
                            .makeShaderLayout({.setLayouts = setLayouts});
        shader_desc.layout = shader_layout;

        //TODO temp workaround
        std::vector<ColorBlend> blends;
        uint32_t color_atts_count = stage.description.attachments.size();
        if(stage.description.attachments.back() == ATTACHMENT_TYPE::DEPTH_STENCIL)
            color_atts_count--;
        for(uint32_t i = 0; i < color_atts_count; i++)
            blends.push_back({});
        shader_desc.colorBlends = blends;

        auto shader = m_material_mngr
                      ->getShaderManager()
                      .makeShader(shader_desc);

        base_shaders[stage_index][static_cast<uint32_t>(ShaderType::Lambert)] = {
            shader, shader_layout
        };   
    }

    Handle<Material> Materials::GenerateBaseCameraMaterial(Handle<RenderStage> stage_handle)
    {
        std::cout <<  "making base camera" << std::endl;
        auto& stage = m_back_buffer->getStage(stage_handle);
        //binds one camera transform
        auto base_setLayout = m_descriptor_mngr->getLayout({
                            .bindingDescriptors = {
                                {//.binding = 0,
                                .type = DESCRIPTOR_TYPE::UNIFORM_BUFFER,
                                .stages = SHADER_STAGE::ALL_GRAPHICS,
                                .descriptorCount= 1,
                                }
                            }
                        });
        auto uniforms = m_material_mngr->createBinding(base_setLayout);

        std::vector<VertexBindings> vertex_bindings = {
                                {.stride = 12, 
                                .attributes = {{.location = 0, 
                                                .format = IMAGE_FORMAT::RGB_32_SFLOAT, 
                                                .offset = 0}}},
                                {.stride = 8, .
                                attributes = {{ .location = 1, 
                                                .format = IMAGE_FORMAT::RG_32_SFLOAT, 
                                                .offset = 0}}},
                                {.stride = 12, 
                                .attributes = {{.location = 2, 
                                                .format = IMAGE_FORMAT::RGB_32_SFLOAT, 
                                                .offset = 0}}},
                                {.stride = 12, 
                                .attributes = {{.location = 3, 
                                                .format = IMAGE_FORMAT::RGB_32_SFLOAT, 
                                                .offset = 0}}}};

        //TODO temp workaround
        std::vector<ColorBlend> blends;
        uint32_t color_atts_count = stage.description.attachments.size();
        if(stage.description.attachments.back() == ATTACHMENT_TYPE::DEPTH_STENCIL)
            color_atts_count--;
        for(uint32_t i = 0; i < color_atts_count; i++)
            blends.push_back({});

        //make the shader and material
        auto shader_layout = m_material_mngr
                            ->getShaderManager()
                            .makeShaderLayout({.setLayouts={base_setLayout}});
        auto& pass = m_target_mngr->get(stage.target).renderpass;
        auto shader = m_material_mngr->getShaderManager().makeShader({
            .vert = {.byteCode = utils::readFile(
                                    "./shaders/base_shaders/base_camera_mat.vert.spv"),
                     .entryFunction = "main"},
            .frag = {.byteCode = utils::readFile(
                                    "./shaders/base_shaders/base_camera_mat.frag.spv"),
                     .entryFunction = "main"},
            .renderPass = pass,
            .layout = shader_layout,
            .colorBlends = blends,
            .vertexBindings = vertex_bindings
            });


        auto material = m_material_mngr->createMaterial({
            .stage_mask = 0,
            .priority = 0,
            .shader = shader,
            .bindings = {uniforms},
            .vertexBufferBindings = {VERTEX_BUFFER_TYPE::POSITION,
                                     VERTEX_BUFFER_TYPE::UV,
                                     VERTEX_BUFFER_TYPE::NORMAL,
                                     VERTEX_BUFFER_TYPE::COLOR},
            .name = "Base Camera Material"
        });
        return material;
    }

    Handle<Material> Materials::GenerateBaseScreenQuadMaterial(Handle<RenderStage> stage_handle)
    {

        std::cout <<  "making base Screen Quad" << std::endl;
        auto& stage = m_back_buffer->getStage(stage_handle);
        auto stage_binding = m_back_buffer->getStageBinding(stage_handle);
        

        //add stage uniforms
        auto stage_layout = m_material_mngr->createBindingsSetLayout(stage_binding);
        std::vector<Handle<MaterialBinding>> uniforms;
        uniforms.push_back(stage_binding);

        std::vector<VertexBindings> vertex_bindings = {{.stride = 12, 
                                .attributes = {{.location = 0, 
                                                .format = IMAGE_FORMAT::RGB_32_SFLOAT, 
                                                .offset = 0}}},
                                {.stride = 8, .
                                attributes = {{ .location = 1, 
                                                .format = IMAGE_FORMAT::RG_32_SFLOAT, 
                                                .offset = 0}}},
                                {.stride = 12, 
                                .attributes = {{.location = 2, 
                                                .format = IMAGE_FORMAT::RGB_32_SFLOAT, 
                                                .offset = 0}}}};

        //TODO temp workaround
        std::vector<ColorBlend> blends;
        uint32_t color_atts_count = stage.description.attachments.size();
        if(stage.description.attachments.back() == ATTACHMENT_TYPE::DEPTH_STENCIL)
            color_atts_count--;
        for(uint32_t i = 0; i < color_atts_count; i++)
            blends.push_back({});

        //auto tex_layout_desc = m_material_mngr->createBindingsSetLayout(stage.materialBinding);

        auto shader_layout = m_material_mngr
                            ->getShaderManager()
                            .makeShaderLayout({.setLayouts={stage_layout}});//, tex_layout_desc}});
        auto& pass = m_target_mngr->get(stage.target).renderpass;
        auto shader = m_material_mngr->getShaderManager().makeShader({
            .vert = {.byteCode = utils::readFile(
                                    "./shaders/base_shaders/base_screen_quad.vert.spv"),
                     .entryFunction = "main"},
            .frag = {.byteCode = utils::readFile(
                                    "./shaders/base_shaders/base_screen_quad.frag.spv"),
                     .entryFunction = "main"},
            .renderPass = pass,
            .layout = shader_layout,
            .colorBlends  = blends,
            .vertexBindings = vertex_bindings,
            });

        auto material = m_material_mngr->createMaterial({
            .stage_mask = 0,
            .priority = 0,
            .shader = shader,
            .bindings = uniforms,
            .vertexBufferBindings = {VERTEX_BUFFER_TYPE::POSITION,
                                     VERTEX_BUFFER_TYPE::UV,
                                     VERTEX_BUFFER_TYPE::COLOR},
            .name = "Base Screen Quad Material"
        });
        return material;
    };
};

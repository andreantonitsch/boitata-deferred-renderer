#include "MaterialManager.hpp"
#include <algorithm>
#include <utils/utils.hpp>
namespace boitatah{
    
    

    void MaterialGraph::orderGraph()
    {
        //TODO change for unique container
        std::vector<Handle<Material>> cur_mats;

        for(auto& k : m_nodes){
            cur_mats.push_back(k.first);
        }

        m_currentOrder.clear();
        m_currentOrder.resize(cur_mats.size());
        auto& nodes_ref = m_nodes;
        auto& order_ref = m_currentOrder;


        const auto add = [&cur_mats, &nodes_ref, &order_ref](const Handle<Material> mat)
            {
                auto add_impl = [&cur_mats, &nodes_ref, &order_ref](const Handle<Material> mat, auto& add_ref) -> void {
                    auto& node = nodes_ref[mat];
                    order_ref.push_back(mat);
                    cur_mats.erase(std::find(cur_mats.begin(), cur_mats.end(), mat));
                    for(auto& child : node.children){
                        add_ref(child, add_ref);
                    }
                    return;
                };

                add_impl(mat, add_impl);
            };

        while(!cur_mats.empty()){
            //find a root mat
            auto root_it = std::find_if(cur_mats.begin(), cur_mats.end(),
                    [&nodes_ref](const Handle<Material> mat){return !nodes_ref[mat].parent;});
            
            if(root_it == cur_mats.end())
                break;
            
            auto root = *root_it;
            //add all children.
            add(root);
        }
    }

    void MaterialGraph::addMaterial(Handle<Material> mat, Handle<Material> parent)
    {
        NODE new_node = NODE{
            .handle = mat,
            .parent = parent,
        };
        m_nodes[parent].children.push_back(mat);
        m_nodes[mat] = new_node;
        m_dirty = true;
    }
    void MaterialGraph::removeMaterial(Handle<Material> mat) {

        auto& node = m_nodes[mat];
        for(auto& child : node.children){
            removeMaterial(child);
        }
        
        m_nodes.erase(m_nodes.find(mat));

        m_dirty = true;
    }
    const std::vector<Handle<Material>>& MaterialGraph::getOrder() {
        if(m_dirty)
            orderGraph();

        return m_currentOrder;
    };

    MaterialManager::MaterialManager(std::shared_ptr<Vulkan> vulkan,
                                     std::shared_ptr<RenderTargetManager> targetManager,
                                     std::shared_ptr<DescriptorSetManager> setManager,
                                     std::shared_ptr<GPUResourceManager> resourceManager) 
     : m_vk(vulkan), m_targetManager(targetManager), m_descriptorManager(setManager),
       m_resourceManager(resourceManager)
    {
        m_shaderManager = std::make_unique<ShaderManager>(m_vk, m_targetManager, m_descriptorManager);
        m_materialPool = std::make_unique<Pool<Material>>(PoolOptions{
            .size = 4096,
            .dynamic = true,
            .name = "Material Pool"
        });
        m_bindingsPool  = std::make_unique<Pool<MaterialBinding>>(PoolOptions{
            .size = 4096,
            .dynamic = true,
            .name = "Binding Pool"
        });
    }

    ShaderManager &MaterialManager::getShaderManager()
    {
        return *m_shaderManager;
    }

    Handle<Material> MaterialManager::createMaterial(const MaterialCreate& description)
    {
        Material mat{};
        mat.shader = description.shader;
        mat.name = description.name;
        mat.bindings = description.bindings;
        mat.parent = description.parent;
        mat.vertexBufferBindings = description.vertexBufferBindings;
        auto handle = m_materialPool->set(mat);
        
        m_materialGraph.addMaterial(handle, mat.parent);
        
        return handle;
    }

    const std::vector<Handle<Material>>& MaterialManager::orderMaterials()
    {
        return m_materialGraph.getOrder();
    }

    Material& MaterialManager::getMaterialContent(const Handle<Material> &handle)
    {
        return m_materialPool->get(handle);
    }

    void MaterialManager::setBaseMaterial(const Handle<Material> &handle)
    {
        m_baseMaterial = handle;
    }

    Handle<MaterialBinding> MaterialManager::createBinding(const Handle<DescriptorSetLayout> &description)
    {
        MaterialBinding binding;
        auto& layout = m_descriptorManager->getLayoutContent(description);

        for(auto& desc : layout.description.bindingDescriptors){
            binding.bindings.push_back(MaterialBindingAtt{.type = desc.type});
        };

        return m_bindingsPool->set(binding);
    }

    std::vector<Handle<MaterialBinding>> MaterialManager::createBindings(const Handle<ShaderLayout> &description)
    {
        std::vector<Handle<MaterialBinding>>bindings;
        std::cout << " creating bindings " << std::endl;
        auto& base_material = getMaterialContent(m_baseMaterial);
        utils::concatenate_vectors(bindings, base_material.bindings);
        int base_binding_count = bindings.size();

        auto& layout = m_shaderManager->get(description);
        std::cout << "layout with " << layout.descriptorSets.size() << " sets " << std::endl;
        int total_bindings = layout.descriptorSets.size();

        for(int i = base_binding_count; i < total_bindings; i++){
            if(!layout.descriptorSets[i])
                std::runtime_error("Invalid layout description when creating bindings");
            bindings.push_back(createBinding(layout.descriptorSets[i]));
        }
        std::cout << " created bindings size " << bindings.size() << std::endl;
        return bindings;
    }

    std::vector<Handle<MaterialBinding>> MaterialManager::createUnlitMaterialBindings()
    {
        return createBindings(unlit_shader.second);
    }

    

    MaterialBinding &MaterialManager::getBinding(Handle<MaterialBinding> &handle)
    {
        return m_bindingsPool->get(handle);
    }

    bool MaterialManager::BindMaterial(Handle<Material>     &handle, 
                                       uint32_t             frame_index,
                                       CommandBuffer        &buffer)
    {
        auto& material = m_materialPool->get(handle);
        auto& shader = m_shaderManager->get(material.shader);

        m_currentBindings.resize(material.bindings.size());

        bool success = true;
        if(material.shader && m_currentPipeline != material.shader){
            success &= BindPipeline(material.shader, buffer);
        }else{
            std::runtime_error("failed to bind pipeline");
        }


 
        for(uint32_t i = 0; i < material.bindings.size(); i++){
            success &= BindBinding( material.bindings[i],
                                    shader.layout,
                                    shader.layout.descriptorSets[i],
                                    i,
                                    frame_index,
                                    buffer);
        }
        return success;
    }

    bool MaterialManager::BindPipeline(Handle<Shader> &handle, CommandBuffer& buffer)
    {
        if(!handle)
            return false;

        if(!m_shaderManager->isValid(handle))
            return false;

        if(m_currentPipeline == handle)
            return true;

        m_currentPipeline = handle;

        m_vk->bindPipelineCommand({
            .drawBuffer = buffer.buffer,
            .pipeline = m_shaderManager->get(m_currentPipeline).pipeline,
        });
        return true;
    }

    bool MaterialManager::BindBinding(
                                    Handle<MaterialBinding>         &handle,
                                    ShaderLayout                    &shaderLayout,
                                    Handle<DescriptorSetLayout>     setLayout,
                                    uint32_t                        set_index,
                                    uint32_t                        frame_index,
                                    CommandBuffer                   &buffer)
    {
        //invalid binding
        if(!m_bindingsPool->contains(handle))
           return false;
        
        //already bound
        if(m_currentBindings[set_index] == handle)
            return true;

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
                                                    getCommitResourceAccessData(binding.bindings[i].binding_handle.renderTex,
                                                    frame_index);
                                                    break;
            }

            bindings.push_back(desc);
        }
        m_descriptorManager->writeSet(bindings,    
                                      set, 
                                      frame_index);

        m_descriptorManager->bindSet(buffer,
                                    shaderLayout,
                                    set,
                                    set_index,
                                    frame_index);


        return true;
    }


    Handle<Material> MaterialManager::createUnlitMaterial(const std::vector<Handle<MaterialBinding>>& bindings)
    {
        return createMaterial({
            .parent = m_baseMaterial,
            .shader = unlit_shader.first,
            .bindings = bindings,
            .vertexBufferBindings = {
                VERTEX_BUFFER_TYPE::POSITION,
                VERTEX_BUFFER_TYPE::UV,
                VERTEX_BUFFER_TYPE::COLOR,
            },
            .name = "unlit material",
        });
    }

    void MaterialManager::resetBindings()
    {
        m_currentBindings.clear();
        m_currentPipeline = Handle<Shader>();
    }

    void MaterialManager::setupBaseMaterials(Handle<RenderPass> renderpass)
    {
        setupUnlitMaterial(renderpass);
    }

    void MaterialManager::setupUnlitMaterial(Handle<RenderPass> renderpass)
    {
        //binds one texture
        Handle<DescriptorSetLayout> layout = m_descriptorManager->getLayout({
            .bindingDescriptors = {
                {
                    .type = DESCRIPTOR_TYPE::COMBINED_IMAGE_SAMPLER,
                    .stages = STAGE_FLAG::FRAGMENT,
                    .descriptorCount = 1,
                },
            }});

        std::vector<Handle<DescriptorSetLayout>> full_layout;

        utils::concatenate_vectors(full_layout,
                                   m_shaderManager->get(getMaterialContent(m_baseMaterial)
                                                        .shader).layout.descriptorSets);
        
        full_layout.push_back(layout);
        
        Handle<ShaderLayout> unlit_layout = m_shaderManager->makeShaderLayout({
            .setLayouts = full_layout
        });

        Handle<Shader> shader = m_shaderManager->makeShader({
            .vert = { .byteCode =  utils::readFile("./shaders/basic_camera_mat.vert.spv"),
                        .entryFunction = "main"},
            .frag=  { .byteCode =  utils::readFile("./shaders/unlit_material.frag.spv"),
                        .entryFunction = "main"},
            .renderPass = renderpass,
            .layout = unlit_layout,
            .vertexBindings = {
                    {.stride = 12, .attributes = {{.location = 0, .format = IMAGE_FORMAT::RGB_32_SFLOAT, .offset = 0}}},
                    {.stride = 8, .attributes = {{.location = 1, .format = IMAGE_FORMAT::RG_32_SFLOAT, .offset = 0}}},
                    {.stride = 12, .attributes = {{.location = 2, .format = IMAGE_FORMAT::RGB_32_SFLOAT, .offset = 0}}},
                              }
            });
        unlit_shader = {shader, unlit_layout};
    }

    ShaderModule ShaderManager::compileShaderModule(const std::vector<char> &bytecode, std::string entryPoint)
    {
        return {.shaderModule = m_vk->createShaderModule(bytecode), .entryFunction = entryPoint};
    }
    Shader &ShaderManager::get(const Handle<Shader> &handle)
    {
        return m_shaderPool->get(handle);
    }

    ShaderManager::ShaderManager(std::shared_ptr<Vulkan> vulkan, std::shared_ptr<RenderTargetManager> targetManager, std::shared_ptr<DescriptorSetManager> descriptorManager)
    :   m_vk(vulkan), 
        m_targetManager(targetManager),
        m_descriptorManager(descriptorManager)
    {
        m_layoutPool = std::make_unique<Pool<ShaderLayout>>(PoolOptions{
            .size = 4096,
            .dynamic = true,
            .name = "shader layout Pool"
        });

        m_shaderPool = std::make_unique<Pool<Shader>>(PoolOptions{
            .size = 4096,
            .dynamic = true,
            .name = "shader Pool"
        });
    }

    ShaderLayout& ShaderManager::get(const Handle<ShaderLayout>& handle){
        return m_layoutPool->get(handle);
    };

    bool ShaderManager::isValid(Handle<Shader> &handle)
    {
        return m_shaderPool->contains(handle);
    }
    Handle<ShaderLayout> ShaderManager::makeShaderLayout(const ShaderLayoutDesc &description)
    {   
        std::vector<VkDescriptorSetLayout> vkLayouts;
        for(int i = 0; i < description.setLayouts.size(); i++){
            auto& layout = m_descriptorManager->getLayoutContent(description.setLayouts[i]);
            vkLayouts.push_back(layout.layout);
        }
        
        ShaderLayout layout{ 
                                .pipeline = m_vk->createShaderLayout(
                                    {
                                        .materialLayouts = vkLayouts,
                                        .pushConstants ={
                                            PushConstantDesc{
                                            .offset = 0, //<-- must be larger or equal than sizeof(glm::mat4)
                                            .size = sizeof(glm::mat4), //<- M matrices
                                            .stages = STAGE_FLAG::ALL_GRAPHICS}},
                                    }),
                                .descriptorSets = description.setLayouts,
                                };

        return m_layoutPool->set(layout);

    }
    Handle<Shader> ShaderManager::makeShader(const MakeShaderDesc &data)
    {
        Shader shader{
            .name = data.name,
            .vert = compileShaderModule(data.vert.byteCode, data.vert.entryFunction),
            .frag = compileShaderModule(data.frag.byteCode, data.vert.entryFunction)};

        ShaderLayout layoutData = m_layoutPool->get(data.layout);
        shader.layout = layoutData;

        // TODO Convert bindings in vulkan class?
        std::vector<VkVertexInputAttributeDescription> vkattributes;
        std::vector<VkVertexInputBindingDescription> vkbindings;
        //uint32_t location = 0;
        for (int i = 0; i < data.vertexBindings.size(); i++)
        {
            auto binding = data.vertexBindings[i];
            VkVertexInputBindingDescription bindingDesc{};
            bindingDesc.stride = binding.stride;
            bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDesc.binding = i;
            
            vkbindings.push_back(bindingDesc);
            uint32_t runningOffset = 0;

            for (int j = 0; j < binding.attributes.size(); j++)
            {
                auto attribute = binding.attributes[j];
                VkVertexInputAttributeDescription attributeDesc;
                attributeDesc.binding = i;
                attributeDesc.format = castEnum<VkFormat>(attribute.format);
                attributeDesc.offset = runningOffset;
                //attributeDesc.location = location++;
                attributeDesc.location = attribute.location;
                vkattributes.push_back(attributeDesc);

                runningOffset += formatSize(attribute.format);
            }
        }
        auto& pass = m_targetManager->get(data.renderPass);
        m_vk->buildShader({
            .name = data.name,
            .renderpass = pass.renderPass,
            .layout = shader.layout.pipeline,
            .use_depth = pass.description.use_depthStencil,
            .bindings = vkbindings,
            .attributes = vkattributes,
            
        }, shader);
        return m_shaderPool->move_set(shader);
    }
    void ShaderManager::destroy(Handle<Shader> &handle)
    {
        Shader shader;
        if (m_shaderPool->clear(handle, shader))
        {
            m_vk->destroyShader(shader);
            m_currentShaders.erase(
                std::find(
                    m_currentShaders.begin(),
                    m_currentShaders.end(), 
                    handle));
        }
    }

    void ShaderManager::destroy(Handle<ShaderLayout> &handle)
    {
        ShaderLayout layout;
        if (m_layoutPool->clear(handle, layout))
        {
            m_vk->destroyPipelineLayout(layout);
        }
    }
};
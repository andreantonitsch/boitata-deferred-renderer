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

    MaterialManager::MaterialManager(std::shared_ptr<Vulkan> vulkan) : m_vk(vulkan)
    {
        m_shaderManager = std::make_unique<ShaderManager>(m_vk);
        m_materialPool = std::make_unique<Pool<Material>>(PoolOptions{
            .size = 4096,
            .dynamic = true,
            .name = "Material Pool"
        });
    }

    ShaderManager &MaterialManager::getShaderManager()
    {
        return *m_shaderManager;
    }

    Handle<Material> MaterialManager::createMaterial(const MaterialCreate& description)
    {
        Material mat{};
        auto bindings = std::vector<Handle<MaterialBinding>>(m_baseBindings);

        utils::move_concatenate_vectors(bindings, description.bindings);

        mat.shader = description.shader;
        mat.name = description.name;
        mat.bindings = bindings;
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

    Handle<MaterialBinding> MaterialManager::createBinding(Handle<DescriptorSetLayout> &description)
    {

        MaterialBinding binding;
        auto& layout = m_descriptorManager->getLayoutContent(description);

        for(auto& desc : layout.description.bindingDescriptors){
            binding.bindings.push_back(MaterialBindingAtt{.type = desc.type});
        };

        return m_bindingsPool->set(binding);
    }

    MaterialBinding &MaterialManager::getBinding(Handle<MaterialBinding> &handle)
    {
        return m_bindingsPool->get(handle);
    }

    bool MaterialManager::BindMaterial(Handle<Material> &handle, CommandBuffer& buffer)
    {
        auto& material = m_materialPool->get(handle);
        m_currentBindings.resize(material.bindings.size());
        bool success = true;
        if(m_currentPipeline != material.shader)
            success &= BindPipeline(material.shader, buffer);

        for(uint32_t i = 0; i <= material.bindings.size(); i++){
            if(material.bindings[i] != m_currentBindings[i]){
                success &= BindBinding(material.bindings[i], i, buffer);
            }
        }
        return success;
    }

    bool MaterialManager::BindPipeline(Handle<Shader> &handle, CommandBuffer& buffer)
    {
        if(m_currentPipeline == handle ||
           !m_shaderManager->isValid(handle))
            return false;
        m_currentPipeline = handle;

        m_vk->bindPipelineCommand({
            .drawBuffer = buffer.buffer,
            .pipeline = m_shaderManager->get(m_currentPipeline).pipeline,
        });
        return true;
    }

    bool MaterialManager::BindBinding(Handle<MaterialBinding> &handle, uint32_t set_index, CommandBuffer& buffer)
    {
        if(m_currentBindings[set_index] == handle ||
           !m_bindingsPool->contains(handle))
           return false;
        m_currentBindings[set_index] = handle;

        return true;
    }

    void MaterialManager::setBaseBindings(std::vector<Handle<MaterialBinding>> &&handles)
    {
        m_baseBindings = std::move(handles);
    }

    void MaterialManager::setBaseBindings(const std::vector<Handle<MaterialBinding>> &handles)
    {
        m_baseBindings = handles;
    }

    ShaderModule ShaderManager::compileShaderModule(const std::vector<char> &bytecode, std::string entryPoint)
    {
        return {.shaderModule = m_vk->createShaderModule(bytecode), .entryFunction = entryPoint};
    }
    Shader &ShaderManager::get(Handle<Shader> &handle)
    {
        return m_shaderPool->get(handle);
    }
    bool ShaderManager::isValid(Handle<Shader> &handle)
    {
        return m_shaderPool->contains(handle);
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
            std::cout << data.vertexBindings[i].stride;
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

        m_vk->buildShader({
            .name = data.name,
            //.renderpass = data.render_compatibility.renderPass,
            .bindings = vkbindings,
            .attributes = vkattributes
        }, shader);
        return m_shaderPool->move_set(shader);
    }
};
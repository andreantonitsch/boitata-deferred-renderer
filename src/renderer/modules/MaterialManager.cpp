#include "MaterialManager.hpp"
#include <algorithm>
namespace boitatah{
    
    /// @brief TODO this is very inefficient
    // void MaterialGraph::orderGraph()
    // {
    //     std::vector<Handle<Material>> cur_mats;

    //     for(auto& k : m_nodes){
    //         cur_mats.push_back(k.first);
    //     }
            
    //     m_currentOrder.clear();
    //     m_currentOrder.push_back(Handle<Material>());

    //     int depth = 0;
    //     int i = 0;

    //     //we have to order all nodes
    //     while(!cur_mats.empty()){
    //         while(i < cur_mats.size()){
                
    //             auto& curr_mat= cur_mats[i];
    //             auto curr_node = m_nodes[curr_mat];

    //             if(curr_node.parent == *(m_currentOrder.end() - depth)){
    //                 m_currentOrder.push_back(curr_mat);
    //                 cur_mats.erase(std::find(cur_mats.begin(), cur_mats.end(), curr_mat));

    //                 i = 0;
    //                 depth = 0;
    //             }else{
    //                 i++;
    //             }
    //         }
    //         depth++;
    //     }
    //     m_currentOrder.erase(m_currentOrder.begin());
    //     m_dirty = false;
    // }    
    void MaterialGraph::orderGraph()
    {
        std::vector<Handle<Material>> cur_mats;

        for(auto& k : m_nodes){
            cur_mats.push_back(k.first);
        }
            
        m_currentOrder.clear();
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

    Handle<Material> MaterialManager::createMaterial(const MaterialCreate &description)
    {
        return Handle<Material>();
    }


    const std::vector<Handle<Material>>& MaterialManager::orderMaterials()
    {
        return m_materialGraph.getOrder();
    }
    Material& MaterialManager::getMaterialContent(const Handle<Material> &handle)
    {
        return m_materialPool->get(handle);
    }
    ShaderModule ShaderManager::compileShaderModule(const std::vector<char> &bytecode, std::string entryPoint)
    {
        return {.shaderModule = m_vk->createShaderModule(bytecode), .entryFunction = entryPoint};
    }
    Handle<Shader> ShaderManager::makeShader(const MakeShaderDesc &data)
    {
        Shader shader{
            .name = data.name,
            .vert = compileShaderModule(data.vert.byteCode, data.vert.entryFunction),
            .frag = compileShaderModule(data.frag.byteCode, data.vert.entryFunction)};

        ShaderLayout layoutData = m_layoutPool->get(data.layout);



        return Handle<Shader>();
    }
};
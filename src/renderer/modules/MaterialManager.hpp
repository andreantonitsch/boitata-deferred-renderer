#pragma once

#include <types/Material.hpp>
#include <collections/Pool.hpp>
#include <types/Shader.hpp>
#include <vulkan/DescriptorPoolManager.hpp>
#include <vulkan/DescriptorSetTree.hpp>
#include <types/BttEnums.hpp>
#include <unordered_map>
#include <unordered_set>

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

        public:
            void compileVertShader();
            void compileFragShader();
            void compileShader();
            void makeShader(); //a shader is a pipeline
            void destroyShader();
            void reflectShader();
    };

    class MaterialManager{
        public:
            MaterialManager();
            ShaderManager& getShaderManager();
            Handle<Material> createMaterial(const MaterialCreate& description);
            void destroyMaterial(const Handle<Material>& handle);
            const std::vector<Handle<Material>>& orderMaterials();
            Material& getMaterialContent(const Handle<Material>& handle);
        private:
            std::unique_ptr<ShaderManager> m_shaderManager;
            MaterialGraph m_materialGraph;
            std::unique_ptr<Pool<Material>> m_materialPool;

            void inheritMaterial();

    };
};
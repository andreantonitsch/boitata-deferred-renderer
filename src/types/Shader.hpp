#ifndef BOITATAH_SHADER_HPP
#define BOITATAH_SHADER_HPP

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "../types/BttEnums.hpp"
#include "RenderTarget.hpp"


namespace boitatah
{

    struct BindingDesc{
        uint32_t binding = 0;
        DESCRIPTOR_TYPE type;
        STAGE_FLAG stages;
        uint32_t descriptorCount = 1;
    };
    struct DescriptorSetLayoutDesc{
        uint32_t m_set = 1;
        std::vector<BindingDesc> bindingDescriptors;
    };

    struct DescriptorSetLayout{
        VkDescriptorSetLayout layout;
    };

    // becomes a shaderlayout with 3 descriptor set layouts and push constants
    struct ShaderLayoutDesc{
        DescriptorSetLayoutDesc materialLayout;
    };

    struct ShaderLayoutDescVk{
        VkDescriptorSetLayout materialLayout;
        VkDescriptorSetLayout baseLayout;
    };

    struct ShaderLayout{
        VkPipelineLayout layout;
        VkDescriptorSetLayout setLayout;
    };


    struct ShaderModule{
        VkShaderModule shaderModule;
        std::string entryFunction = "main";
    };
    
    struct ShaderModules{
        ShaderModule vert;
        ShaderModule frag;
    };


    struct ShaderStage{
        std::vector<char> byteCode;
        std::string entryFunction = "main";
    };

    struct VertexAttribute{
        FORMAT format;
        uint32_t offset;
    };

    struct VertexBindings{
        uint32_t stride;
        std::vector<VertexAttribute> attributes;
    };

    struct ShaderDesc{
        //required arguments
        std::string name;
        ShaderStage vert;
        ShaderStage frag;

        // A compatible framebuffer.
        // sets the RenderPass for this PSO.
        Handle<RenderTarget> framebuffer; //optional
        Handle<ShaderLayout> layout;

        std::vector<VertexBindings> bindings;
    };

    struct ShaderDescVk{
        //required arguments
        std::string name;
        ShaderModule vert;
        ShaderModule frag;

        VkRenderPass renderpass;
        VkPipelineLayout layout;
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;
    };



    struct Shader
    {
        std::string name;
        ShaderModule vert;
        ShaderModule frag;
        
        VkPipeline pipeline;

    };

    struct Binding
    {
        uint32_t layout;
        uint32_t set;
        uint32_t binding;
        // Type
        // Data
    };

    //gets updated every frame
    struct FrameUniforms{
        glm::mat4 projection;
        glm::mat4 view;
        float time;
        float deltaTime;
    };

    //Needs a material level setter
    struct Uniform //a uniform buffer object
    {
        // GPU data
        Handle<BufferReservation> buffers[2]; // one for writing and one for reading
        DESCRIPTOR_TYPE type;
        void* data; //most current data version. align on 32/64
        uint32_t size;
        uint8_t reading  = 0; // one or zero
        uint8_t dirty;
        uint8_t read_only = 0; //only needs one buffer.

        public:
            void update(void* new_data, uint32_t new_size) { /*data = new_data;*/ size = new_size;  dirtify(); };
            void dirtify(){ dirty = 255u;};
            void clean_frame_index(int index) {dirty = dirty & ~(static_cast<uint8_t>(1) << index);};
            bool check_dirt(int index) { return  static_cast<uint8_t>(0) < (dirty & (static_cast<uint8_t>(1) << index)); };

    };

    struct Material {
        Handle<Shader> shader;
        std::vector<Uniform> uniforms;


    };


}

#endif // BOITATAH_SHADER_HPP
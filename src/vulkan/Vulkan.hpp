#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <cstring>
#include <optional>
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>

#include <vulkan/VulkanStructs.hpp>

#include <types/BttEnums.hpp>
#include <types/Shader.hpp>
#include <types/commands/CommandBuffer.hpp>
#include <types/RenderTarget.hpp>
#include <types/Memory.hpp>
#include <types/Image.hpp>
#include "CommandsVk.hpp"

//#include "VkCommandBufferWriter.hpp"


namespace boitatah::vk
{
    class WindowManager;
    
    ///Vulkan Instance.
    /// Manages the Vulkan base structures, and logical device.
    /// Manages, creates and destroys vulkan structures.
    /// finish_initialization requires a WindowManager object attached.
    class VulkanInstance : public std::enable_shared_from_this<VulkanInstance>
    {
        protected:
            VulkanInstance(VulkanOptions opts);
        public:
            
            //Creates a VulkanInstance
            static std::shared_ptr<VulkanInstance> create(VulkanOptions opts);
            // Vulkan();

            ~VulkanInstance(void);

            //Finishes initialization.
            // requires an attached WindowManager
            void finish_initialization();

            VkInstance get_instance() const;
            VkDevice get_device() const;
            VkPhysicalDevice get_physical_device() const;
            VkQueue get_transfer_queue() const;
            VkQueue get_present_queue() const;
            VkQueue get_graphics_queue() const;
            
            //Attaches a WindowManager to this VulkanInstance
            void attach_window(std::shared_ptr<WindowManager> window);

            const QueueFamilyIndices get_queuefamily_indices() const;
            QueueFamilyIndices find_queuefamilies(VkPhysicalDevice device) const;
            
            // Create Vulkan Objects
            VkShaderModule create_shadermodule(const std::vector<char> &bytecode) const;
            VkFramebuffer create_framebuffer(const FramebufferDescVk &desc) const;
            VkAttachmentDescription create_attachmentdescription(const AttachmentDesc &attDesc) const;
            VkRenderPass create_renderpass(const RenderPassDesc &desc) const;
            Image create_image(const ImageDesc &desc) const;
            VkImageView create_imageview(VkImage image, const ImageDesc &desc) const;
            VkPipelineLayout create_shaderlayout(const ShaderLayoutDescVk &desc) const;
            VkDescriptorSetLayout create_descriptorlayout(const DescriptorSetLayoutDesc &desc) const;
            VkFence create_fence(bool signaled) const;
            VkSemaphore create_semaphore() const;
            BufferVkData create_buffer(const BufferDescVk & desc) const;
            VkSampler create_sampler(const SamplerData& data) const;
            
            //Gets the buffer alignment and memory types.
            BufferVkData get_buffer_alignment_memorytype(const BufferDescVk & desc) const;
            //Allocates rendertarget synchronization structures
            RenderTargetSync allocate_render_sync_structures();

            //Build a VkPipeline and fills the Shader object.
            // requires shader to have shadermodules already filled.
            void build_shader(const ShaderDescVk &desc, Shader &shader);
            
            //Allocates vkDevice Memory.
            VkDeviceMemory allocate_memory(const MemoryDesc &desc) const;
            //Maps VkDeviceMemory to a void ptr. 
            void* map_memory(const MapMemoryVk &desc) const;
            //Unmaps VkMemory
            void unmap_memory(const UnmapMemoryVk &desc) const;

            //Binds VkDeviceMemory and VkImage together
            void bind_image_memory(VkDeviceMemory memory, VkImage image) const;

            //Copies a data pointer to a VkBuffer.
            //This function maps and unmaps memory.
            void data_copy_to_buffer(CopyToBufferVk op) const;
            //Copies data to a mapped memory address
            void copy_to_mapped_memory(const CopyMappedMemoryVk &op) const;

            uint32_t get_buffer_alignment(const VkBuffer buffer) const;


            //returns if frame was successfully presented;
            bool present(Image &swapchainImage,
                VkSwapchainKHR &swapchain,
                            uint32_t &scIndex,
                            const PresentCommandVk &command);

            //Allocates a CommandBuffer
            CommandBuffer allocate_commandbuffer(const CommandBufferDesc &desc);

            // Sync Methods
            //Waits for the rendertarget to finish rendering.
            void wait_for_frame(RenderTargetSync &bufferData);
            //Waits for the GPU to idle
            void wait_idle();
            //Waits for the fence
            void wait_for_fence(const VkFence &fence) const;
            //Resets fence to unflagged state
            void reset_fence(const VkFence &fence) const;
            //Checks the fence status
            bool check_fence_status(VkFence fence);

            //Destroys the VkPipeline, ShaderModules and supporting objects
            void destroy_shader(Shader &shader);
            //Destroys a VkRenderpass object
            void destroy_renderpass(Renderpass &pass);
            //Destroys a VkFramebuffer and supporting objects
            void destroy_framebuffer(RenderTarget &framebuffer);
            //Destroys the VkImage, VkImageView and supporting objects
            void destroy_image(Image image);
            //Destroys a ShaderLayout object
            void destroy_pipelinelayout(ShaderLayout &layout);
            //Destroys a RenderTarget synchronization object
            void destroy_rendertarget_sync(const RenderTargetSync &sync);
            //Destroys a Vulkan Buffer and supporting objects
            void destroy_buffer(BufferVkData buffer) const;
            //Destroys a VkFence
            void destroy_fence(VkFence fence);
            //Destroys a VkSemaphore
            void destroy_semaphore(VkSemaphore semaphore);
            //Destroys  a VkDescriptorPool
            void destroy_descriptorpool(VkDescriptorPool pool);
            //Destroys a VkDescriptorSetLayout
            void destroy_descriptorset_layout(VkDescriptorSetLayout &layout);
            //Destrpys a VkSampler
            void destroy_sampler(VkSampler& sampler);
            
        private:

            QueueFamilyIndices m_queue_family_indices;
            VulkanOptions m_options;
            std::shared_ptr<WindowManager> window;
            
            // Instances and Devices
            VkInstance instance;
            VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
            VkPhysicalDeviceProperties m_device_properties;
            VkDebugUtilsMessengerEXT m_debug_messenger;
            // window::WindowManager *window;
            
            // Logical Device
            VkDevice m_device; 
            // Queues and Pools
            CommandPools m_command_pools;
            CommandQueues m_queues;

            // Extensions and Layers
            std::vector<const char *> m_validation_layers;
            std::vector<const char *> m_device_extensions;
            std::vector<const char *> m_instance_extensions;

    #pragma region Vulkan Setup

            void init_instance();

            // Extensions
            boitatah::vk::SwapchainSupport get_swapchain_support(VkPhysicalDevice physicalDevice);
            bool check_required_extensions(const std::vector<VkExtensionProperties> &available,
                                        const std::vector<const char *> &required);
            std::vector<VkExtensionProperties> retrieve_available_extensions();

            // Validation Layers
            void init_debug_messenger();
            bool check_validation_support(const std::vector<const char *> &layers);
            void populate_messenger(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

            // Device
            int eval_physical_device(VkPhysicalDevice device);
            bool check_device_ext_support(VkPhysicalDevice device);
            void init_physical_device();
            void init_logical_device_queues();

            // Queues

            void set_queues();
            void create_commandpools();


            uint32_t find_memory_index(const MemoryDesc &props) const;

    #pragma endregion Vulkan Setup
    };

}

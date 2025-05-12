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

#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../types/commands/CommandBuffer.hpp"
#include "../types/RenderTarget.hpp"
#include "../types/Memory.hpp"
#include "../types/Image.hpp"
#include "CommandsVk.hpp"

//#include "VkCommandBufferWriter.hpp"


namespace boitatah::vk
{
    class WindowManager;
    

    class Vulkan : public std::enable_shared_from_this<Vulkan>
    {
    public:
        Vulkan(VulkanOptions opts);
        
        static std::shared_ptr<Vulkan> create(VulkanOptions opts);
        // Vulkan();

        ~Vulkan(void);
        void completeInit();

        VkInstance getInstance() const;
        VkDevice getDevice() const;
        VkPhysicalDevice getPhysicalDevice();
        VkQueue getTransferQueue();
        VkQueue getPresentQueue();
        VkQueue getGraphicsQueue();
        void attachWindow(std::shared_ptr<WindowManager> window);

        QueueFamilyIndices familyIndices;

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        // Create Objects
        VkShaderModule createShaderModule(const std::vector<char> &bytecode);
        VkFramebuffer createFramebuffer(const FramebufferDescVk &desc);
        VkAttachmentDescription createAttachmentDescription(const AttachmentDesc &attDesc);
        VkRenderPass createRenderPass(const RenderPassDesc &desc);
        Image createImage(const ImageDesc &desc);
        VkImageView createImageView(VkImage image, const ImageDesc &desc);
        VkPipelineLayout createShaderLayout(const ShaderLayoutDescVk &desc);
        VkDescriptorSetLayout createDescriptorLayout(const DescriptorSetLayoutDesc &desc);
        VkFence createFence(bool signaled);
        VkSemaphore createSemaphore() const;
        BufferVkData createBuffer(const BufferDescVk & desc) const;
        VkSampler createSampler(const SamplerData& data);


        BufferVkData getBufferAlignmentMemoryType(const BufferDescVk & desc) const;
        RenderTargetSync allocateBufferSync();
        void buildShader(const ShaderDescVk &desc, Shader &shader);

        // Manage Memory
        VkDeviceMemory allocateMemory(const MemoryDesc &desc);
        void* mapMemory(const MapMemoryVk &desc) const;
        void unmapMemory(const UnmapMemoryVk &desc) const;
        void bindImageMemory(VkDeviceMemory memory, VkImage image);
        void mapDataAndCopyToBuffer(CopyToBufferVk op);
        void copyToMappedMemory(const CopyMappedMemoryVk &op) const;

        uint32_t getAlignmentForBuffer(const VkBuffer buffer) const;


        //returns if frame was successfully presented;
        bool presentFrame(Image &swapchainImage,
                          VkSwapchainKHR &swapchain,
                          uint32_t &scIndex,
                          const PresentCommandVk &command);

#pragma region Commands

        // Generic Commands
        CommandBuffer allocateCommandBuffer(const CommandBufferDesc &desc);
        void resetCmdBuffer(const VkCommandBuffer buffer);
#pragma endregion Commands


        // Sync Methods
        void waitForFrame(RenderTargetSync &bufferData);
        void waitIdle();
        void waitForFence(const VkFence &fence) const;
        void reset_fence(const VkFence &fence) const;
        bool checkFenceStatus(VkFence fence);

        // Destroy Objects
        void destroyShader(Shader &shader);
        void destroyRenderpass(RenderPass &pass);
        void destroyFramebuffer(RenderTarget &framebuffer);
        void destroyImage(Image image);
        void destroyPipelineLayout(ShaderLayout &layout);
        void destroyRenderTargetCmdData(const RenderTargetSync &sync);
        void destroyBuffer(BufferVkData buffer) const;
        void destroyFence(VkFence fence);
        void destroySemaphore(VkSemaphore semaphore);
        void destroyDescriptorPool(VkDescriptorPool pool);
        void destroyDescriptorSetLayout(VkDescriptorSetLayout &layout);
        void destroySampler(VkSampler& sampler);

    private:

        VulkanOptions options;
        std::shared_ptr<WindowManager> window;

        // Instances and Devices
        VkInstance instance;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties deviceProperties;
        VkDebugUtilsMessengerEXT debugMessenger;
        // window::WindowManager *window;

        VkDevice device; // Logical Device
        // Queues and Pools
        CommandPools commandPools;
        CommandQueues queues;

        //std::shared_ptr<VkCommandBufferWriter> m_commandBufferWriter;
        //void createCommandBufferWriter();

        // Extensions and Layers
        std::vector<const char *> validationLayers;
        std::vector<const char *> deviceExtensions;
        std::vector<const char *> instanceExtensions;

        void beginCommands(const VkCommandBuffer &buffer);
        void endTransferCommands(const VkCommandBuffer &buffer,
                         const VkQueue &queue,
                         const std::vector<VkSemaphore> &wait,
                         const VkSemaphore &signal,
                         const VkFence &fence);

        // Deprecated. use commandbufferwriter instead
        void CmdCopyImage(const CopyImageCommandVk &command);
        // Deprecated. use commandbufferwriter instead
        void CmdTransitionLayout(const TransitionLayoutCmdVk &command);

#pragma region Vulkan Setup

        void initInstance();

        // Extensions
        boitatah::vk::SwapchainSupport getSwapchainSupport(VkPhysicalDevice physicalDevice);
        bool checkRequiredExtensions(const std::vector<VkExtensionProperties> &available,
                                     const std::vector<const char *> &required);
        std::vector<VkExtensionProperties> retrieveInstanceAvailableExtensions();

        // Validation Layers
        void initializeDebugMessenger();
        bool checkValidationLayerSupport(const std::vector<const char *> &layers);
        void populateMessenger(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

        // Device
        int evaluatePhysicalDevice(VkPhysicalDevice device);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        void initPhysicalDevice();
        void initLogicalDeviceNQueues();

        // Queues

        void setQueues();
        void createCommandPools();


        uint32_t findMemoryIndex(const MemoryDesc &props) const;

#pragma endregion Vulkan Setup
    };

}

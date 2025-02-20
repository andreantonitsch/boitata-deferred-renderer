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
        BufferVkData getBufferAlignmentMemoryType(const BufferDescVk & desc) const;

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
        bool presentFrame(Image &image,
                          Image &swapchainImage,
                          VkSwapchainKHR &swapchain,
                          uint32_t &scIndex,
                          const PresentCommandVk &command);

#pragma region Commands


        //std::shared_ptr<VkCommandBufferWriter> getCommandBufferWriter();
        

        // Generic Commands
        CommandBuffer allocateCommandBuffer(const CommandBufferDesc &desc);
        void beginCmdBuffer(const BeginCommandVk &command);
        void resetCmdBuffer(const VkCommandBuffer buffer);
        void submitCmdBuffer(const SubmitCommandVk &command);

        // Render Commands
        //void beginBufferCommand(const BeginCommandVk &command);
        void beginRenderpassCommand(const BeginRenderpassCommandVk &command);
        void endRenderpassCommand(const EndRenderpassCommandVk &command);
        void recordDrawCommand(const DrawCommandVk &command);
        void submitDrawCmdBuffer(const SubmitDrawCommandVk &command);
        void bindPipelineCommand(const BindPipelineCommandVk &command);
        void bindDescriptorSet(const BindDescriptorSetCommandVk& command);

        // Transfer Commands
        void CmdCopyImage(const CopyImageCommandVk &command);
        void CmdCopyBuffer(const CopyBufferCommandVk &command) const;
        void CmdTransitionLayout(const TransitionLayoutCmdVk &command);

        void CmdBindVertexBuffers(const BindBuffersCommandVk &command);
        void CmdBindIndexBuffer(const BindBuffersCommandVk &command);

#pragma endregion Commands


        // Sync Methods
        void waitForFrame(RenderTargetSync &bufferData);
        void waitIdle();
        void waitForFence(const VkFence &fence) const;
        bool checkFenceStatus(VkFence fence) ;

        // Destroy Objects
        void destroyShader(Shader &shader);
        void destroyRenderpass(RenderPass &pass);
        void destroyFramebuffer(RenderTarget &framebuffer);
        void destroyImage(Image image);
        void destroyPipelineLayout(ShaderLayout &layout);
        void destroyRenderTargetCmdData(const RenderTargetSync &sync);
        void destroyBuffer(BufferVkData buffer) const;
        void destroyFence(VkFence fence);
        void destroyDescriptorPool(VkDescriptorPool pool);
        void destroyDescriptorSetLayout(VkDescriptorSetLayout &layout);
        // Copy assignment?
        // Vulkan& operator= (const Vulkan &v);//copy assignment


    private:

        VulkanOptions options;
        std::shared_ptr<WindowManager> window;

        // Instances and Devices
        VkInstance instance;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
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
        void endCommands(const VkCommandBuffer &buffer,
                         const VkQueue &queue,
                         const VkSemaphore &wait,
                         const VkSemaphore &signal,
                         const VkFence &fence);

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

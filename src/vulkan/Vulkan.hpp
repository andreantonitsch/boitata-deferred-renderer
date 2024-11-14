#ifndef BOITATAH_VK_VULKAN_HPP
#define BOITATAH_VK_VULKAN_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../types/commands/CommandBuffer.hpp"
#include "../types/RenderTarget.hpp"
#include "../types/Memory.hpp"
#include "../types/Image.hpp"
#include "CommandsVk.hpp"
#include "../renderer/modules/Window.hpp"

namespace boitatah::vk
{
    struct BufferDescVk
    {
        // uint32_t alignment;
        uint32_t size;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };

    struct BufferVkData{
        VkBuffer buffer;
        VkDeviceMemory memory;
        uint64_t alignment;
        uint64_t actualSize;

        uint32_t memoryTypeBits;

    };

    struct CommandPools
    {
        VkCommandPool graphicsPool;
        VkCommandPool transferPool;
        VkCommandPool presentPool;
    };

    struct CommandQueues
    {
        VkQueue graphicsQueue;
        VkQueue transferQueue;
        VkQueue presentQueue;
    };

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;

        bool hasFullSupport()
        {
            return graphicsFamily.has_value() &&
                   presentFamily.has_value() &&
                   transferFamily.has_value();
        }
    };

    struct SwapchainSupport
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct VulkanOptions
    {
        char *appName = nullptr;
        std::vector<const char *> extensions;
        bool useValidationLayers = false;
        bool debugMessages = false;

        // TODO glfw requires instance to create a surface.
        // I would like to keep both things separate.
        GLFWwindow *window;
    };

    class Vulkan
    {
    public:
        // Vulkan();
        Vulkan(VulkanOptions opts);
        ~Vulkan(void);
        void completeInit();

        VkInstance getInstance() const;
        VkDevice getDevice() const;
        VkPhysicalDevice getPhysicalDevice();
        void attachWindow(boitatah::window::WindowManager *window);

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
        VkSemaphore createSemaphore();
        BufferVkData createBuffer(const BufferDescVk & desc) const;
        BufferVkData getBufferAlignmentMemoryType(const BufferDescVk & desc) const;

        VkDescriptorPool createSetPool();
        void resetPool(const VkDescriptorPool pool);


        void buildShader(const ShaderDescVk &desc, Shader &shader);

        // Manage Memory
        VkDeviceMemory allocateMemory(const MemoryDesc &desc);
        void bindImageMemory(VkDeviceMemory memory, VkImage image);
        void copyDataToBuffer(CopyToBufferVk op);

        uint32_t getAlignmentForBuffer(const VkBuffer buffer) const;


        //returns if frame was successfully presented;
        bool presentFrame(Image &image,
                          Image &swapchainImage,
                          VkSwapchainKHR &swapchain,
                          uint32_t &scIndex,
                          const PresentCommandVk &command);

#pragma region Commands

        // Generic Commands
        VkCommandBuffer allocateCommandBuffer(const CommandBufferDesc &desc);
        void beginCmdBuffer(const BeginCommandVk &command);
        void resetCmdBuffer(const VkCommandBuffer buffer);
        void submitCmdBuffer(const SubmitCommandVk &command);

        // Render Commands
        //void beginBufferCommand(const BeginCommandVk &command);
        void beginRenderpassCommand(const BeginRenderpassCommandVk &command);
        void recordDrawCommand(const DrawCommandVk &command);
        void submitDrawCmdBuffer(const SubmitDrawCommandVk &command);
        void bindPipelineCommand(const BindPipelineCommandVk &command);


        // Transfer Commands
        void CmdCopyImage(const CopyImageCommandVk &command);
        void CmdCopyBuffer(const CopyBufferCommandVk &command);
        void CmdTransitionLayout(const TransitionLayoutCmdVk &command);

#pragma endregion Commands


        // Sync Methods
        void waitForFrame(RenderTargetCmdBuffers &bufferData);
        void waitIdle();
        void waitForFence(const VkFence &fence) const;
        bool checkFenceStatus(const VkFence &fence) const;

        // Destroy Objects
        void destroyShader(Shader &shader);
        void destroyRenderpass(RenderPass &pass);
        void destroyFramebuffer(RenderTarget &framebuffer);
        void destroyImage(Image image);
        void destroyPipelineLayout(ShaderLayout &layout);
        void destroyRenderTargetCmdData(const RenderTargetCmdBuffers &sync);
        void destroyBuffer(BufferVkData buffer) const;
        void destroyFence(VkFence fence);
        void destroyDescriptorPool(VkDescriptorPool pool);
        // Copy assignment?
        // Vulkan& operator= (const Vulkan &v);//copy assignment


    private:
        VulkanOptions options;
        window::WindowManager *window;

        // Instances and Devices
        VkInstance instance;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger;
        // window::WindowManager *window;

        VkDevice device; // Logical Device

        // Queues and Pools
        CommandPools commandPools;
        CommandQueues queues;

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

#endif // BOITATAH_VK_VULKAN_HPP
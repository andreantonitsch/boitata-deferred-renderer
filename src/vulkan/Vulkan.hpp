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
#include "../types/RenderTarget.hpp"
#include "../types/Memory.hpp"
#include "../types/Image.hpp"
#include "../types/CommandBuffer.hpp"
//#include "../types/Buffer.hpp"
// #include "../types/Swapchain.hpp"
#include "../renderer/Window.hpp"

namespace boitatah::vk
{

    struct BufferDescVk
    {
        // uint32_t alignment;
        uint32_t size;
        BUFFER_USAGE usage;
        SHARING_MODE sharing;
    };

    struct BufferVkObjects{
        VkBuffer buffer;
        VkDeviceMemory memory;
        uint32_t alignment;
        uint32_t actualSize;
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

        VkInstance getInstance();
        VkDevice getDevice();
        VkPhysicalDevice getPhysicalDevice();
        void attachWindow(boitatah::window::WindowManager *window);


        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        // Create Objects
        VkShaderModule createShaderModule(const std::vector<char> &bytecode);
        VkFramebuffer createFramebuffer(const FramebufferDescVk &desc);
        VkAttachmentDescription createAttachmentDescription(const AttachmentDesc &attDesc);
        VkRenderPass createRenderPass(const RenderPassDesc &desc);
        Image createImage(const ImageDesc &desc);
        VkImageView createImageView(VkImage image, const ImageDesc &desc);
        VkPipelineLayout createPipelineLayout(const PipelineLayoutDesc &desc);
        VkFence createFence(bool signaled);
        VkSemaphore createSemaphore();
        BufferVkObjects createBuffer(const BufferDescVk & desc) const;
        void buildShader(const ShaderDescVk &desc, Shader &shader);

        // Manage Memory
        VkDeviceMemory allocateMemory(const MemoryDesc &desc);
        void bindImageMemory(VkDeviceMemory memory, VkImage image);
        void copyDataToBuffer(CopyToBufferOp op);

        uint32_t getAlignmentForBuffer(const VkBuffer buffer) const;

        // Generic Commands
        VkCommandBuffer allocateCommandBuffer(const CommandBufferDesc &desc);
        void resetCommandBuffer(const VkCommandBuffer buffer);

        // Render Commands
        void recordCommand(const DrawCommandVk &command);
        void submitDrawCmdBuffer(const SubmitCommand &command);

        //returns if frame was successfully presented;
        bool presentFrame(Image &image,
                          Image &swapchainImage,
                          VkSwapchainKHR &swapchain,
                          uint32_t &scIndex,
                          SubmitCommand &command);

        // Transfer Commands
        void CmdCopyImage(const CopyImageCommandVk &command);

        // Sync Methods
        void waitForFrame(RTCmdBuffers &bufferData);
        void waitIdle();

        // Destroy Objects
        void destroyShader(Shader &shader);
        void destroyRenderpass(RenderPass &pass);
        void destroyFramebuffer(RenderTarget &framebuffer);
        void destroyImage(Image image);
        void destroyPipelineLayout(PipelineLayout &layout);
        void destroyRenderTargetCmdData(const RTCmdBuffers &sync);
        void destroyBuffer(BufferVkObjects buffer) const;
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

        // Sync Objects
        // VkSemaphore SemImageAvailable;
        // VkSemaphore SemRenderFinished;
        // VkSemaphore SemTransferComplete;
        // VkFence FenInFlight;
        // VkFence FenTransferSwapchain;

        // Extensions and Layers
        std::vector<const char *> validationLayers;
        std::vector<const char *> deviceExtensions;
        std::vector<const char *> instanceExtensions;

#pragma region Commands

        // void beginOneshotCommands(const BufferType)
        // void endOneshotCommands(const BufferType)

        void beginCommands(const VkCommandBuffer &buffer);
        void endCommands(const VkCommandBuffer &buffer,
                         const VkQueue &queue,
                         VkSemaphore &wait,
                         VkSemaphore &signal,
                         VkFence &fence);
        void CmdTransitionLayout(const TransitionLayoutCmdVk &command);

#pragma endregion Commands

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
#include <vulkan/vulkan.h>
#include <vector>

namespace boitatah::vk
{

    struct VulkanOptions
    {
        char *appName = nullptr;
        const char **extensions;
        uint32_t extensionsCount = 0;
    };

    class Vulkan
    {
    public:
        Vulkan();
        Vulkan(VulkanOptions opts);
        ~Vulkan(void);

        // Copy assignment?
        // Vulkan& operator= (const Vulkan &v);//copy assignment
    private:
        VkInstance instance;
        VulkanOptions options;

        // Clean up
        void cleanup();

        // Extensions
        bool checkRequiredExtensions(std::vector<VkExtensionProperties> available,
                                     const char **required,
                                     uint32_t requiredCount);
        std::vector<VkExtensionProperties> retrieveAvailableExtensions();
        // bool validateExtensions(const char* extensions);
    };

}
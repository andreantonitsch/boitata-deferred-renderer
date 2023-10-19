#ifndef BOITATAH_VK_VULKAN_HPP
#define BOITATAH_VK_VULKAN_HPP


#include <vulkan/vulkan.h>
#include <vector>

namespace boitatah::vk
{

    struct VulkanOptions
    {
        char *appName = nullptr;
        std::vector<const char*> extensions;
        bool useValidationLayers = false;
    };

    class Vulkan
    {
    public:
        //Vulkan();
        Vulkan(VulkanOptions opts);
        ~Vulkan(void);

        // Copy assignment?
        // Vulkan& operator= (const Vulkan &v);//copy assignment
    private:
        VkInstance instance;
        VulkanOptions options;
        VkDebugUtilsMessengerEXT debugMessenger;

        // Extensions
        bool checkRequiredExtensions(const std::vector<VkExtensionProperties> &available,
                                     const std::vector<const char*> &required);
        std::vector<VkExtensionProperties> retrieveAvailableExtensions();
        // bool validateExtensions(const char* extensions);

        // Validation Layers
        void initializeDebugMessenger();
        bool checkValidationLayerSupport(const std::vector<const char *> &layers);
        void populateMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    };

}

#endif //BOITATAH_VK_VULKAN_HPP
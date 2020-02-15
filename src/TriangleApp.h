/**
  * Created by Earl Kennedy
  * https://github.com/Mnenmenth
  */

#ifndef VULKANTUTORIAL_TRIANGLEAPP_H
#define VULKANTUTORIAL_TRIANGLEAPP_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>

#include "types.h"
#include <optional>

/**
 *
 * Tutorial last left of at:
 * https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Choosing-the-right-settings-for-the-swap-chain
 *
 */

class TriangleApp
{
public:
    static constexpr int WIN_WIDTH = 800;
    static constexpr int WIN_HEIGHT = 600;

    auto run() -> void;

private:
    GLFWwindow* window;
    VkSurfaceKHR surface;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    // Handles to swap chain images
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    // Image format and extent chosen in initialization
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    // Required validation layers
    const std::vector<type::cstr> validationLayers =
            {
                "VK_LAYER_KHRONOS_validation"
            };
    // Required device extensions
    const std::vector<type::cstr> deviceExtensions =
            {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };
#ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
    static constexpr bool enableValidationLayers = true;
#endif

    auto initWindow() -> void;

    static auto debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
            ) -> VKAPI_ATTR VkBool32 VKAPI_CALL;
    // Utility function to fill info struct
    static auto populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) -> void;
    auto setupDebugMessenger() -> void;

    // Check if validation layers are supported
    auto checkValidationLayerSupport() -> bool;
    // Get vulkan extensions required to run
    auto getRequiredExtensions() -> std::vector<type::cstr>;

    // Hold the indices for the queue family from the list of queue families found
    struct QueueFamilyIndices
    {
        // Drawing command support
        std::optional<type::uint32> graphicsFamily;
        // Presentation command support (displaying to a surface)
        std::optional<type::uint32> presentFamily;
        inline auto isComplete() -> bool { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };
    // Check if the device supports the type of commands we want to send
    auto findQueueFamilies(VkPhysicalDevice device) -> QueueFamilyIndices;

    // Details about swap chain support for a device
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        // List of available color depths and formats
        std::vector<VkSurfaceFormatKHR> formats;
        // Conditions for presenting images to the screen/how they're queued to be presented
        std::vector<VkPresentModeKHR> presentModes;
    };
    // Populate struct with support details
    auto querySwapChainSupport(VkPhysicalDevice device) -> SwapChainSupportDetails;
    auto chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) -> VkSurfaceFormatKHR;
    auto chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) -> VkPresentModeKHR;
    // Choose the swap extent that matches the window resolution
    auto chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) -> VkExtent2D;

    // Check if a given device supports the required extensions
    auto checkDeviceExtensionSupport(VkPhysicalDevice device) -> bool;
    // Rate the device based on type and available features
    auto ratePhysicalDevice(VkPhysicalDevice device) -> int;
    // Pick the device we want to use
    auto pickPhysicalDevice() -> void;

    auto createSwapChain() -> void;
    auto createImageViews() -> void;
    auto createLogicalDevice() -> void;
    auto createInstance() -> void;
    auto createSurface() -> void;
    auto initVulkan() -> void;

    auto mainLoop() -> void;
    auto cleanup() -> void;
};


#endif //VULKANTUTORIAL_TRIANGLEAPP_H
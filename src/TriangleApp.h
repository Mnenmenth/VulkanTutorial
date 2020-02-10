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
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkDebugUtilsMessengerEXT debugMessenger;

    const std::vector<type::cstr> validationLayers =
            {
                "VK_LAYER_KHRONOS_validation"
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
    // Rate the device based on type and available features
    auto ratePhysicalDevice(VkPhysicalDevice device) -> int;
    // Pick the device we want to use
    auto pickPhysicalDevice() -> void;

    auto createLogicalDevice() -> void;

    auto createInstance() -> void;
    auto createSurface() -> void;
    auto initVulkan() -> void;

    auto mainLoop() -> void;
    auto cleanup() -> void;
};


#endif //VULKANTUTORIAL_TRIANGLEAPP_H

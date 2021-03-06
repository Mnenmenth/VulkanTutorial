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
#include "Vertex.h"
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
    // Allow graphics pipeline to work on rendering
        // more images before image is done being presented
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    auto run() -> void;

private:

#ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
    static constexpr bool enableValidationLayers = true;
#endif

/*
 * Window Initialization
 */
    GLFWwindow* window;
    auto initWindow() -> void;
    static auto framebufferResizeCallback(GLFWwindow* window, int width, int height) -> void;
    /*
 * Vulkan Initialization
 */
    auto initVulkan() -> void;

/* Instance Creation */
    VkInstance instance;
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
    // Check if validation layers are supported
    auto checkValidationLayerSupport() -> bool;
    // Get vulkan extensions required to run
    auto getRequiredExtensions() -> std::vector<type::cstr>;
    auto createInstance() -> void;

/* Debug Messenger Setup */
    VkDebugUtilsMessengerEXT debugMessenger;
    static auto debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
    ) -> VKAPI_ATTR VkBool32 VKAPI_CALL;
    // Utility function to fill info struct
    static auto populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) -> void;
    auto setupDebugMessenger() -> void;

/* Surface Creation */
    VkSurfaceKHR surface;
    auto createSurface() -> void;

/* Logical and Physical Device Creation */
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice;
    // Check if a given device supports the required extensions
    auto checkDeviceExtensionSupport(VkPhysicalDevice device) -> bool;
    // Rate the device based on type and available features
    auto ratePhysicalDevice(VkPhysicalDevice device) -> int;
    // Pick the device we want to use
    auto pickPhysicalDevice() -> void;
    auto createLogicalDevice() -> void;

/* Queue Family Setup */
    VkQueue graphicsQueue;
    VkQueue presentQueue;
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

/* Swap Chain Creation */
    VkSwapchainKHR swapChain;
    // Handles to swap chain images
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    // Image format and extent chosen in initialization
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
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
    auto createSwapChain() -> void;
    auto createImageViews() -> void;
    auto cleanupSwapchain() -> void;
    // For recreating the swap chain in the event of something like a window resize
    auto recreateSwapChain() -> void;


/* Render Pass Setup */
    VkRenderPass renderPass;
    auto createRenderPass() -> void;

/* Graphics Pipeline Creation */
    VkPipeline graphicsPipeline;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    static auto readFile(const std::string& fileName, std::vector<char>& buffer) -> std::vector<char>;
    auto createShaderModule(const std::vector<char>& code) -> VkShaderModule;
    auto createDescriptorSetLayout() -> void;
    auto createGraphicsPipeline() -> void;

/* Swap Chain Framebuffers Creation */
    std::vector<VkFramebuffer> swapChainFramebuffers;
    auto createFramebuffers() -> void;

/* Command Pool Creation */
    //** Command pools manage memory for buffers and command buffers
    VkCommandPool commandPool;
    auto createCommandPool() -> void;

/* Buffer Creation */
    static constexpr std::array<Vertex, 4> vertices =
            {
                    Vertex{{-0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}},
                    Vertex{{ 0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}},
                    Vertex{{ 0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},
                    Vertex{{-0.5f,  0.5f}, { 1.0f,  1.0f,  1.0f}}

            };
    // uint32 or uint16 can be used here, but uint16 is being used for now because
        // there are less than 65535 unique vertices
    static constexpr std::array<type::uint16, 6> indices = {0, 1, 2, 2, 3, 0};
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBufferMemories;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    // Find memory specification and layout of GPU
    auto findMemoryType(type::uint32 typeFilter, VkMemoryPropertyFlags properties) -> type::uint32;
    auto copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) -> void;
    auto createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& buffer, VkDeviceMemory &bufferMemory) -> void;
    auto createVertexBuffer() -> void;
    auto createIndexBuffer() -> void;
    auto createUniformBuffers() -> void;
    // Allocate pool of descriptors from which to bind uniform buffers
    auto createDescriptorPool() -> void;
    auto createDescriptorSets() -> void;

/* Command Buffer Allocation */
    std::vector<VkCommandBuffer> commandBuffers;
    auto createCommandBuffers() -> void;

/* Semaphore and Fence Creation - For syncing command buffers */
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    type::size currentFrame = 0;
    bool framebufferResized = false;
    auto createSyncObjects() -> void;


/*
 * Application Maintenance
 */
    // Get image from swap chain,
    // Execute image as attachment for framebuffer
    // Return image to swap chain for presentation
    auto updateUniformBuffer(type::uint32 currImg) -> void;
    auto drawFrame() -> void;
    auto mainLoop() -> void;
    auto cleanup() -> void;
};


#endif //VULKANTUTORIAL_TRIANGLEAPP_H
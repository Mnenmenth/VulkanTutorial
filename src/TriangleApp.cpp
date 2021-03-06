/**
  * Created by Earl Kennedy
  * https://github.com/Mnenmenth
  */

#include <stdexcept>
#include <iostream>
#include <cstring>
#include <map>
#include <set>
#include <fstream>
#include "TriangleApp.h"
#include "Vertex.h"
#include "UBO.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

auto TriangleApp::run() -> void
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

/**
 *
 * Window Initialization
 *
 */
auto TriangleApp::initWindow() -> void
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Vulkan App", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

}

auto TriangleApp::framebufferResizeCallback(GLFWwindow *window, int width, int height) -> void
{
    auto app = reinterpret_cast<TriangleApp*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

/**
 *
 * Vulkan Initialization
 *
 */
auto TriangleApp::initVulkan() -> void
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

/**
 * Instance Creation
 */

auto TriangleApp::checkValidationLayerSupport() -> bool
{
    type::uint32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // Check if all of the requested layers are available
    for(type::cstr layerName : validationLayers)
    {
        bool layerFound = false;

        for(const auto& layerProperties : availableLayers)
        {
            if(strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if(!layerFound)
        {
            return false;
        }
    }

    return true;
}

auto TriangleApp::getRequiredExtensions() -> std::vector<type::cstr>
{
    type::uint32 glfwExtensionCount = 0;
    type::cstr* glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<type::cstr> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if(enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

auto TriangleApp::createInstance() -> void
{
    if(enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("Validation layers requested but not available");
    }

    // Specify application info
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Triangle App";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Specify instance info
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();

    // Enable required extensions in instance
    createInfo.enabledExtensionCount = static_cast<type::uint32>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if(enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<type::uint32>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    // Create the instance
    if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Vulkan instance creation failed");
    }
}

/**
 * Debug Messenger Setup
 */

auto CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger
) -> VkResult
{

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if(func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

auto DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator
)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

auto TriangleApp::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                VkDebugUtilsMessageTypeFlagsEXT messageType,
                                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData
) -> VKAPI_ATTR VkBool32 VKAPI_CALL
{
    std::cerr << "Validation Layer Debug: \n\t" << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

auto TriangleApp::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) -> void
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

auto TriangleApp::setupDebugMessenger() -> void
{
    if(!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to setup debug messenger");
    }
}

/**
 * Surface Creation
 */
auto TriangleApp::createSurface() -> void
{
    if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Window surface creation failed");
    }
}

/**
 * Logical Device Creation
 */
auto TriangleApp::checkDeviceExtensionSupport(VkPhysicalDevice device) -> bool
{
    // Get number of extensions supported by device
    type::uint32 extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    // Get extensions supported by device
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    // Set of the required extensions
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    // Iterate through the available extensions and make sure that
    // all required extensions are present
    for(const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

auto TriangleApp::ratePhysicalDevice(VkPhysicalDevice device) -> int
{
    // The device is not suitable if it doesn't
    // support the required queue families
    // and extensions
    QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    if(!indices.isComplete() || !extensionsSupported)
    {
        return 0;
    }

    // The device is not suitable if there is not at
    // least one format and present mode
    // ** Important: This check must be made after verifying
    // that the extensions are supported **
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    if(swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
    {
        return 0;
    }

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    int score = 0;

    // Discrete GPU is highly preferred over integrated
    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    // Higher texture size maximum affects quality of graphics
    score += deviceProperties.limits.maxImageDimension2D;

    return score;
}

auto TriangleApp::pickPhysicalDevice() -> void
{
    type::uint32 deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if(deviceCount == 0)
    {
        throw std::runtime_error("No GPU's with Vulkan support available");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    for(const auto& device : devices)
    {
        candidates.insert(std::make_pair(ratePhysicalDevice(device), device));
    }

    if(candidates.rbegin()->first > 0)
    {
        physicalDevice = candidates.rbegin()->second;
    }
    else
    {
        throw std::runtime_error("GPU('s) found, but none are suitable");
    }
}

auto TriangleApp::createLogicalDevice() -> void
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    // From here to end of for loop is for creating create info struct
    // for each type of queue family
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<type::uint32> uniqueQueueFamililies =
            {
                    indices.graphicsFamily.value(),
                    indices.presentFamily.value()
            };

    float queuePriority = 1.0f;
    for(type::uint32 queueFamily : uniqueQueueFamililies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    /// This will be used later on
    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<type::uint32>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<type::uint32>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if(enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<type::uint32>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create the logical device");
    }

    // Get handle for the graphics queue
    vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
    // Get handle for the presentation queue
    vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}

/**
 * Queue Family Setup
 */
auto TriangleApp::findQueueFamilies(VkPhysicalDevice device) -> TriangleApp::QueueFamilyIndices
{
    QueueFamilyIndices indices;

    type::uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // Iterate through found queue families until one is found
    // that supports what we need
    //** Note: Yes the double if statement is currently redundant, but apparently it will
    // become more necessary later in the tutorial
    int i = 0;
    for(const auto& queueFamily : queueFamilies)
    {
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        // Does the device support presentation to a surface
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if(presentSupport)
        {
            indices.presentFamily = i;
        }

        if(indices.isComplete())
        {
            break;
        }
        ++i;
    }

    return indices;
}

/**
 * Swap Chain Creation
 */
auto TriangleApp::querySwapChainSupport(VkPhysicalDevice device) -> TriangleApp::SwapChainSupportDetails
{
    SwapChainSupportDetails details;

    // Get capabilities, taking both the device and surface into account
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    // Query number of supported surface formats
    type::uint32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    // Actually get supported surface formats if there are any
    if(formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // Query number of supported presentation modes
    type::uint32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    // Actually get supported presentation modes if there are any
    if(presentModeCount != 0)
    {
        details.presentModes.resize(formatCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }



    return details;
}

auto TriangleApp::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) -> VkSurfaceFormatKHR
{
    // Try to find support for 8 bit SRGB and use it if exists
    for(const auto& availableFormat : availableFormats)
    {
        if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    // Otherwise just settle for the first available surface format
    return availableFormats[0];
}

auto TriangleApp::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) -> VkPresentModeKHR
{
    // Mailbox mode is preferred, so try to find that first
    // MAILBOX (triple buffering) is uses a queue to present images,
    // and if the queue is full already queued images are overwritten with newer images
    for(const auto& availablePresentMode : availablePresentModes)
    {
        if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    // Otherwise default to the only mode guaranteed to be supported
    // FIFO is basically traditional "vertical sync"
    return VK_PRESENT_MODE_FIFO_KHR;
}

auto TriangleApp::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) -> VkExtent2D
{
    // Vulkan says to match resolution window
    if(capabilities.currentExtent.width != type::uint32_max)
    {
        return capabilities.currentExtent;
    }
        // Vulkan says that window manager allows custom resolution
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        // Determine if vulkan's resolution or our custom resolution is the best fit
        VkExtent2D actualExtent =
                {
                    static_cast<type::uint32>(width),
                    static_cast<type::uint32>(height)
                };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

auto TriangleApp::createSwapChain() -> void
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // How many images should be in the swap chain
    // 1 more than minimum helps with mitigating wait times from driver
    // before another image is available to be rendered to
    type::uint32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

    // However, make sure that this isn't exceeding the max image count
    // 0 value for maxImageCount indicates no maximum
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    // Amount of layers in each image
    // This is always 1 unless doing stereoscopic 3D
    createInfo.imageArrayLayers = 1;
    // What kind of operations the image is being used for
    // In this case, it being directly rendered to, so its a
    // color attachment. VK_IMAGE_USAGE_TRANSFER_DST_BIT would be
    // another option for something like post-processing since
    // the swap chain would be used for transferring an already
    // rendered-to image
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Specify how to handle swap chain images across multiple queue families
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    type::uint32 queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    // Determine if there are multiple queue families
    if(indices.graphicsFamily != indices.presentFamily)
    {
        // Use concurrent mode if yes. Worse performance but no
        // ownership transfers
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        // Use exclusive mode if no. Best performance
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    // Can specify transformations to be applied to images in the swap chain
    // Such as rotations or flips. currentTransform indicates no transformation
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    // Composite alpha allows for blending with other windows in the window system
    // VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR ignores the alpha channels and
    // performs no blending
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // Clip any pixels that are obscured by something (such as another window)
    // This is normally desired, however you'd want to disable it if
    // you need to do something like continually reading pixel values
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    // This is used in the event of creating a new swap chain on the fly
    // For example, if the window is resized then a new swap chain need to
    // be entirely recreated and the handle to the old swap chain needs
    // to be stored here.
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Swap chain creation failed");
    }

    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

/** Image View Creation */

auto TriangleApp::createImageViews() -> void
{
    swapChainImageViews.resize(swapChainImages.size());

    for(type::size i = 0; i < swapChainImages.size(); ++i)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        // How image data should be interpreted
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        // Color channel swizzling. We'll leave as default
        // Can be used for stuff like mapping all color
        // channels to red for monochrome color
        createInfo.components.r =
        createInfo.components.g =
        createInfo.components.b =
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // Images purpose and which part of the image should be accessed
        // For example, with stereographic 3D the swap chain would
        // have multiple layers. Multiple image views for each image
        // could be created to represent left eyes, accessed by
        // different layers
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Image view creation failed");
        }
    }
}

/** Cleanup swapchain */
auto TriangleApp::cleanupSwapchain() -> void
{
    for(type::size i = 0; i < swapChainFramebuffers.size(); ++i)
    {
        vkDestroyFramebuffer(logicalDevice, swapChainFramebuffers[i], nullptr);
    }

    vkFreeCommandBuffers(logicalDevice, commandPool, static_cast<type::uint32>(commandBuffers.size()), commandBuffers.data());

    vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    vkDestroyRenderPass(logicalDevice, renderPass, nullptr);

    for(type::size i = 0; i < swapChainImageViews.size(); ++i)
    {
        vkDestroyImageView(logicalDevice, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);

    for(type::size i = 0; i < swapChainImages.size(); ++i)
    {
        vkDestroyBuffer(logicalDevice, uniformBuffers[i], nullptr);
        vkFreeMemory(logicalDevice, uniformBufferMemories[i], nullptr);
    }
    vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
}

/** Recreate swap chain */
auto TriangleApp::recreateSwapChain() -> void
{
    // Frame buffer size is 0 while window is minimizing,
        // so wait until its not
    int width, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while(width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    // Make sure no resources are in use
    vkDeviceWaitIdle(logicalDevice);

    cleanupSwapchain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
}

/**
 * Render Pass Setup
 */
auto TriangleApp::createRenderPass() -> void
{
    VkAttachmentDescription colorAttachment = {};
    // Format should match the swap chain format
    colorAttachment.format = swapChainImageFormat;
    // No multisampling yet, so 1 sample is fine
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // What to do with data before and after rendering
        // Applies to color and depth data
    // We want to present a rendered triangle, so we're
        // going to store
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //** Stencil config is set by stencilLoadOp and stencilStoreOp
        // but we're not doing anything with that for now so it can be
        // left out
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // Set pixel layout in memory for framebuffers and textures
        // initialLayout is the layout of an image before render pass
        // finalLayout is what to transition to after render pass
    // These settings mean that we don't care about the layout
        // the previous pixel layout and to just convert it.
        // This means that there is no guarantee for the
        // preservation of the original image, but we don't
        // care about that anyway since we're just rendering
        // then immediately discarding
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    /* Sub Passes */
    //** Subpasses are post-rendering operations depending on the contents
        // of the framebuffer contents. Used for post-processing. Grouping
        // all of these sub passes into a group bundles into into a single
        // render pass for better performance and memory efficiency

    // Subpass attachment reference
    VkAttachmentReference colorAttachmentRef = {};
    // Index of attachment to reference from attachments desc. array
    colorAttachmentRef.attachment = 0;
    // Layout we want that attachment to have during a subpass with
        // this reference.
        // This attachment is for use as a color buffer, so we'll use
        // the layout with best performance
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass description
    VkSubpassDescription subpass = {};
    // Possibility of vulkan supporting compute pipelines in future,
        // so we have to be explicit that this is for graphics
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // Attach color attachment
    subpass.colorAttachmentCount = 1;
    // Index of this attachment is directly referenced in
        // fragment shader by the layout(location = 0) out vec4 outColor;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Subpass dependencies
    VkSubpassDependency dependency = {};
    // Set dependency to implicit subpass before render
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    // Index of subpass that we depend on after render
    dependency.dstSubpass = 0;
    // Wait for swap chain to reading image before accessing by waiting for
        // for the color attachment output
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    // Prevent transition from happening until reading and writing of color attachment
        // is done
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if(vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Render Pass setup failed");
    }
}

/**
 * Graphics Pipeline Creation
 */
auto TriangleApp::readFile(const std::string& fileName, std::vector<char>& buffer) -> std::vector<char>
{
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    type::size fileSize = static_cast<type::size>(file.tellg());
    buffer.resize(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

auto TriangleApp::createShaderModule(const std::vector<char> &code) -> VkShaderModule
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    // Vector class already ensures data alignment requirements
    createInfo.pCode = reinterpret_cast<const type::uint32*>(code.data());

    VkShaderModule shaderModule;
    if(vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module");
    }
    return shaderModule;
}

/* Describe the layout for the uniform buffer */
auto TriangleApp::createDescriptorSetLayout() -> void
{
    // Describe the binding in the shader that we want to link to
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    // Which shader stage this UBO is for
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    // For if we're doing image sampling
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if(vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Descriptor layout set creation failed");
    }

}

auto TriangleApp::createGraphicsPipeline() -> void
{
    /* Load and create shaders */
    std::vector<char> vertShaderCode, fragShaderCode;
    readFile("shaders/triangle.vert.spv", vertShaderCode);
    readFile("shaders/triangle.frag.spv", fragShaderCode);

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    // Specify the shader entry point
        // It is possible to have multiple entry points and combined multiple
        // shaders into a single shader module
    vertShaderStageInfo.pName = "main";
    // This allows constants to be specified at compile time, meaning
    // that the shaders can contain configuration code to eliminate the need
    // for runtime if statements and such
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    /* Setup Pipeline Input */

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    auto bindingDesc = Vertex::getBindingDescription();
    auto attributeDescs = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<type::uint32>(attributeDescs.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();

    // Sets what kind of geometry is being drawn from vertices (triangle strips, point list, etc)
    // and if primitive restart should be enabled (which is for stuff like element buffers)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    /* Setup pipeline viewport */

    // Setup viewport
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    // Depth buffer range
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Specify scissor rectangle
        // Scissor rectangle basically sets an area where any pixels outside of the bounds
        // get filtered out and aren't rendered
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    // Combine viewport and scissor
        // Some graphics cards support multiple viewports and scissors, so that's
        // why the struct members reference and array
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    /* Setup Rasterizer */
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // If enabled, clamps fragments outside of near and far planes instead of clipping them
    rasterizer.depthClampEnable = VK_FALSE;
    // If enabled, geometry never goes through rasterizer
    // This effectively disables output to framebuffer
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    // How fragments are generated for geometry
        // VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    // Thickness of lines in fragments
        // Max line thickness depends on hardware and anything higher than 1.0f
        // required wideLines to be enabled on gpu
    rasterizer.lineWidth = 1.0f;
    // Face culling
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    // Vertex order
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    // Bias depth values based on constant or fragment's slope
        // Sometimes used for shadow mapping
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    /* Setup Multisampling */
    // Using multisampling requires enabling a GPU feature
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    // We're going to disable multisampling for now
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    /* Depth and Stencil Testing */
    /*
     * None for now
     */

    /* Color Blending */
    // ** Two types of color blending structs
        // VkPipelineColorBlendAttachmentState is per-framebuffer
        // VkPipelineColorBlendStateCreateInfo is global blending settings
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
    // Leave blending disabled for now
    colorBlendAttachment.blendEnable = VK_FALSE;
    //** There's a bunch of color blending options that are going to be
        // left out for now since we're not enabling it anyway

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // Enable or disable bitwise combination blending
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    // Which color channels in framebuffer will be affected
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    /* Dynamic States */
    /*
     * None for now, but this would allow for stuff like dynamic viewport state that
     * would be set at drawtime
     */

    /* Pipeline Layout */
    //** Pipeline layout is where uniform values in shaders are specified
        // so they can be used
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if(vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Pipeline Layout creation failed");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // nullptr for now since we aren't using it
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // nullptr for now since we aren't using it
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    // Subpass index
    pipelineInfo.subpass = 0;
    // These two settings for are if you're creating a new pipeline
        // based off an existing one. This allows for better efficiency
        // since you can create a new pipeline based off of an existing
        // one with similar features
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if(vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Graphics Pipeline creation failed");
    }

    // Cleanup shaders
    vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}

/**
 * Swap Chain Framebuffers Creation
 */
auto TriangleApp::createFramebuffers() -> void
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    // Create framebuffers for each image view
    for(type::size i = 0; i < swapChainImageViews.size(); ++i)
    {
        VkImageView attachments[] =
                {
                    swapChainImageViews[i]
                };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if(vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Framebuffer creation failed");
        }
    }
}

/**
 * Command Pool Creation
 */
auto TriangleApp::createCommandPool() -> void
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // We're recording commands for drawing, so we use graphics family
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    // Optional flags for efficiency with reuse, but we're only recording
        // the command buffers at the beginning of the program so we'll use none
    poolInfo.flags = 0;

    if(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Command Pool creation failed");
    }
}

/**
 * Vertex Buffer Creation
 */
auto TriangleApp::findMemoryType(type::uint32 typeFilter, VkMemoryPropertyFlags properties) -> type::uint32
{
    VkPhysicalDeviceMemoryProperties memProp;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProp);

    // Check for memory type that satifies all requirements and return the index
    for(type::uint32 i = 0; i < memProp.memoryTypeCount; ++i)
    {
        if((typeFilter & (1 << i)) && (memProp.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Suitable memory type unavailable");
}

auto TriangleApp::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) -> void
{
    // In the future, would be a good idea to create a command pool for short-term
        // buffers such as this
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = commandPool;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logicalDevice, &allocateInfo, &commandBuffer);

    // Record the command buffer
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // Tell the driver that this command buffer will only be used once
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    // Submit the command buffer for execution
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    // Using a fence here would allow for asynchronous transfers
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

auto TriangleApp::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props,
                               VkBuffer &buffer, VkDeviceMemory &bufferMemory) -> void
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    // Only one queue can own this buffer at a time
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer");
    }

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(logicalDevice, buffer, &memReq);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, props);

    // In real applications, vkAllocateMemory should not be called everytime a new buffer is created
        // as there is a maximum number of allocations allowed.
        // Allocating a large block and then using offsets for the buffers should be used instead
    if(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Buffer bufferMemory allocation failed");
    }

    // Last parameter is offset for this buffer in this bufferMemory section
    vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
}

auto TriangleApp::createVertexBuffer() -> void
{
    //! Why a staging buffer is created to transfer data into the vertex buffer
        // This allows the vertex buffer to only be accessible by the GPU as the
        // staging buffer is the one that requires the flags to be accessible
        // by the CPU. This allows the driver to make memory optimizations

    // Size of data to be contained in buffer
    VkDeviceSize bufferSize = sizeof(vertices[0])*vertices.size();

    /* Create Staging (Transfer) Buffer */
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    // Setup staging buffer as the source of copied data
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    /* Filling Staging Buffer */
    void* data;
    // Access region in memory defined by offset and size
    vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    // Copy the vertex data in the memory region
    memcpy(data, vertices.data(), static_cast<type::size>(bufferSize));
    vkUnmapMemory(logicalDevice, stagingBufferMemory);

    /* Create Vertex Buffer */
    // Setup vertex buffer as destinaton of copied data
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertexBuffer, vertexBufferMemory);

    // Copy the data from the staging buffer into the vertex buffer
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

auto TriangleApp::createIndexBuffer() -> void
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), static_cast<type::size>(bufferSize));
    vkUnmapMemory(logicalDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

auto TriangleApp::createUniformBuffers() -> void
{
    VkDeviceSize bufferSize = sizeof(UBO::MVP);

    uniformBuffers.resize(swapChainImages.size());
    uniformBufferMemories.resize(swapChainImages.size());

    for(type::size i = 0; i < swapChainImages.size(); ++i)
    {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                uniformBuffers[i], uniformBufferMemories[i]);
    }
}

auto TriangleApp::createDescriptorPool() -> void
{
    // Which descriptor types are being used and how many
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<type::uint32>(swapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    // Maximum amount of descriptor sets that can be allocated
    poolInfo.maxSets = static_cast<type::uint32>(swapChainImages.size());

    if(vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Descriptor pool creation failed");
    }
}

auto TriangleApp::createDescriptorSets() -> void
{
    std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    // Descriptor pool to allocate from
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<type::uint32>(swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    // Allocate a descriptor set for each swap chain image
    descriptorSets.resize(swapChainImages.size());
    if(vkAllocateDescriptorSets(logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Descriptor set allocation failed");
    }

    // Configure each descriptor
    for(type::size i = 0; i < swapChainImages.size(); ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        //bufferInfo.range = sizeof(UBO::MVP);
        // VK_WHOLE_SIZE can be used instead since we're overwriting
            // the whole buffer
        bufferInfo.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        // Binding index of the UBO in shader
        descriptorWrite.dstBinding = 0;
        // First index in array of descriptors being updated,
            // but we're only updating one so this is 0
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        // Array (or single pointer) of descriptorCount structs
            // that configure the descriptors
        descriptorWrite.pBufferInfo = &bufferInfo;
        // Same but for descriptors for image data
        descriptorWrite.pImageInfo = nullptr;
        // Same but for descriptors for buffer views
        descriptorWrite.pTexelBufferView = nullptr;

        // Apply updates
        vkUpdateDescriptorSets(logicalDevice, 1, &descriptorWrite, 0, nullptr);
    }
}

/**
 * Command Buffer Allocation
 */
auto TriangleApp::createCommandBuffers() -> void
{
    commandBuffers.resize(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = commandPool;
    // Specified if buffer is primary or second level
        // Primary means it can be submitted to queue for execution but can't
            // be called from other command buffers
        // Secondary means it can't be submitted directly but can be called
            // from other command buffers. This is so that common operations
            // can be reused across the primary command buffers
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = static_cast<type::uint32>(commandBuffers.size());

    if(vkAllocateCommandBuffers(logicalDevice, &allocateInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Command buffer allocation failed");
    }

    /* Start command buffer recording */
    for(type::size i = 0; i < commandBuffers.size(); ++i)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // The flags are for how the command buffer that will be used. None of the options
            // apply to what we're doing right now
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Command buffer recording failed to start");
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        // Use our framebuffer that we set up as a color attachment
        renderPassInfo.framebuffer = swapChainFramebuffers[i];
        // Set size of render area
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        // Define the clear values for the color attachment load op
            // (which as set to *_LOAD_OP_CLEAR
        static constexpr VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        // Last parameter is to set that it is for the primary command buffer.
            // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFER would be for commands
            // executed from secondary command buffers
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        /* Begin basic drawing */
        // Bind the pipeline that we want to use
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        // Bind the vertex and index buffer(s)
        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        // Bind the correct descriptor set for each swap chain image to the
            // descriptors in the shader
        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
        // Draw command
        vkCmdDrawIndexed(commandBuffers[i], static_cast<type::uint32>(indices.size()), 1, 0, 0, 0);
        vkCmdEndRenderPass(commandBuffers[i]);
        if(vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Command buffer recording failed");
        }
    }

}

/**
 * Semaphore Creation
 */
auto TriangleApp::createSyncObjects() -> void
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // Initialize fence to already signaled so it doesn't
        // hang on first frame
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(type::size i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if(vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
           vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
           vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS
        )
        {
            throw std::runtime_error("Semaphore creation failed");
        }
    }
}

/**
 *
 * Application Maintenance
 *
 */

auto TriangleApp::updateUniformBuffer(type::uint32 currImg) -> void
{
    // Timer for consistent geometry rotation
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currTime-startTime).count();

    UBO::MVP mvp = {};
    mvp.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    mvp.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    mvp.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / static_cast<float>(swapChainExtent.height), 0.1f, 10.0f);
    // GLM was designed in OpenGL in mind and OpenGL inverts the Y axis
        // Vulkan however does not, so undo the inversion
    mvp.proj[1][1] *= -1;

    // This is not the most efficient way to use a UBO
        // Look into passing a small buffer of push constants
    void* data;
    vkMapMemory(logicalDevice, uniformBufferMemories[currImg], 0, sizeof(mvp), 0, &data);
    memcpy(data, &mvp, sizeof(mvp));
    vkUnmapMemory(logicalDevice, uniformBufferMemories[currImg]);
}

auto TriangleApp::drawFrame() -> void
{
    // Sync queues before continuing
    vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, type::uint64_max);

    /* Submit image to queue */
    // Get image from swap chain
    type::uint32 imageIndex;
    // uint64 max value for timeout disables timeout. Fence is null since we're using semaphores, not fences
    VkResult result = vkAcquireNextImageKHR(logicalDevice, swapChain, type::uint64_max,
            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    // Create new swap chain if needed
    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return;
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image");
    }

    // Check if a previous frame is still using this image
    if(imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, type::uint64_max);
    }
    // Mark image as in use
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    updateUniformBuffer(imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Wait until image is available
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    // Wait at the color attachment stage
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    // Which semaphores to wait on and what stages in execution to wait at
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    // Which command buffers to submit for execution
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
    // Which semaphores to signal when the command buffers have finished
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);
    if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Command buffer submission failed");
    }

    /* Presentation */
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // Which semaphores to wait on before presentation
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    // Swap chains to present images to
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    // Index of image for each swapchain
    presentInfo.pImageIndices = &imageIndex;
    // Array of VkResult values to check
    // presentation success for each swap chain.
        // Unnecessary since we have only one swap chain
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

auto TriangleApp::mainLoop() -> void
{
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        drawFrame();
    }
    // Sync everything before exiting and cleaning up memory
    vkDeviceWaitIdle(logicalDevice);
}

auto TriangleApp::cleanup() -> void
{
    cleanupSwapchain();

    vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);

    vkDestroyBuffer(logicalDevice, indexBuffer, nullptr);
    vkFreeMemory(logicalDevice, indexBufferMemory, nullptr);

    vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr);
    vkFreeMemory(logicalDevice, vertexBufferMemory, nullptr);

    for(type::size i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

    vkDestroyDevice(logicalDevice, nullptr);

    if(enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}
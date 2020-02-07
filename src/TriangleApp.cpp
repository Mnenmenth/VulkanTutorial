/**
  * Created by Earl Kennedy
  * https://github.com/Mnenmenth
  */

#include <stdexcept>
#include <iostream>
#include <cstring>
#include <map>
#include "TriangleApp.h"

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

auto TriangleApp::initWindow() -> void
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Vulkan App", nullptr, nullptr);

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

auto TriangleApp::findQueueFamilies(VkPhysicalDevice device) -> TriangleApp::QueueFamilyIndices
{
    QueueFamilyIndices indices;

    type::uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for(const auto& queueFamily : queueFamilies)
    {
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        if(indices.isComplete())
        {
            break;
        }
        ++i;
    }

    return indices;
}

auto TriangleApp::ratePhysicalDevice(VkPhysicalDevice device) -> int
{
    if(!findQueueFamilies(device).isComplete())
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

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    /// This will be used later on
    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;

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

auto TriangleApp::initVulkan() -> void
{
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
}

auto TriangleApp::mainLoop() -> void
{
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

auto TriangleApp::cleanup() -> void
{
    vkDestroyDevice(logicalDevice, nullptr);

    if(enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}

auto TriangleApp::run() -> void
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}
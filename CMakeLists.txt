cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(VulkanTutorial)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SRC "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.h")


add_executable(VulkanTutorial ${SRC})
target_include_directories(VulkanTutorial PRIVATE Vulkan::Vulkan)
target_link_libraries(VulkanTutorial glfw3 Vulkan::Vulkan)
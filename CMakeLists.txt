cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(VulkanTutorial)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

## Compile Shaders
add_custom_target(SHADERS_SCRIPT
        COMMAND ${CMAKE_COMMAND}
        -DSHADER_DIR=${CMAKE_SOURCE_DIR}/shaders
        -DOUT_DIR=${CMAKE_BINARY_DIR}/shaders
        -P ${CMAKE_SOURCE_DIR}/shaders/CompileShaders.cmake
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

file(GLOB_RECURSE SRC "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.h")

add_executable(VulkanTutorial ${SRC})
target_include_directories(VulkanTutorial PRIVATE Vulkan::Vulkan glm)
target_link_libraries(VulkanTutorial glfw Vulkan::Vulkan)
## Compile Shaders
add_dependencies(VulkanTutorial SHADERS_SCRIPT)

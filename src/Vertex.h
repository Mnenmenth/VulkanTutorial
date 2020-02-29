/**
  * Created by Earl Kennedy
  * https://github.com/Mnenmenth
  */

#ifndef VULKANTUTORIAL_VERTEX_H
#define VULKANTUTORIAL_VERTEX_H

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    static auto getBindingDescription() -> VkVertexInputBindingDescription
    {
        VkVertexInputBindingDescription bindingDesc = {};
        // Index of binding in array of bindings
        bindingDesc.binding = 0;
        // Offset between entries
        bindingDesc.stride = sizeof(Vertex);
        // Move to next data entry after every vertex
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDesc;
    }

    static auto getAttributeDescriptions() -> std::array<VkVertexInputAttributeDescription, 2>
    {
        std::array<VkVertexInputAttributeDescription, 2> descs = {};
        // Which binding the per-vertex data comes from
        descs[0].binding = 0;
        // Refers to the 'location =' in vertex layout
        descs[0].location = 0;
        // Byte size of attribute data
            // VK_FORMAT_R32G32_SFLOAT = vec2
        descs[0].format = VK_FORMAT_R32G32_SFLOAT;
        // Offset of pos member in struct in bytes
        descs[0].offset = offsetof(Vertex, pos);

        descs[1].binding = 0;
        descs[1].location = 1;
        descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        descs[1].offset = offsetof(Vertex, color);
        return descs;
    }

};

#endif //VULKANTUTORIAL_VERTEX_H

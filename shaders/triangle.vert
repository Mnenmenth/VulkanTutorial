#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 VertPos;
layout(location = 1) in vec3 VertColor;

layout(location = 0) out vec3 FragColor;

layout(binding = 0) uniform MVP_UBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

void main()
{
    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(VertPos, 0.0, 1.0);
    FragColor = VertColor;
}
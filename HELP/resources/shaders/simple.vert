#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 faceColor;

layout(push_constant) uniform Push {
    mat4 transform;
} push;

void main() {
    gl_Position = push.transform * vec4(position, 1.0);
    faceColor = color;
}
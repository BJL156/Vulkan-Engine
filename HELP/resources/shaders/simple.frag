#version 450

layout(location = 0) in vec3 faceColor;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(faceColor, 1.0);
}
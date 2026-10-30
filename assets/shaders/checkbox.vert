#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec4 color;
uniform vec2 resolution;
out vec4 vertexColor;
void main() {
    vec2 clip = vec2(2.0 * position.x / resolution.x - 1.0,
                     1.0 - 2.0 * position.y / resolution.y);
    gl_Position = vec4(clip, 0.0, 1.0);
    vertexColor = color;
}

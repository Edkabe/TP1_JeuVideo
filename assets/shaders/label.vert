#version 330 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
uniform vec2 uViewport;
uniform vec2 uPosition;
uniform vec2 uSize;
out vec2 vTexCoord;
void main() {
    vec2 pixel = uPosition + aPosition * uSize;
    gl_Position = vec4(pixel.x * 2.0 / uViewport.x - 1.0,
                       1.0 - pixel.y * 2.0 / uViewport.y, 0.0, 1.0);
    vTexCoord = aTexCoord;
}

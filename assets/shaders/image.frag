#version 330 core
in vec2 vTexCoord;
uniform sampler2D uTexture;
out vec4 pixel;
void main() { pixel = texture(uTexture, vTexCoord); }

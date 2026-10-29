#version 330 core
in vec2 vTexCoord;
uniform sampler2D uText;
uniform vec4 uColor;
out vec4 outColor;
void main() {
    float coverage = texture(uText, vTexCoord).a;
    outColor = vec4(uColor.rgb, uColor.a * coverage);
}

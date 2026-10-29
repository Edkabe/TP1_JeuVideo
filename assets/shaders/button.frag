#version 330 core
in vec2 vLocal;
uniform vec2 uSize;
uniform float uBorderWidth;
uniform vec4 uBorder;
uniform vec4 uBackground;
out vec4 pixel;
void main() {
    bool edge = vLocal.x < uBorderWidth || vLocal.y < uBorderWidth ||
                vLocal.x >= uSize.x - uBorderWidth || vLocal.y >= uSize.y - uBorderWidth;
    pixel = edge ? uBorder : uBackground;
}

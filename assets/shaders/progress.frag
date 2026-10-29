#version 330 core
in vec2 vLocal;
uniform vec2 uSize;
uniform float uBorderWidth;
uniform float uProgress;
uniform vec4 uBorder;
uniform vec4 uBackground;
uniform vec4 uFill;
out vec4 pixel;
void main() {
    float border = min(uBorderWidth, min(uSize.x, uSize.y) * 0.5);
    bool edge = vLocal.x < border || vLocal.y < border ||
                vLocal.x >= uSize.x - border || vLocal.y >= uSize.y - border;
    float end = border + (uSize.x - 2.0 * border) * uProgress;
    pixel = edge ? uBorder : (vLocal.x < end ? uFill : uBackground);
}

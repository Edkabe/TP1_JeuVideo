#version 330 core
uniform vec2 uViewport;
uniform vec2 uPosition;
uniform vec2 uSize;
out vec2 vLocal;
const vec2 vertices[6] = vec2[6](vec2(0,0),vec2(0,1),vec2(1,1),vec2(0,0),vec2(1,1),vec2(1,0));
void main() {
    vLocal = vertices[gl_VertexID] * uSize;
    vec2 pixel = uPosition + vLocal;
    gl_Position = vec4(pixel.x * 2.0 / uViewport.x - 1.0, 1.0 - pixel.y * 2.0 / uViewport.y, 0.0, 1.0);
}

#include "moteur/CheckBox.h"
#include <cmath>

namespace moteur {
namespace {
constexpr const char* vertexSource = R"glsl(
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
)glsl";

constexpr const char* fragmentSource = R"glsl(
#version 330 core
in vec4 vertexColor;
out vec4 outputColor;
void main() { outputColor = vertexColor; }
)glsl";
}

bool CheckBox::initialize() {
    error.clear();
    if (initialized) return true;
    if (!vertexShader.compile(vertexSource) || !fragmentShader.compile(fragmentSource)) {
        error = vertexShader.getError() + fragmentShader.getError();
        return false;
    }
    if (!program.link({&vertexShader, &fragmentShader})) {
        error = program.getError();
        return false;
    }
    resolutionLocation = glGetUniformLocation(program.getId(), "resolution");
    if (resolutionLocation < 0 || !vao.create() || !vbo.create()) {
        error = "Creation des ressources OpenGL de la case impossible.";
        return false;
    }

    vao.bind();
    vbo.bind();
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * 42), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    VAO::unbind();
    VBO::unbind();
    initialized = true;
    return true;
}

void CheckBox::addTriangle(float ax, float ay, float bx, float by, float cx, float cy,
                           Color color, std::array<Vertex, 42>& vertices, std::size_t& count) const {
    vertices[count++] = {ax, ay, color.red, color.green, color.blue, color.alpha};
    vertices[count++] = {bx, by, color.red, color.green, color.blue, color.alpha};
    vertices[count++] = {cx, cy, color.red, color.green, color.blue, color.alpha};
}

void CheckBox::addRectangle(float x, float y, float width, float height, Color color,
                            std::array<Vertex, 42>& vertices, std::size_t& count) const {
    addTriangle(x, y, x + width, y, x + width, y + height, color, vertices, count);
    addTriangle(x, y, x + width, y + height, x, y + height, color, vertices, count);
}

void CheckBox::addLine(float x1, float y1, float x2, float y2, float thickness, Color color,
                       std::array<Vertex, 42>& vertices, std::size_t& count) const {
    const float dx = x2 - x1;
    const float dy = y2 - y1;
    const float length = std::sqrt(dx * dx + dy * dy);
    if (length <= 0.0f) return;
    const float halfWidth = thickness * 0.5f;
    const float ox = -dy / length * halfWidth;
    const float oy = dx / length * halfWidth;
    addTriangle(x1 + ox, y1 + oy, x2 + ox, y2 + oy, x2 - ox, y2 - oy, color, vertices, count);
    addTriangle(x1 + ox, y1 + oy, x2 - ox, y2 - oy, x1 - ox, y1 - oy, color, vertices, count);
}

void CheckBox::render(float viewportWidth, float viewportHeight) {
    if (!initialized || size <= 0.0f || viewportWidth <= 0.0f || viewportHeight <= 0.0f) return;

    std::array<Vertex, 42> vertices{};
    std::size_t count = 0;
    const float x = position[0];
    const float y = position[1];
    const float border = size * 0.09f;

    // Le fond est rentré sous la bordure pour former un cadre net.
    addRectangle(x + border, y + border, size - 2.0f * border, size - 2.0f * border,
                 backgroundColor, vertices, count);
    addRectangle(x, y, size, border, borderColor, vertices, count);
    addRectangle(x, y + size - border, size, border, borderColor, vertices, count);
    addRectangle(x, y + border, border, size - 2.0f * border, borderColor, vertices, count);
    addRectangle(x + size - border, y + border, border, size - 2.0f * border, borderColor, vertices, count);
    if (checked) {
        // Deux segments épais composent une coche simple, sans image externe.
        addLine(x + size * 0.22f, y + size * 0.51f, x + size * 0.42f, y + size * 0.70f,
                size * 0.11f, checkColor, vertices, count);
        addLine(x + size * 0.39f, y + size * 0.66f, x + size * 0.79f, y + size * 0.27f,
                size * 0.11f, checkColor, vertices, count);
    }

    program.use();
    glUniform2f(resolutionLocation, viewportWidth, viewportHeight);
    vao.bind();
    vbo.setData(vertices.data(), static_cast<GLsizeiptr>(sizeof(Vertex) * count));
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(count));
    VAO::unbind();
    Program::unuse();
}

bool CheckBox::handleEvent(const SDL_Event& event) {
    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN || event.button.button != SDL_BUTTON_LEFT) return false;
    const float mouseX = event.button.x;
    const float mouseY = event.button.y;
    if (mouseX < position[0] || mouseX > position[0] + size ||
        mouseY < position[1] || mouseY > position[1] + size) return false;
    checked = !checked;
    return true;
}

}

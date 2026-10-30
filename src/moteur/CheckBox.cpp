#include "moteur/CheckBox.h"
#include "moteur/ProgramCache.h"
#include "moteur/Engine.h"
#include <cmath>

namespace moteur {
bool CheckBox::initialize() {
    error.clear();
    if (initialized) return true;
    const char* base = SDL_GetBasePath();
    if (!base) { error = SDL_GetError(); return false; }
    const auto directory = std::filesystem::path(base) / "assets/shaders";
    ProgramCache cache;
    if (!cache.load(program, {{GL_VERTEX_SHADER, directory / "checkbox.vert"},
                              {GL_FRAGMENT_SHADER, directory / "checkbox.frag"}},
                    std::filesystem::path(base) / "cache/programs/checkbox.bin",
                    Engine::getInstance().getSettings())) {
        error = cache.getError(); return false;
    }
    resolutionLocation = program.getUniformLocation("resolution");
    if (resolutionLocation < 0 || !vao.create() || !vbo.create()) {
        error = "Creation des ressources OpenGL de la case impossible.";
        return false;
    }

    vao.bind();
    vbo.bind();
    vbo.setData(nullptr, static_cast<GLsizeiptr>(sizeof(Vertex) * 42), GL_DYNAMIC_DRAW);
    vao.setAttribute(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    vao.setAttribute(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 2 * sizeof(float));
    VAO::unbind();
    VBO::unbind();
    initialized = true;
    return true;
}

void CheckBox::addTriangle(float ax, float ay, float bx, float by, float cx, float cy,
                           Color color, std::array<Vertex, 42>& vertices, std::size_t& count) const {
    vertices[count++] = {ax, ay, color.r, color.g, color.b, color.a};
    vertices[count++] = {bx, by, color.r, color.g, color.b, color.a};
    vertices[count++] = {cx, cy, color.r, color.g, color.b, color.a};
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

    // Le fond est rentrÃ© sous la bordure pour former un cadre net.
    addRectangle(x + border, y + border, size - 2.0f * border, size - 2.0f * border,
                 backgroundColor, vertices, count);
    addRectangle(x, y, size, border, borderColor, vertices, count);
    addRectangle(x, y + size - border, size, border, borderColor, vertices, count);
    addRectangle(x, y + border, border, size - 2.0f * border, borderColor, vertices, count);
    addRectangle(x + size - border, y + border, border, size - 2.0f * border, borderColor, vertices, count);
    if (checked) {
        // Deux segments Ã©pais composent une coche simple, sans image externe.
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
    float mouseX = event.button.x;
    float mouseY = event.button.y;
    // SDL fournit des coordonnÃ©es logiques; la case est dessinÃ©e en pixels
    // du viewport, comme les autres contrÃ´les interactifs.
    SDL_Window* window = SDL_GetWindowFromID(event.button.windowID);
    if (window && SDL_GL_GetCurrentContext()) {
        int logicalWidth = 0, logicalHeight = 0, pixelWidth = 0, pixelHeight = 0;
        if (SDL_GetWindowSize(window, &logicalWidth, &logicalHeight) &&
            logicalWidth > 0 && logicalHeight > 0 &&
            SDL_GetWindowSizeInPixels(window, &pixelWidth, &pixelHeight)) {
            GLint viewport[4]{};
            glGetIntegerv(GL_VIEWPORT, viewport);
            mouseX = mouseX * pixelWidth / logicalWidth - viewport[0];
            mouseY = mouseY * pixelHeight / logicalHeight -
                (pixelHeight - viewport[1] - viewport[3]);
        }
    }
    if (mouseX < position[0] || mouseX > position[0] + size ||
        mouseY < position[1] || mouseY > position[1] + size) return false;
    checked = !checked;
    return true;
}

}

namespace moteur {
bool CheckBox::render() {
    if (!SDL_GL_GetCurrentContext()) { error = "Contexte OpenGL actif necessaire."; return false; }
    if (!initialize()) return false;
    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] > 0 && viewport[3] > 0)
        render(static_cast<float>(viewport[2]), static_cast<float>(viewport[3]));
    return true;
}
void CheckBox::release() {
    vbo.release(); vao.release(); program.release();
    initialized = false;
    resolutionLocation = -1;
}
}

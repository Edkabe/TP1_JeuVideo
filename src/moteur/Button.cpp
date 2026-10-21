#include "moteur/Button.h"
#include "moteur/Engine.h"
#include "moteur/ProgramCache.h"
#include <SDL3/SDL.h>
#include <filesystem>
#include <cmath>

namespace moteur {
namespace {
bool finite(Vector2 value) { return std::isfinite(value.x) && std::isfinite(value.y); }
Vector2 mousePosition(float x, float y, SDL_WindowID id) {
    // Les evenements SDL sont en coordonnees logiques, le rendu en pixels.
    SDL_Window* window = SDL_GetWindowFromID(id);
    if (!window || !SDL_GL_GetCurrentContext()) return {x,y};
    int width = 0, height = 0, pixelsX = 0, pixelsY = 0;
    if (!SDL_GetWindowSize(window, &width, &height) || width <= 0 || height <= 0 ||
        !SDL_GetWindowSizeInPixels(window, &pixelsX, &pixelsY)) return {x,y};
    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    return {x * pixelsX / width - viewport[0], y * pixelsY / height - (pixelsY - viewport[1] - viewport[3])};
}
// Restauration egalement si le rendu du titre echoue.
struct RenderState {
    GLint program = 0, vao = 0, srcRgb = 0, dstRgb = 0, srcAlpha = 0, dstAlpha = 0, eqRgb = 0, eqAlpha = 0;
    GLboolean blend, depth, cull;
    RenderState() : blend(glIsEnabled(GL_BLEND)), depth(glIsEnabled(GL_DEPTH_TEST)), cull(glIsEnabled(GL_CULL_FACE)) {
        glGetIntegerv(GL_CURRENT_PROGRAM, &program); glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
        glGetIntegerv(GL_BLEND_SRC_RGB, &srcRgb); glGetIntegerv(GL_BLEND_DST_RGB, &dstRgb);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha); glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
        glGetIntegerv(GL_BLEND_EQUATION_RGB, &eqRgb); glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &eqAlpha);
    }
    ~RenderState() {
        glUseProgram(program); glBindVertexArray(vao);
        glBlendFuncSeparate(srcRgb, dstRgb, srcAlpha, dstAlpha); glBlendEquationSeparate(eqRgb, eqAlpha);
        if (blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (depth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (cull) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    }
};
}
Button::Button(std::shared_ptr<Font> font, std::string title) : label(std::move(font), std::move(title)) { centerTitle(); }
void Button::centerTitle() {
    const auto size = label.getDimensions();
    label.setPosition(position + (dimensions - size) * 0.5f);
}
bool Button::setPosition(Vector2 value) {
    error.clear();
    if (!finite(value)) { error = "Position invalide."; return false; }
    position = value; hovered = pressed = false; centerTitle(); return true;
}
bool Button::setDimensions(Vector2 value) {
    error.clear();
    if (!finite(value) || value.x < 0 || value.y < 0) { error = "Dimensions invalides."; return false; }
    dimensions = value; hovered = pressed = false; centerTitle(); return true;
}
bool Button::setTitle(std::string value) {
    error.clear();
    if (!label.setText(std::move(value))) { error = label.getError(); return false; }
    centerTitle(); return true;
}
bool Button::setFont(std::shared_ptr<Font> value) {
    error.clear();
    if (!label.setFont(std::move(value))) { error = label.getError(); return false; }
    centerTitle(); return true;
}
bool Button::setBorderWidth(float value) {
    error.clear();
    if (!std::isfinite(value) || value < 0) { error = "Largeur de bordure invalide."; return false; }
    borderWidth = value; return true;
}
bool Button::contains(Vector2 point) const {
    return point.x >= position.x && point.y >= position.y &&
        point.x < position.x + dimensions.x && point.y < position.y + dimensions.y;
}
void Button::onEvent(const Event& event) {
    const auto& sdl = event.getSDLEvent();
    if (sdl.type == SDL_EVENT_WINDOW_FOCUS_LOST || sdl.type == SDL_EVENT_WINDOW_MOUSE_LEAVE) {
        hovered = pressed = false; return;
    }
    if (sdl.type == SDL_EVENT_MOUSE_MOTION) {
        hovered = contains(mousePosition(sdl.motion.x, sdl.motion.y, sdl.motion.windowID)); return;
    }
    if ((sdl.type != SDL_EVENT_MOUSE_BUTTON_DOWN && sdl.type != SDL_EVENT_MOUSE_BUTTON_UP) ||
        sdl.button.button != SDL_BUTTON_LEFT) return;
    hovered = contains(mousePosition(sdl.button.x, sdl.button.y, sdl.button.windowID));
    if (sdl.type == SDL_EVENT_MOUSE_BUTTON_DOWN) { pressed = hovered; return; }
    const bool clicked = pressed && hovered;
    pressed = false;
    // Copier le callback avant appel : il peut retirer le controle de la scene.
    const auto handler = onClick;
    if (clicked && handler) handler();
}
bool Button::createGraphics() {
    if (program.getId() && vao.getId()) return true;
    const char* base = SDL_GetBasePath();
    if (!base) { error = SDL_GetError(); return false; }
    const auto directory = std::filesystem::path(base) / "assets/shaders";
    ProgramCache cache;
    if (!cache.load(program, {{GL_VERTEX_SHADER, directory / "button.vert"},
                              {GL_FRAGMENT_SHADER, directory / "button.frag"}},
                    std::filesystem::path(base) / "cache/programs/button.bin", Engine::getInstance().getSettings())) {
        error = cache.getError(); return false;
    }
    if (!vao.create()) { error = "Creation du VAO du bouton impossible."; return false; }
    return true;
}
bool Button::render() {
    error.clear();
    if (!SDL_GL_GetCurrentContext()) { error = "Contexte OpenGL actif necessaire."; return false; }
    if (dimensions.x == 0 || dimensions.y == 0) return true;
    GLint viewport[4]{}; glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] <= 0 || viewport[3] <= 0) return true;
    if (!createGraphics()) return false;
    RenderState state;
    glEnable(GL_BLEND); glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE);
    glBlendEquation(GL_FUNC_ADD); glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    program.use(); vao.bind();
    glUniform2f(glGetUniformLocation(program.getId(), "uViewport"), static_cast<float>(viewport[2]), static_cast<float>(viewport[3]));
    glUniform2f(glGetUniformLocation(program.getId(), "uPosition"), position.x, position.y);
    glUniform2f(glGetUniformLocation(program.getId(), "uSize"), dimensions.x, dimensions.y);
    glUniform1f(glGetUniformLocation(program.getId(), "uBorderWidth"), borderWidth);
    const auto color = hovered ? hover : background;
    glUniform4f(glGetUniformLocation(program.getId(), "uBackground"), color.r, color.g, color.b, color.a);
    glUniform4f(glGetUniformLocation(program.getId(), "uBorder"), border.r, border.g, border.b, border.a);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    centerTitle();
    if (!label.render()) { error = label.getError(); return false; }
    return true;
}
void Button::release() { label.release(); vao.release(); program.release(); }
}

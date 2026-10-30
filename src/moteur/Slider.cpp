#include "moteur/Slider.h"
#include "moteur/ProgramCache.h"
#include "moteur/Engine.h"
#include "moteur/Vector2.h"
#include "moteur/VertexShader.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <filesystem>

namespace moteur {
namespace {
bool finite(Vector2 point) {
    return std::isfinite(point.x) && std::isfinite(point.y);
}

// Les Ã©vÃ©nements SDL utilisent les pixels logiques de la souris; le slider
// est placÃ© dans le viewport OpenGL, qui peut Ãªtre en pixels haute rÃ©solution.
Vector2 mousePosition(float x, float y, SDL_WindowID windowId) {
    SDL_Window* window = SDL_GetWindowFromID(windowId);
    if (!window || !SDL_GL_GetCurrentContext()) return {x, y};

    int logicalWidth = 0, logicalHeight = 0, pixelWidth = 0, pixelHeight = 0;
    if (!SDL_GetWindowSize(window, &logicalWidth, &logicalHeight) ||
        logicalWidth <= 0 || logicalHeight <= 0 ||
        !SDL_GetWindowSizeInPixels(window, &pixelWidth, &pixelHeight)) return {x, y};

    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    return {x * pixelWidth / logicalWidth - viewport[0],
            y * pixelHeight / logicalHeight - (pixelHeight - viewport[1] - viewport[3])};
}

// Le contrÃ´le restaure l'Ã©tat OpenGL pour ne pas modifier le rendu suivant.
struct RenderState {
    GLint program = 0, vao = 0;
    GLint sourceRgb = 0, destinationRgb = 0, sourceAlpha = 0, destinationAlpha = 0;
    GLint equationRgb = 0, equationAlpha = 0;
    GLboolean blend, depth, cull;

    RenderState()
        : blend(glIsEnabled(GL_BLEND)), depth(glIsEnabled(GL_DEPTH_TEST)),
          cull(glIsEnabled(GL_CULL_FACE)) {
        glGetIntegerv(GL_CURRENT_PROGRAM, &program);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
        glGetIntegerv(GL_BLEND_SRC_RGB, &sourceRgb);
        glGetIntegerv(GL_BLEND_DST_RGB, &destinationRgb);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &sourceAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &destinationAlpha);
        glGetIntegerv(GL_BLEND_EQUATION_RGB, &equationRgb);
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &equationAlpha);
    }

    ~RenderState() {
        glUseProgram(static_cast<GLuint>(program));
        glBindVertexArray(static_cast<GLuint>(vao));
        glBlendFuncSeparate(sourceRgb, destinationRgb, sourceAlpha, destinationAlpha);
        glBlendEquationSeparate(equationRgb, equationAlpha);
        if (blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (depth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (cull) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    }
};
}

bool Slider::setPosition(Vector2 next) {
    error.clear();
    if (!finite(next)) { error = "Position du glisseur invalide."; return false; }
    position = next;
    return true;
}

bool Slider::setDimensions(Vector2 next) {
    error.clear();
    if (!finite(next) || next.x < 0.0f || next.y < 0.0f) {
        error = "Dimensions du glisseur invalides.";
        return false;
    }
    dimensions = next;
    return true;
}

bool Slider::setRange(float nextMinimum, float nextMaximum, float nextIncrement) {
    error.clear();
    if (!std::isfinite(nextMinimum) || !std::isfinite(nextMaximum) ||
        !std::isfinite(nextIncrement) || nextMinimum >= nextMaximum || nextIncrement <= 0.0f) {
        error = "Plage ou increment du glisseur invalide.";
        return false;
    }
    minimum = nextMinimum;
    maximum = nextMaximum;
    increment = nextIncrement;
    return setValue(value);
}

bool Slider::setMinimum(float nextMinimum) {
    return setRange(nextMinimum, maximum, increment);
}

bool Slider::setMaximum(float nextMaximum) {
    return setRange(minimum, nextMaximum, increment);
}

bool Slider::setIncrement(float nextIncrement) {
    return setRange(minimum, maximum, nextIncrement);
}

bool Slider::setValue(float nextValue) {
    error.clear();
    if (!std::isfinite(nextValue)) { error = "Valeur du glisseur invalide."; return false; }

    // Arrondir au pas le plus proche, puis borner pour conserver une valeur valide.
    const float bounded = (std::clamp)(nextValue, minimum, maximum);
    // Le maximum reste une valeur valide mÃªme si l'intervalle n'est pas un multiple du pas.
    if (bounded == maximum) { value = maximum; return true; }
    const float steps = std::round((bounded - minimum) / increment);
    value = (std::clamp)(minimum + steps * increment, minimum, maximum);
    return true;
}

float Slider::thumbWidth() const {
    return (std::min)(18.0f, dimensions.x);
}

float Slider::valueRatio() const {
    return maximum > minimum ? (value - minimum) / (maximum - minimum) : 0.0f;
}

float Slider::thumbLeft() const {
    return position.x + valueRatio() * (dimensions.x - thumbWidth());
}

bool Slider::containsThumb(Vector2 point) const {
    const float left = thumbLeft();
    return point.x >= left && point.x <= left + thumbWidth() &&
        point.y >= position.y && point.y <= position.y + dimensions.y;
}

void Slider::updateFromMouse(float x) {
    const float travel = dimensions.x - thumbWidth();
    if (travel <= 0.0f) return;
    // Conserver le point exact oÃ¹ la poignÃ©e a Ã©tÃ© saisie pendant le glissement.
    const float ratio = (std::clamp)((x - dragOffset - position.x) / travel, 0.0f, 1.0f);
    setValue(minimum + ratio * (maximum - minimum));
}

void Slider::onEvent(const Event& event) {
    const auto& sdl = event.getSDLEvent();
    if (sdl.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
        dragging = false;
        hovered = false;
        return;
    }
    if (sdl.type == SDL_EVENT_WINDOW_MOUSE_LEAVE) {
        // Garder le glissement actif jusqu'au relÃ¢chement du bouton gauche.
        hovered = false;
        return;
    }
    if (sdl.type == SDL_EVENT_MOUSE_MOTION) {
        const Vector2 point = mousePosition(sdl.motion.x, sdl.motion.y, sdl.motion.windowID);
        if (dragging) updateFromMouse(point.x);
        hovered = containsThumb(point);
        return;
    }
    if (sdl.type == SDL_EVENT_MOUSE_BUTTON_DOWN && sdl.button.button == SDL_BUTTON_LEFT) {
        const Vector2 point = mousePosition(sdl.button.x, sdl.button.y, sdl.button.windowID);
        hovered = containsThumb(point);
        dragging = hovered;
        if (dragging) dragOffset = point.x - thumbLeft();
        return;
    }
    if (sdl.type == SDL_EVENT_MOUSE_BUTTON_UP && sdl.button.button == SDL_BUTTON_LEFT) {
        // Le relÃ¢chement termine le glissement mÃªme si la souris est hors du contrÃ´le.
        const Vector2 point = mousePosition(sdl.button.x, sdl.button.y, sdl.button.windowID);
        dragging = false;
        hovered = containsThumb(point);
    }
}

bool Slider::createGraphics() {
    if (program.getId() != 0 && vao.getId() != 0) return true;
    const char* basePath = SDL_GetBasePath();
    if (!basePath) { error = SDL_GetError(); return false; }

    // Le shader rectangulaire du bouton sert aussi aux morceaux du glisseur.
    const auto shaderDirectory = std::filesystem::path(basePath) / "assets" / "shaders";
    ProgramCache cache;
    if (!cache.load(program, {{GL_VERTEX_SHADER, shaderDirectory / "button.vert"},
                              {GL_FRAGMENT_SHADER, shaderDirectory / "button.frag"}},
                    std::filesystem::path(basePath) / "cache/programs/button.bin",
                    Engine::getInstance().getSettings())) {
        error = cache.getError(); return false;
    }
    if (!vao.create()) { error = "Creation du VAO du glisseur impossible."; return false; }
    return true;
}

bool Slider::render() {
    error.clear();
    if (!SDL_GL_GetCurrentContext()) { error = "Contexte OpenGL actif necessaire."; return false; }
    if (dimensions.x <= 0.0f || dimensions.y <= 0.0f) return true;

    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] <= 0 || viewport[3] <= 0) return true;
    if (!createGraphics()) return false;

    RenderState state;
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    program.use();
    vao.bind();

    glUniform2f(program.getUniformLocation("uViewport"),
                static_cast<float>(viewport[2]), static_cast<float>(viewport[3]));
    const GLint positionLocation = program.getUniformLocation("uPosition");
    const GLint sizeLocation = program.getUniformLocation("uSize");
    const GLint borderWidthLocation = program.getUniformLocation("uBorderWidth");
    const GLint borderLocation = program.getUniformLocation("uBorder");
    const GLint backgroundLocation = program.getUniformLocation("uBackground");

    auto drawRectangle = [&](Vector2 topLeft, Vector2 size, Color border, Color fill, float borderWidth) {
        glUniform2f(positionLocation, topLeft.x, topLeft.y);
        glUniform2f(sizeLocation, size.x, size.y);
        glUniform1f(borderWidthLocation, borderWidth);
        glUniform4f(borderLocation, border.r, border.g, border.b, border.a);
        glUniform4f(backgroundLocation, fill.r, fill.g, fill.b, fill.a);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    };

    const float knobWidth = thumbWidth();
    const float trackWidth = dimensions.x - knobWidth;
    const float trackHeight = (std::min)(8.0f, dimensions.y);
    const float trackY = position.y + (dimensions.y - trackHeight) * 0.5f;
    const float trackX = position.x + knobWidth * 0.5f;
    drawRectangle({trackX, trackY}, {trackWidth, trackHeight}, borderColor, backgroundColor, 1.0f);

    const float innerTrackHeight = (std::max)(0.0f, trackHeight - 2.0f);
    const float progressWidth = trackWidth * valueRatio();
    if (progressWidth > 0.0f && innerTrackHeight > 0.0f) {
        drawRectangle({trackX + 1.0f, trackY + 1.0f}, {progressWidth, innerTrackHeight},
                      progressColor, progressColor, 0.0f);
    }

    const Color knobFill = hovered || dragging ? hoverColor : backgroundColor;
    drawRectangle({thumbLeft(), position.y}, {knobWidth, dimensions.y},
                  borderColor, knobFill, 1.0f);
    return true;
}

void Slider::release() {
    vao.release();
    program.release();
}

} // namespace moteur

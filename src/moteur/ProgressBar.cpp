#include "moteur/ProgressBar.h"
#include "moteur/Engine.h"
#include "moteur/ProgramCache.h"
#include <SDL3/SDL.h>
#include <filesystem>
#include <algorithm>
#include <cmath>

namespace moteur {
bool ProgressBar::setPosition(Vector2 candidate) {
    error.clear();
    if (!std::isfinite(candidate.x) || !std::isfinite(candidate.y)) { error = "Position invalide."; return false; }
    position = candidate; return true;
}
bool ProgressBar::setDimensions(Vector2 candidate) {
    error.clear();
    if (!std::isfinite(candidate.x) || !std::isfinite(candidate.y) || candidate.x < 0 || candidate.y < 0) {
        error = "Dimensions invalides."; return false;
    }
    dimensions = candidate; return true;
}
bool ProgressBar::setRange(float lower, float upper) {
    error.clear();
    if (!std::isfinite(lower) || !std::isfinite(upper) || lower >= upper) {
        error = "Bornes invalides : minimum doit etre inferieur a maximum."; return false;
    }
    minimum = lower; maximum = upper;
    value = std::clamp(value, minimum, maximum);
    return true;
}
bool ProgressBar::setValue(float candidate) {
    error.clear();
    if (!std::isfinite(candidate)) { error = "Valeur non finie refusee."; return false; }
    value = std::clamp(candidate, minimum, maximum); return true;
}
bool ProgressBar::advance(float amount) {
    error.clear();
    if (!std::isfinite(amount)) { error = "Progression non finie refusee."; return false; }
    // Le calcul en double evite le debordement meme aux limites de float.
    value = static_cast<float>(std::clamp(static_cast<double>(value) + amount,
        static_cast<double>(minimum), static_cast<double>(maximum)));
    return true;
}
float ProgressBar::getProgress() const {
    return static_cast<float>((static_cast<double>(value) - minimum) / (static_cast<double>(maximum) - minimum));
}
bool ProgressBar::setBorderWidth(float width) {
    error.clear();
    if (!std::isfinite(width) || width < 0) { error = "Largeur de bordure invalide."; return false; }
    borderWidth = width; return true;
}
bool ProgressBar::createGraphics() {
    if (program.getId() && vao.getId()) return true;
    const char* base = SDL_GetBasePath();
    if (!base) { error = SDL_GetError(); return false; }
    const auto directory = std::filesystem::path(base) / "assets/shaders";
    ProgramCache cache;
    if (!cache.load(program, {{GL_VERTEX_SHADER, directory / "progress.vert"},
                              {GL_FRAGMENT_SHADER, directory / "progress.frag"}},
                    std::filesystem::path(base) / "cache/programs/progress.bin", Engine::getInstance().getSettings())) {
        error = cache.getError(); return false;
    }
    if (!vao.create()) { error = "Creation du VAO de progression impossible."; return false; }
    return true;
}
bool ProgressBar::render() {
    error.clear();
    if (!SDL_GL_GetCurrentContext()) { error = "Contexte OpenGL actif necessaire."; return false; }
    if (dimensions.x == 0 || dimensions.y == 0) return true;
    GLint viewport[4]{}; glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] <= 0 || viewport[3] <= 0) return true;
    if (!createGraphics()) return false;
    GLint oldProgram = 0, oldVao = 0, srcRgb = 0, dstRgb = 0, srcAlpha = 0, dstAlpha = 0, eqRgb = 0, eqAlpha = 0;
    const auto blend = glIsEnabled(GL_BLEND), depth = glIsEnabled(GL_DEPTH_TEST), cull = glIsEnabled(GL_CULL_FACE);
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram); glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
    glGetIntegerv(GL_BLEND_SRC_RGB, &srcRgb); glGetIntegerv(GL_BLEND_DST_RGB, &dstRgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha); glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &eqRgb); glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &eqAlpha);
    glEnable(GL_BLEND); glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE);
    glBlendEquation(GL_FUNC_ADD); glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    program.use(); vao.bind();
    glUniform2f(glGetUniformLocation(program.getId(), "uViewport"), static_cast<float>(viewport[2]), static_cast<float>(viewport[3]));
    glUniform2f(glGetUniformLocation(program.getId(), "uPosition"), position.x, position.y);
    glUniform2f(glGetUniformLocation(program.getId(), "uSize"), dimensions.x, dimensions.y);
    glUniform1f(glGetUniformLocation(program.getId(), "uBorderWidth"), borderWidth);
    glUniform1f(glGetUniformLocation(program.getId(), "uProgress"), getProgress());
    glUniform4f(glGetUniformLocation(program.getId(), "uBackground"), background.r, background.g, background.b, background.a);
    glUniform4f(glGetUniformLocation(program.getId(), "uBorder"), border.r, border.g, border.b, border.a);
    glUniform4f(glGetUniformLocation(program.getId(), "uFill"), progress.r, progress.g, progress.b, progress.a);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(oldProgram); glBindVertexArray(oldVao);
    glBlendFuncSeparate(srcRgb, dstRgb, srcAlpha, dstAlpha); glBlendEquationSeparate(eqRgb, eqAlpha);
    if (!blend) glDisable(GL_BLEND);
    if (depth) glEnable(GL_DEPTH_TEST);
    if (cull) glEnable(GL_CULL_FACE);
    return true;
}
void ProgressBar::release() { vao.release(); program.release(); }
}

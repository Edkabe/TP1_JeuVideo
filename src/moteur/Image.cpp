#include "moteur/Image.h"
#include "moteur/Matrix4.h"
#include "moteur/Engine.h"
#include "moteur/ProgramCache.h"
#include <SDL3/SDL.h>
#include <filesystem>
#include <algorithm>
#include <cmath>

namespace moteur {
Image::Image(std::shared_ptr<Texture> value) { setTexture(std::move(value)); }
bool Image::setTexture(std::shared_ptr<Texture> value) {
    error.clear();
    if (value && !value->isLoaded()) { error = "Texture non chargee."; return false; }
    texture = std::move(value);
    dimensions = texture ? Vector2{static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight())} : Vector2{};
    return true;
}
bool Image::setPosition(Vector2 value) {
    error.clear();
    if (!std::isfinite(value.x) || !std::isfinite(value.y)) { error = "Position invalide."; return false; }
    position = value;
    return true;
}
bool Image::resize(Vector2 bounds, bool keepAspectRatio) {
    error.clear();
    if (!std::isfinite(bounds.x) || !std::isfinite(bounds.y) || bounds.x < 0 || bounds.y < 0) {
        error = "Dimensions invalides."; return false;
    }
    if (keepAspectRatio) {
        if (!texture || !texture->isLoaded() || texture->getWidth() <= 0 || texture->getHeight() <= 0) {
            error = "Texture chargee necessaire pour calculer les proportions."; return false;
        }
        const float scale = (std::min)(bounds.x / texture->getWidth(), bounds.y / texture->getHeight());
        dimensions = {texture->getWidth() * scale, texture->getHeight() * scale};
    } else dimensions = bounds;
    return true;
}
bool Image::createGraphics() {
    if (program.getId() && vao.getId() && vbo.getId()) return true;
    const char* base = SDL_GetBasePath();
    if (!base) { error = SDL_GetError(); return false; }
    const auto directory = std::filesystem::path(base) / "assets/shaders";
    ProgramCache cache;
    if (!cache.load(program, {{GL_VERTEX_SHADER, directory / "image.vert"},
                              {GL_FRAGMENT_SHADER, directory / "image.frag"}},
                    std::filesystem::path(base) / "cache/programs/image.bin", Engine::getInstance().getSettings())) {
        error = cache.getError(); return false;
    }
    if (!vao.create() || !vbo.create()) { error = "Creation du rendu Image impossible."; return false; }
    constexpr GLfloat vertices[] = {
        0,0,0,0, 0,1,0,1, 1,1,1,1,
        0,0,0,0, 1,1,1,1, 1,0,1,0
    };
    GLint oldVao = 0, oldBuffer = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldBuffer);
    vao.bind();
    vbo.setData(vertices, sizeof(vertices), GL_STATIC_DRAW);
    vao.setAttribute(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    vao.setAttribute(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 2 * sizeof(GLfloat));
    glBindVertexArray(static_cast<GLuint>(oldVao));
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(oldBuffer));
    return true;
}
bool Image::render() {
    error.clear();
    if (!SDL_GL_GetCurrentContext()) { error = "Contexte OpenGL actif necessaire."; return false; }
    if (!texture || !texture->isLoaded()) { error = "Aucune texture chargee."; return false; }
    if (dimensions.x == 0 || dimensions.y == 0) return true;
    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] <= 0 || viewport[3] <= 0) return true;
    if (!createGraphics()) return false;
    GLint oldProgram = 0, oldVao = 0, oldActive = 0, oldTexture = 0, oldSampler = 0;
    GLint srcRgb = 0, dstRgb = 0, srcAlpha = 0, dstAlpha = 0, eqRgb = 0, eqAlpha = 0;
    const auto blend = glIsEnabled(GL_BLEND), depth = glIsEnabled(GL_DEPTH_TEST), cull = glIsEnabled(GL_CULL_FACE);
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActive);
    glGetIntegerv(GL_BLEND_SRC_RGB, &srcRgb); glGetIntegerv(GL_BLEND_DST_RGB, &dstRgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha); glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &eqRgb); glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &eqAlpha);
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);
    glGetIntegerv(GL_SAMPLER_BINDING, &oldSampler);
    glBindSampler(0, 0);
    glEnable(GL_BLEND); glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    program.use(); vao.bind(); texture->bind();
    const auto projection = Matrix4::orthographic(0, static_cast<float>(viewport[2]), static_cast<float>(viewport[3]), 0);
    glUniformMatrix4fv(program.getUniformLocation("uProjection"), 1, GL_FALSE, projection.data());
    glUniform2f(program.getUniformLocation("uPosition"), position.x, position.y);
    glUniform2f(program.getUniformLocation("uSize"), dimensions.x, dimensions.y);
    glUniform1i(program.getUniformLocation("uTexture"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldTexture)); glBindSampler(0, static_cast<GLuint>(oldSampler));
    glActiveTexture(static_cast<GLenum>(oldActive));
    glBindVertexArray(static_cast<GLuint>(oldVao)); glUseProgram(static_cast<GLuint>(oldProgram));
    glBlendFuncSeparate(srcRgb, dstRgb, srcAlpha, dstAlpha); glBlendEquationSeparate(eqRgb, eqAlpha);
    if (!blend) glDisable(GL_BLEND);
    if (depth) glEnable(GL_DEPTH_TEST);
    if (cull) glEnable(GL_CULL_FACE);
    return true;
}
void Image::release() { vbo.release(); vao.release(); program.release(); }
}

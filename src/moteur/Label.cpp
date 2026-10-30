#include "moteur/Label.h"
#include "moteur/ProgramCache.h"
#include "moteur/Engine.h"
#include <SDL3/SDL.h>
#include <algorithm>

namespace moteur {
Label::Label(std::shared_ptr<Font> font, std::string text) : font_(std::move(font)), text_(std::move(text)) {}
Label::~Label() { release(); }

bool Label::setFont(std::shared_ptr<Font> font) {
    if (font && !font->isLoaded()) {
        error_ = "La police doit etre chargee avant son utilisation.";
        return false;
    }
    font_ = std::move(font);
    textureDirty_ = true;
    error_.clear();
    return true;
}

bool Label::setText(std::string text) {
    text_ = std::move(text);
    textureDirty_ = true;
    error_.clear();
    if (font_ && font_->isLoaded() && !text_.empty()) {
        int width = 0, height = 0;
        if (!font_->measureText(text_, width, height)) {
            error_ = "Mesure du texte impossible.";
            return false;
        }
    }
    return true;
}

Vector2 Label::getDimensions() const {
    if (!font_ || !font_->isLoaded() || text_.empty()) return {};
    int width = 0, height = 0;
    if (!font_->measureText(text_, width, height)) return {};
    return {static_cast<float>(width), static_cast<float>(height)};
}

bool Label::createGraphics() {
    if (program_.getId() && vao_.getId() && vbo_.getId() && viewportUniform_ >= 0 &&
        positionUniform_ >= 0 && sizeUniform_ >= 0 && colorUniform_ >= 0 && samplerUniform_ >= 0) return true;
    error_.clear();
    const char* base = SDL_GetBasePath();
    if (!base) { error_ = SDL_GetError(); return false; }
    const auto directory = std::filesystem::path(base) / "assets/shaders";
    ProgramCache cache;
    if (!cache.load(program_, {{GL_VERTEX_SHADER, directory / "label.vert"},
                               {GL_FRAGMENT_SHADER, directory / "label.frag"}},
                    std::filesystem::path(base) / "cache/programs/label.bin", Engine::getInstance().getSettings())) {
        error_ = cache.getError(); return false;
    }
    if (!vao_.create() || !vbo_.create()) { error_ = "Creation des objets du texte impossible."; return false; }

    // Un quad unité permet de changer position et taille sans renvoyer ses sommets.
    constexpr GLfloat vertices[] = {
        0, 0, 0, 0,  0, 1, 0, 1,  1, 1, 1, 1,
        0, 0, 0, 0,  1, 1, 1, 1,  1, 0, 1, 0
    };
    GLint previousVao = 0, previousBuffer = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBuffer);
    vao_.bind();
    vbo_.bind();
    vbo_.setData(vertices, sizeof(vertices), GL_STATIC_DRAW);
    vao_.setAttribute(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    vao_.setAttribute(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 2 * sizeof(GLfloat));
    glBindVertexArray(static_cast<GLuint>(previousVao));
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previousBuffer));

    viewportUniform_ = program_.getUniformLocation("uViewport");
    positionUniform_ = program_.getUniformLocation("uPosition");
    sizeUniform_ = program_.getUniformLocation("uSize");
    colorUniform_ = program_.getUniformLocation("uColor");
    samplerUniform_ = program_.getUniformLocation("uText");
    if (viewportUniform_ < 0 || positionUniform_ < 0 || sizeUniform_ < 0 || colorUniform_ < 0 || samplerUniform_ < 0) {
        error_ = "Uniform du texte introuvable.";
        return false;
    }
    return true;
}

bool Label::updateTexture() {
    if (text_.empty()) {
        if (texture_) glDeleteTextures(1, &texture_);
        texture_ = 0;
        textureWidth_ = textureHeight_ = 0;
        textureDirty_ = false;
        return true;
    }
    if (!font_ || !font_->isLoaded()) { error_ = "Aucune police chargee pour le texte."; return false; }
    if (!SDL_GL_GetCurrentContext()) { error_ = "Un contexte OpenGL actif est necessaire."; return false; }

    if (!font_->createTextTexture(text_, texture_, textureWidth_, textureHeight_)) {
        error_ = font_->getError(); return false;
    }
    textureDirty_ = false;
    return true;
}

bool Label::render() {
    error_.clear();
    if (!SDL_GL_GetCurrentContext()) { error_ = "Un contexte OpenGL actif est necessaire."; return false; }
    if (text_.empty()) return !textureDirty_ || updateTexture();
    if (!font_ || !font_->isLoaded()) { error_ = "Aucune police chargee pour le texte."; return false; }
    if (!createGraphics()) return false;
    if (textureDirty_ && !updateTexture()) return false;
    if (!texture_) return true;

    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] <= 0 || viewport[3] <= 0) return true;
    GLint oldProgram = 0, oldVao = 0, oldActiveTexture = GL_TEXTURE0;
    GLint oldBlendSrcRgb = GL_ONE, oldBlendDstRgb = GL_ZERO;
    GLint oldBlendSrcAlpha = GL_ONE, oldBlendDstAlpha = GL_ZERO;
    GLint oldTexture = 0;
    const GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActiveTexture);
    glGetIntegerv(GL_BLEND_SRC_RGB, &oldBlendSrcRgb);
    glGetIntegerv(GL_BLEND_DST_RGB, &oldBlendDstRgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &oldBlendSrcAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &oldBlendDstAlpha);
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    program_.use();
    vao_.bind();
    glBindTexture(GL_TEXTURE_2D, texture_);
    glUniform2f(viewportUniform_, static_cast<float>(viewport[2]), static_cast<float>(viewport[3]));
    glUniform2f(positionUniform_, position_.x, position_.y);
    glUniform2f(sizeUniform_, static_cast<float>(textureWidth_), static_cast<float>(textureHeight_));
    glUniform4f(colorUniform_, color_.r, color_.g, color_.b, color_.a);
    glUniform1i(samplerUniform_, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldTexture));
    glActiveTexture(static_cast<GLenum>(oldActiveTexture));
    glBindVertexArray(static_cast<GLuint>(oldVao));
    glUseProgram(static_cast<GLuint>(oldProgram));
    glBlendFuncSeparate(static_cast<GLenum>(oldBlendSrcRgb), static_cast<GLenum>(oldBlendDstRgb),
                        static_cast<GLenum>(oldBlendSrcAlpha), static_cast<GLenum>(oldBlendDstAlpha));
    if (!blendWasEnabled) glDisable(GL_BLEND);
    return glGetError() == GL_NO_ERROR;
}

void Label::release() {
    if (texture_) { glDeleteTextures(1, &texture_); texture_ = 0; }
    textureWidth_ = textureHeight_ = 0;
    textureDirty_ = true;
    vbo_.release();
    vao_.release();
    program_.release();
    vertexShader_.release();
    fragmentShader_.release();
}
}

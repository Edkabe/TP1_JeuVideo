#include "moteur/Font.h"
#include <cmath>
#include <memory>
namespace moteur {
Font::~Font() { release(); }
void Font::release() {
    if (handle) { TTF_CloseFont(handle); handle = nullptr; }
    if (ttfInitialized) { TTF_Quit(); ttfInitialized = false; }
    loaded = false;
}
int Font::getHeight() const { return handle ? TTF_GetFontHeight(handle) : 0; }
bool Font::measureText(const std::string& text, int& width, int& height) const {
    width = 0;
    height = 0;
    if (!handle) return false;
    return TTF_GetStringSize(handle, text.c_str(), text.size(), &width, &height);
}
bool Font::load(const std::string& path) {
    error.clear();
    if (!std::isfinite(pointSize) || pointSize <= 0) { error = "Taille de police invalide."; return false; }
    if (!TTF_Init()) { error = SDL_GetError(); return false; }
    TTF_Font* candidate = TTF_OpenFont(path.c_str(), pointSize);
    if (!candidate) { error = SDL_GetError(); TTF_Quit(); return false; }
    // La référence TTF du candidat maintient la bibliothèque initialisée.
    release();
    handle = candidate; ttfInitialized = true; loaded = true;
    return true;
}
}

namespace moteur {
bool Font::createTextTexture(const std::string& text, GLuint& texture, int& width, int& height) {
    error.clear();
    if (!handle || !SDL_GL_GetCurrentContext()) { error = "Police chargee et contexte OpenGL actif requis."; return false; }
    if (text.empty()) {
        if (texture) glDeleteTextures(1, &texture);
        texture = 0; width = height = 0; return true;
    }
    struct SurfaceDeleter { void operator()(SDL_Surface* value) const { SDL_DestroySurface(value); } };
    const SDL_Color white{255, 255, 255, 255};
    using Surface = std::unique_ptr<SDL_Surface, SurfaceDeleter>;
    Surface rendered(TTF_RenderText_Blended(handle, text.c_str(), text.size(), white));
    if (!rendered) { error = SDL_GetError(); return false; }
    Surface pixels(SDL_ConvertSurface(rendered.get(), SDL_PIXELFORMAT_RGBA32));
    if (!pixels || !SDL_LockSurface(pixels.get())) { error = SDL_GetError(); return false; }

    GLint oldTexture = 0, oldAlignment = 4, oldRowLength = 0;
    GLint oldSkipRows = 0, oldSkipPixels = 0, oldPixelBuffer = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldAlignment);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &oldRowLength);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS, &oldSkipRows);
    glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &oldSkipPixels);
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &oldPixelBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, pixels->pitch / 4);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    GLuint candidate = 0;
    glGenTextures(1, &candidate);
    if (candidate) {
        glBindTexture(GL_TEXTURE_2D, candidate);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pixels->w, pixels->h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixels->pixels);
    }
    GLint uploadedWidth = 0;
    if (candidate) glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &uploadedWidth);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldTexture));
    glPixelStorei(GL_UNPACK_ALIGNMENT, oldAlignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, oldRowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, oldSkipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, oldSkipPixels);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, static_cast<GLuint>(oldPixelBuffer));
    SDL_UnlockSurface(pixels.get());

    if (!candidate || uploadedWidth != pixels->w) {
        if (candidate) glDeleteTextures(1, &candidate);
        error = "Envoi du texte a OpenGL impossible.";
        return false;
    }
    if (texture) glDeleteTextures(1, &texture);
    texture = candidate;
    width = pixels->w;
    height = pixels->h;
    return true;
}
}

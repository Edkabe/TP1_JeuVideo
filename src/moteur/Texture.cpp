#include "moteur/Texture.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <memory>

namespace moteur {
    Texture::~Texture() { release(); }
    void Texture::release() {
        if (id) { glDeleteTextures(1, &id); id = 0; }
        width = height = 0; loaded = false;
    }
    void Texture::bind() const { glBindTexture(GL_TEXTURE_2D, id); }
    void Texture::unbind() { glBindTexture(GL_TEXTURE_2D, 0); }
    bool Texture::load(const std::string& path) {
        error.clear();
        if (!SDL_GL_GetCurrentContext() || !glGenTextures) {
            error = "Un contexte OpenGL initialise est necessaire."; return false;
        }
        using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;
        Surface source(IMG_Load(path.c_str()), SDL_DestroySurface);
        if (!source) { error = SDL_GetError(); return false; }
        Surface pixels(SDL_ConvertSurface(source.get(), SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
        if (!pixels || !SDL_LockSurface(pixels.get())) { error = SDL_GetError(); return false; }
        GLint limit = 0;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &limit);
        if (pixels->w > limit || pixels->h > limit) {
            SDL_UnlockSurface(pixels.get()); error = "Image trop grande pour ce pilote."; return false;
        }
        GLuint candidate = 0;
        glGenTextures(1, &candidate);
        if (!candidate) { SDL_UnlockSurface(pixels.get()); error = "Creation de texture impossible."; return false; }
        GLint binding, alignment, rowLength, skipRows, skipPixels, pixelBuffer;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &binding);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
        glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
        glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows);
        glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skipPixels);
        glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &pixelBuffer);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, pixels->pitch / 4);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glBindTexture(GL_TEXTURE_2D, candidate);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pixels->w, pixels->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels->pixels);
        GLint uploadedWidth = 0;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &uploadedWidth);
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(binding));
        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipPixels);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, static_cast<GLuint>(pixelBuffer));
        SDL_UnlockSurface(pixels.get());
        if (uploadedWidth != pixels->w) {
            glDeleteTextures(1, &candidate); error = "Envoi de l'image a OpenGL impossible."; return false;
        }
        release();
        id = candidate; width = pixels->w; height = pixels->h; loaded = true;
        return true;
    }
}

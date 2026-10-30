#pragma once
#include "moteur/Resource.h"
#include <glad/gl.h>

namespace moteur {
class Texture final : public Resource {
public:
    Texture() = default;
    explicit Texture(std::string title) : Resource(std::move(title)) {}
    ~Texture() override;
    bool load(const std::string& path) override;
    void release() override;
    // Parametres conserves lors d'un rechargement. Contexte courant requis.
    bool setFiltering(GLenum minification, GLenum magnification);
    bool setWrapping(GLenum horizontal, GLenum vertical);
    void bind() const;
    static void unbind();
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    explicit operator GLuint() const { return id; }
private:
    GLenum minFilter = GL_LINEAR, magFilter = GL_LINEAR;
    GLenum wrapS = GL_CLAMP_TO_EDGE, wrapT = GL_CLAMP_TO_EDGE;
    GLuint id = 0;
    int width = 0, height = 0;
};
}

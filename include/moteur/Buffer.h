#pragma once
#include <glad/gl.h>
namespace moteur {
/// Contrat d'un tampon GPU; un contexte courant est requis pour ses operations.
class Buffer {
public:
    virtual ~Buffer() = default;
    virtual bool create() = 0;
    virtual void bind() const = 0;
    virtual void setData(const void* data, GLsizeiptr size, GLenum usage = GL_DYNAMIC_DRAW) const = 0;
    virtual void release() = 0;
    virtual GLuint getId() const = 0;
};
}

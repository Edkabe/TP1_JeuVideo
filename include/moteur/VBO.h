#pragma once

#include <glad/gl.h>

namespace moteur {

// Petit propriétaire RAII d'un Vertex Buffer Object OpenGL.
class VBO {
public:
    VBO() = default;
    ~VBO();

    VBO(const VBO&) = delete;
    VBO& operator=(const VBO&) = delete;

    bool create();
    void bind() const;
    static void unbind();
    void setData(const void* data, GLsizeiptr size, GLenum usage = GL_DYNAMIC_DRAW) const;
    void release();
    GLuint getId() const { return id; }

private:
    GLuint id = 0;
};

}

#include "moteur/VBO.h"

namespace moteur {

VBO::~VBO() { release(); }

bool VBO::create() {
    GLuint candidate = 0;
    glGenBuffers(1, &candidate);
    if (!candidate) return false;
    release();
    id = candidate;
    return true;
}

void VBO::bind() const { glBindBuffer(GL_ARRAY_BUFFER, id); }
void VBO::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

void VBO::setData(const void* data, GLsizeiptr size, GLenum usage) const {
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void VBO::release() {
    if (id) {
        glDeleteBuffers(1, &id);
        id = 0;
    }
}

}

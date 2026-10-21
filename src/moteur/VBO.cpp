#include "moteur/VBO.h"
#include <utility>

namespace moteur {
VBO::~VBO() { release(); }
VBO::VBO(VBO&& other) noexcept : id(std::exchange(other.id, 0)) {}
VBO& VBO::operator=(VBO&& other) noexcept {
    if (this != &other) {
        release();
        id = std::exchange(other.id, 0);
    }
    return *this;
}
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
    // Remettre à zéro rend les appels répétés sans effet.
    if (id) { glDeleteBuffers(1, &id); id = 0; }
}
}

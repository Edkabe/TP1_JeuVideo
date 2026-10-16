#include "moteur/VAO.h"
#include <utility>
namespace moteur {
VAO::~VAO() { release(); }
VAO::VAO(VAO&& other) noexcept : id(std::exchange(other.id, 0)) {}
VAO& VAO::operator=(VAO&& other) noexcept {
    if (this != &other) {
        release();
        id = std::exchange(other.id, 0);
    }
    return *this;
}
bool VAO::create() {
    GLuint candidate = 0;
    glGenVertexArrays(1, &candidate);
    if (!candidate) return false;
    release(); id = candidate; return true;
}
void VAO::bind() const { glBindVertexArray(id); }
void VAO::unbind() { glBindVertexArray(0); }
void VAO::setAttribute(GLuint index, GLint size, GLenum type, GLboolean normalized,
                       GLsizei stride, std::size_t offset) const {
    bind();
    // OpenGL capture le VBO lié à cet instant : le délier ensuite reste permis.
    glVertexAttribPointer(index, size, type, normalized, stride,
                          reinterpret_cast<const void*>(offset));
    glEnableVertexAttribArray(index);
}
void VAO::release() { if (id) { glDeleteVertexArrays(1, &id); id = 0; } }
}

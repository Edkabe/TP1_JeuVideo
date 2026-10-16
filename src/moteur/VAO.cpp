#include "moteur/VAO.h"
namespace moteur {
VAO::~VAO() { release(); }
bool VAO::create() {
    GLuint candidate = 0;
    glGenVertexArrays(1, &candidate);
    if (!candidate) return false;
    release(); id = candidate; return true;
}
void VAO::bind() const { glBindVertexArray(id); }
void VAO::unbind() { glBindVertexArray(0); }
void VAO::release() { if (id) { glDeleteVertexArrays(1, &id); id = 0; } }
}

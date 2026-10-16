#include "moteur/Program.h"
#include <vector>

namespace moteur {
Program::~Program() { release(); }
void Program::release() { if (id) { glDeleteProgram(id); id = 0; } }
void Program::use() const { glUseProgram(id); }
void Program::unuse() { glUseProgram(0); }
bool Program::link(std::initializer_list<const Shader*> shaders) {
    error.clear();
    if (!shaders.size()) { error = "Aucun nuanceur fourni."; return false; }
    for (const auto* shader : shaders) {
        if (!shader || !shader->getId()) { error = "Nuanceur absent ou non compile."; return false; }
    }
    const GLuint candidate = glCreateProgram();
    if (!candidate) { error = "Creation du programme impossible."; return false; }
    for (const auto* shader : shaders) glAttachShader(candidate, shader->getId());
    glLinkProgram(candidate);
    GLint success = GL_FALSE;
    glGetProgramiv(candidate, GL_LINK_STATUS, &success);
    for (const auto* shader : shaders) glDetachShader(candidate, shader->getId());
    if (success != GL_TRUE) {
        GLint size = 0;
        glGetProgramiv(candidate, GL_INFO_LOG_LENGTH, &size);
        std::vector<char> log(static_cast<size_t>(size > 1 ? size : 1));
        glGetProgramInfoLog(candidate, static_cast<GLsizei>(log.size()), nullptr, log.data());
        error = log.data(); glDeleteProgram(candidate); return false;
    }
    release();
    id = candidate;
    return true;
}
}

#include "moteur/Shader.h"
#include <fstream>
#include <iterator>
#include <vector>
#include <limits>

namespace moteur {
Shader::Shader(GLenum type) : type(type) {}
Shader::~Shader() { release(); }
void Shader::release() { if (id) { glDeleteShader(id); id = 0; } }
bool Shader::loadShader(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) { error = "Nuanceur introuvable : " + path; return false; }
    const std::string source((std::istreambuf_iterator<char>(file)), {});
    if (file.bad()) { error = "Lecture du nuanceur impossible : " + path; return false; }
    return compile(source);
}
bool Shader::compile(const std::string& source) {
    error.clear();
    if (source.empty() || source.size() > static_cast<size_t>(std::numeric_limits<GLint>::max())) {
        error = "Source de nuanceur vide ou trop longue."; return false;
    }
    const GLuint candidate = glCreateShader(type);
    if (!candidate) { error = "Creation du nuanceur impossible."; return false; }
    const char* data = source.data();
    const GLint size = static_cast<GLint>(source.size());
    glShaderSource(candidate, 1, &data, &size);
    glCompileShader(candidate);
    GLint success = GL_FALSE;
    glGetShaderiv(candidate, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        GLint sizeLog = 0;
        glGetShaderiv(candidate, GL_INFO_LOG_LENGTH, &sizeLog);
        std::vector<char> log(static_cast<size_t>(sizeLog > 1 ? sizeLog : 1));
        glGetShaderInfoLog(candidate, static_cast<GLsizei>(log.size()), nullptr, log.data());
        error = log.data();
        glDeleteShader(candidate);
        return false;
    }
    release(); // Conserver l'ancien nuanceur si une recompilation échoue.
    id = candidate;
    return true;
}
}

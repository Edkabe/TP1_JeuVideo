#include "moteur/Program.h"
#include <vector>
#include <limits>
#include <algorithm>

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
        if (supportsBinary()) glProgramParameteri(candidate, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
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
    bool Program::supportsBinary() {
        // Notre GLAD expose ces fonctions a partir d'OpenGL 4.1.
        if (!GLAD_GL_VERSION_4_1 || !glGetProgramBinary || !glProgramBinary || !glProgramParameteri) return false;
        GLint count = 0;
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &count);
        return count > 0;
    }

    bool Program::getBinary(Binary& binary) {
        error.clear();
        if (!supportsBinary()) { error = "Binaires de programmes non disponibles."; return false; }
        if (!id) { error = "Programme absent."; return false; }
        GLint length = 0;
        glGetProgramiv(id, GL_PROGRAM_BINARY_LENGTH, &length);
        if (length <= 0) { error = "Binaire de programme indisponible."; return false; }
        Binary candidate;
        candidate.data.resize(static_cast<size_t>(length));
        GLsizei written = 0;
        glGetProgramBinary(id, length, &written, &candidate.format, candidate.data.data());
        if (written <= 0 || written > length) { error = "Extraction du binaire echouee."; return false; }
        candidate.data.resize(static_cast<size_t>(written));
        binary = std::move(candidate);
        return true;
    }

    bool Program::loadBinary(const Binary& binary) {
        error.clear();
        if (!supportsBinary()) { error = "Binaires de programmes non disponibles."; return false; }
        if (binary.data.empty() || binary.data.size() > static_cast<size_t>((std::numeric_limits<GLsizei>::max)())) {
            error = "Taille du binaire invalide."; return false;
        }
        GLint count = 0;
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &count);
        std::vector<GLint> formats(static_cast<size_t>(count));
        glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, formats.data());
        if (std::none_of(formats.begin(), formats.end(), [&](GLint format) { return static_cast<GLenum>(format) == binary.format; })) {
            error = "Format binaire non pris en charge."; return false;
        }
        const GLuint candidate = glCreateProgram();
        if (!candidate) { error = "Creation du programme impossible."; return false; }
        glProgramBinary(candidate, binary.format, binary.data.data(), static_cast<GLsizei>(binary.data.size()));
        GLint success = GL_FALSE;
        glGetProgramiv(candidate, GL_LINK_STATUS, &success);
        if (success != GL_TRUE) {
            error = "Chargement du binaire echoue; recompiler les sources.";
            glDeleteProgram(candidate);
            return false;
        }
        release();
        id = candidate;
        return true;
    }
}

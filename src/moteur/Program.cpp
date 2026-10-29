#include "moteur/Program.h"
#include <vector>
#include <limits>
#include <algorithm>
#include <SDL3/SDL.h>
#include <string_view>

namespace moteur {
Program::~Program() { release(); }
    void Program::release() { if (id) { glDeleteProgram(id); id = 0; } }
    void Program::use() const { glUseProgram(id); }
    void Program::unuse() { glUseProgram(0); }
    bool Program::link(std::initializer_list<const Shader*> shaders) {
        return link(std::vector<const Shader*>(shaders));
    }
    bool Program::link(const std::vector<const Shader*>& shaders) {
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
        if (!SDL_GL_GetCurrentContext()) return false;
        // Un pilote 3.3 peut proposer l'extension sans exposer le coeur 4.1.
        // Notre GLAD ne charge pas ces trois entrees dans ce cas.
        if (!GLAD_GL_VERSION_4_1) {
            if (!glGetStringi) return false;
            GLint extensions = 0; glGetIntegerv(GL_NUM_EXTENSIONS, &extensions);
            bool available = false;
            for (GLint i = 0; i < extensions; ++i) {
                const auto* name = glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i));
                if (name && std::string_view(reinterpret_cast<const char*>(name)) == "GL_ARB_get_program_binary") {
                    available = true; break;
                }
            }
            if (!available) return false;
            glad_glGetProgramBinary = reinterpret_cast<PFNGLGETPROGRAMBINARYPROC>(SDL_GL_GetProcAddress("glGetProgramBinary"));
            glad_glProgramBinary = reinterpret_cast<PFNGLPROGRAMBINARYPROC>(SDL_GL_GetProcAddress("glProgramBinary"));
            glad_glProgramParameteri = reinterpret_cast<PFNGLPROGRAMPARAMETERIPROC>(SDL_GL_GetProcAddress("glProgramParameteri"));
        }
        if (!glGetProgramBinary || !glProgramBinary || !glProgramParameteri) return false;
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

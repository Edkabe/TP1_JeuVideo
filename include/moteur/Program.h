#pragma once
#include "moteur/Shader.h"
#include <initializer_list>
#include <vector>

namespace moteur {
class Program {
public:
    struct Binary {
        GLenum format = 0;
        std::vector<unsigned char> data;
    };
    Program() = default;
    ~Program();
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
    bool link(std::initializer_list<const Shader*> shaders);
    bool link(const std::vector<const Shader*>& shaders);
    // Necessite un contexte OpenGL courant et charge par GLAD.
    static bool supportsBinary();
    bool getBinary(Binary& binary);
    bool loadBinary(const Binary& binary);
    // -1 si le nom est absent ou le programme vide.
    GLint getUniformLocation(const char* name) const;
    GLint getAttributeLocation(const char* name) const;
    void use() const;
    static void unuse();
    void release();
    GLuint getId() const { return id; }
    const std::string& getError() const { return error; }
private:
    GLuint id = 0;
    std::string error;
};
}

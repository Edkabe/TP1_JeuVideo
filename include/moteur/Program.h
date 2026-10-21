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
    // Necessite un contexte OpenGL courant et charge par GLAD.
    static bool supportsBinary();
    bool getBinary(Binary& binary);
    bool loadBinary(const Binary& binary);
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

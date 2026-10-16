#pragma once
#include "moteur/Shader.h"
#include <initializer_list>

namespace moteur {
class Program {
public:
    Program() = default;
    ~Program();
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
    bool link(std::initializer_list<const Shader*> shaders);
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

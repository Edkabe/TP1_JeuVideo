#pragma once
#include <glad/gl.h>
#include <string>

namespace moteur {
class Shader {
public:
    explicit Shader(GLenum type);
    virtual ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    bool loadShader(const std::string& filePath);
    bool compile(const std::string& source);
    void release();
    GLuint getId() const { return id; }
    const std::string& getError() const { return error; }
private:
    GLenum type;
    GLuint id = 0;
    std::string error;
};
}

#pragma once
#include <glad/gl.h>
namespace moteur {
class VAO {
public:
    VAO() = default;
    ~VAO();
    VAO(const VAO&) = delete;
    VAO& operator=(const VAO&) = delete;
    bool create();
    void bind() const;
    static void unbind();
    void release();
    GLuint getId() const { return id; }
private:
    GLuint id = 0;
};
}

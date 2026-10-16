#pragma once
#include <glad/gl.h>
#include <cstddef>
namespace moteur {
// Le VAO mémorise le format des attributs et leur tampon de sommets.
// Il ne possède pas le VBO : chaque wrapper libère son propre identifiant.
// Créer/utiliser/détruire avec le contexte OpenGL propriétaire actif.
class VAO {
public:
    VAO() = default;
    ~VAO();
    VAO(const VAO&) = delete;
    VAO& operator=(const VAO&) = delete;
    VAO(VAO&& other) noexcept;
    VAO& operator=(VAO&& other) noexcept;
    bool create();
    void bind() const;
    static void unbind();
    // Lier d'abord le VBO. offset et stride sont exprimés en octets.
    // Cette méthode lie ce VAO et active un attribut flottant (glVertexAttribPointer).
    void setAttribute(GLuint index, GLint size, GLenum type, GLboolean normalized,
                      GLsizei stride, std::size_t offset) const;
    void release();
    // Identifiant emprunté, à ne pas supprimer depuis le code appelant.
    GLuint getId() const { return id; }
private:
    GLuint id = 0;
};
}

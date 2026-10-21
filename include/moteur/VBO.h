#pragma once
#include <glad/gl.h>

namespace moteur {
// Propriétaire unique d'un tampon de sommets sur le GPU.
// Le contexte OpenGL doit être actif de create() jusqu'à release()/destruction.
class VBO {
public:
    VBO() = default;
    ~VBO();
    VBO(const VBO&) = delete;
    VBO& operator=(const VBO&) = delete;
    // Déplacer transfère la propriété : l'objet source devient vide.
    VBO(VBO&& other) noexcept;
    VBO& operator=(VBO&& other) noexcept;

    bool create();
    void bind() const;
    static void unbind();
    // Remplace les données du tampon. nullptr réserve seulement de la mémoire.
    // Préconditions : create() réussi, size >= 0 et usage OpenGL valide.
    void setData(const void* data, GLsizeiptr size, GLenum usage = GL_DYNAMIC_DRAW) const;
    void release();
    // Identifiant emprunté : l'appelant ne doit jamais le supprimer lui-même.
    GLuint getId() const { return id; }
private:
    GLuint id = 0;
};
}

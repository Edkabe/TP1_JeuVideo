#pragma once
#include "moteur/Texture.h"
#include "moteur/Vector2.h"
#include "moteur/Program.h"
#include "moteur/VAO.h"
#include "moteur/VBO.h"
#include <memory>

namespace moteur {
// Position en pixels, origine en haut a gauche du viewport.
// Detruire/liberer les objets graphiques avant de fermer le contexte.
class Image {
public:
    explicit Image(std::shared_ptr<Texture> texture = {});
    bool setTexture(std::shared_ptr<Texture> texture);
    const std::shared_ptr<Texture>& getTexture() const { return texture; }
    bool setPosition(Vector2 value);
    Vector2 getPosition() const { return position; }
    Vector2 getDimensions() const { return dimensions; }
    // Avec respect de l'aspect, inscrit l'image dans le rectangle demande.
    bool resize(Vector2 bounds, bool keepAspectRatio = false);
    bool render();
    void release(); // Libere le rendu propre au controle, pas la texture partagee.
    const std::string& getError() const { return error; }
private:
    bool createGraphics();
    std::shared_ptr<Texture> texture;
    Vector2 position, dimensions;
    Program program;
    VAO vao;
    VBO vbo;
    std::string error;
};
}

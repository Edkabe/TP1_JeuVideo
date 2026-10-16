#include "moteur/Bibliotheques.h"
#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include <iostream>

namespace moteur
{
    bool verifierBibliotheques()
    {
        // Le chargement réel de GLAD attendra un contexte OpenGL actif.
        auto chargeur = &gladLoadGL;
        const glm::vec2 position(2.0f, 3.0f);
        const glm::vec2 deplacement(1.0f, 1.0f);
        const auto resultat = position + deplacement;
        std::cout << "SDL3 : " << SDL_GetVersion()
                  << " | SDL_image : " << IMG_Version()
                  << " | SDL_ttf : " << TTF_Version() << '\n';
        return chargeur != nullptr && resultat.x == 3.0f && resultat.y == 4.0f;
    }
}

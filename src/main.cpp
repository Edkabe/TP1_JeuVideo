#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "moteur/Bibliotheques.h"

/// @brief Fonction principale
/// @param argc Nombre d'arguments
/// @param argv Arguments
/// @return Code de fin de programme
int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  return moteur::verifierBibliotheques() ? 0 : 1;
}

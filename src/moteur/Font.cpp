#include "moteur/Font.h"
#include <cmath>
namespace moteur {
Font::~Font() { release(); }
void Font::release() {
    if (handle) { TTF_CloseFont(handle); handle = nullptr; }
    if (ttfInitialized) { TTF_Quit(); ttfInitialized = false; }
    loaded = false;
}
int Font::getHeight() const { return handle ? TTF_GetFontHeight(handle) : 0; }
bool Font::load(const std::string& path) {
    error.clear();
    if (!std::isfinite(pointSize) || pointSize <= 0) { error = "Taille de police invalide."; return false; }
    if (!TTF_Init()) { error = SDL_GetError(); return false; }
    TTF_Font* candidate = TTF_OpenFont(path.c_str(), pointSize);
    if (!candidate) { error = SDL_GetError(); TTF_Quit(); return false; }
    // La référence TTF du candidat maintient la bibliothèque initialisée.
    release();
    handle = candidate; ttfInitialized = true; loaded = true;
    return true;
}
}

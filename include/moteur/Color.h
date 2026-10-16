#pragma once

namespace moteur {

/// @brief Couleur RGBA dont les composantes sont normalisées entre 0 et 1.
struct Color {
    float r;
    float g;
    float b;
    float a;

    /// @brief Construit une couleur; par défaut, le blanc est opaque.
    constexpr Color(float rouge = 1.0f, float vert = 1.0f,
                    float bleu = 1.0f, float alpha = 1.0f)
        : r(rouge), g(vert), b(bleu), a(alpha) {}
};

} // namespace moteur

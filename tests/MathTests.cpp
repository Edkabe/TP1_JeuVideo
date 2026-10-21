#include "moteur/Color.h"
#include "moteur/Matrix4.h"
#include "moteur/Vector2.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
int checks = 0;
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
bool near(float actual, float expected) { return std::fabs(actual - expected) < 0.0001f; }
}

int main() {
    try {
        const moteur::Vector2 a{2.0f, 3.0f};
        check(a + moteur::Vector2{1.0f, -1.0f} == moteur::Vector2{3.0f, 2.0f}, "Addition Vector2");
        check(near(a.length(), std::sqrt(13.0f)), "Longueur Vector2");
        check(a.normalized().length() > 0.9999f && a.normalized().length() < 1.0001f, "Normalisation Vector2");
        const moteur::Vector3 x{1, 0, 0};
        const moteur::Vector3 y{0, 1, 0};
        check(x.cross(y) == moteur::Vector3{0, 0, 1}, "Produit vectoriel");

        const auto translated = moteur::Matrix4::translation({4, -2, 1});
        check(translated.transformPoint({1, 2, 3}) == moteur::Vector3{5, 0, 4}, "Translation connue");
        const auto scaled = moteur::Matrix4::scaling({2, 3, 4});
        check(scaled.transformPoint({1, 2, 3}) == moteur::Vector3{2, 6, 12}, "Échelle connue");
        const auto quarterTurn = moteur::Matrix4::rotationZ(3.14159265358979323846f / 2.0f);
        const auto rotated = quarterTurn.transformPoint({1, 0, 0});
        check(near(rotated.x, 0) && near(rotated.y, 1), "Rotation connue");
        const auto ortho = moteur::Matrix4::orthographic(0, 800, 0, 600);
        const auto origin = ortho.transformPoint({0, 0, 0});
        const auto center = ortho.transformPoint({400, 300, 0});
        const auto upperRight = ortho.transformPoint({800, 600, 0});
        check(near(origin.x, -1) && near(origin.y, -1), "Coin bas gauche orthographique");
        check(near(center.x, 0) && near(center.y, 0), "Centre orthographique");
        check(near(upperRight.x, 1) && near(upperRight.y, 1), "Coin haut droit orthographique");
        const auto combined = moteur::Matrix4::translation({4, 0, 0}) * moteur::Matrix4::scaling({2, 2, 2});
        check(combined.transformPoint({1, 0, 0}) == moteur::Vector3{6, 0, 0}, "Composition colonne-major");

        constexpr moteur::Color white{};
        constexpr moteur::Color custom{0.1f, 0.2f, 0.3f, 0.4f};
        check(white.r == 1 && white.g == 1 && white.b == 1 && white.a == 1, "Couleur par défaut");
        check(custom.r == 0.1f && custom.g == 0.2f && custom.b == 0.3f && custom.a == 0.4f, "Couleur configurable");

        std::cout << "Tests mathématiques réussis : " << checks << " vérifications.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Échec après " << checks << " vérifications : " << error.what() << '\n';
        return 1;
    }
}

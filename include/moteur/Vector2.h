#pragma once
#include <cmath>

namespace moteur {

/// @brief Vecteur 2D utilisé pour les positions et dimensions à l'écran.
struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vector2() = default;
    constexpr Vector2(float xValue, float yValue) : x(xValue), y(yValue) {}

    constexpr Vector2 operator+(Vector2 other) const { return {x + other.x, y + other.y}; }
    constexpr Vector2 operator-(Vector2 other) const { return {x - other.x, y - other.y}; }
    constexpr Vector2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
    constexpr Vector2 operator/(float scalar) const { return {x / scalar, y / scalar}; }
    constexpr Vector2& operator+=(Vector2 other) { x += other.x; y += other.y; return *this; }
    constexpr Vector2& operator-=(Vector2 other) { x -= other.x; y -= other.y; return *this; }
    constexpr bool operator==(Vector2 other) const { return x == other.x && y == other.y; }
    constexpr bool operator!=(Vector2 other) const { return !(*this == other); }

    constexpr float dot(Vector2 other) const { return x * other.x + y * other.y; }
    constexpr float lengthSquared() const { return dot(*this); }
    float length() const { return std::sqrt(lengthSquared()); }
    Vector2 normalized() const {
        const float magnitude = length();
        return magnitude == 0.0f ? Vector2{} : *this / magnitude;
    }
};

constexpr Vector2 operator*(float scalar, Vector2 vector) { return vector * scalar; }

} // namespace moteur

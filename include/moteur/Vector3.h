#pragma once
#include <cmath>

namespace moteur {

/// @brief Vecteur 3D avec les opérations géométriques courantes.
struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vector3() = default;
    constexpr Vector3(float xValue, float yValue, float zValue)
        : x(xValue), y(yValue), z(zValue) {}

    constexpr Vector3 operator+(Vector3 other) const { return {x + other.x, y + other.y, z + other.z}; }
    constexpr Vector3 operator-(Vector3 other) const { return {x - other.x, y - other.y, z - other.z}; }
    constexpr Vector3 operator*(float scalar) const { return {x * scalar, y * scalar, z * scalar}; }
    constexpr Vector3 operator/(float scalar) const { return {x / scalar, y / scalar, z / scalar}; }
    constexpr bool operator==(Vector3 other) const { return x == other.x && y == other.y && z == other.z; }
    constexpr bool operator!=(Vector3 other) const { return !(*this == other); }

    constexpr float dot(Vector3 other) const { return x * other.x + y * other.y + z * other.z; }
    constexpr Vector3 cross(Vector3 other) const {
        return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
    }
    constexpr float lengthSquared() const { return dot(*this); }
    float length() const { return std::sqrt(lengthSquared()); }
    Vector3 normalized() const {
        const float magnitude = length();
        return magnitude == 0.0f ? Vector3{} : *this / magnitude;
    }
};

constexpr Vector3 operator*(float scalar, Vector3 vector) { return vector * scalar; }

} // namespace moteur

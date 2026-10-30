#pragma once
#include "moteur/Vector3.h"
#include <array>
#include <cmath>

namespace moteur {

/// @brief Matrice 4x4 colonne-major, compatible avec les matrices OpenGL.
class Matrix4 {
public:
    /// @brief Construit une matrice identité.
    constexpr Matrix4() : values{1, 0, 0, 0, 0, 1, 0, 0,
                                 0, 0, 1, 0, 0, 0, 0, 1} {}

    /// @brief Construit une matrice à partir de 16 valeurs colonne-major.
    explicit constexpr Matrix4(std::array<float, 16> columnMajor) : values(columnMajor) {}

    static constexpr Matrix4 identity() { return Matrix4{}; }
    static constexpr Matrix4 translation(Vector3 offset) {
        return Matrix4{{1, 0, 0, 0, 0, 1, 0, 0,
                        0, 0, 1, 0, offset.x, offset.y, offset.z, 1}};
    }
    static constexpr Matrix4 scaling(Vector3 scale) {
        return Matrix4{{scale.x, 0, 0, 0, 0, scale.y, 0, 0,
                        0, 0, scale.z, 0, 0, 0, 0, 1}};
    }
    static Matrix4 rotationZ(float radians) {
        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);
        return Matrix4{{cosine, sine, 0, 0, -sine, cosine, 0, 0,
                        0, 0, 1, 0, 0, 0, 0, 1}};
    }

    /// @brief Projection orthographique suivant la convention OpenGL (-1 à 1).
    static Matrix4 orthographic(float left, float right, float bottom, float top,
                                float nearPlane = -1.0f, float farPlane = 1.0f) {
        if (left == right || bottom == top || nearPlane == farPlane)
            return Matrix4{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
        return Matrix4{{2.0f / (right - left), 0, 0, 0,
                        0, 2.0f / (top - bottom), 0, 0,
                        0, 0, -2.0f / (farPlane - nearPlane), 0,
                        -(right + left) / (right - left),
                        -(top + bottom) / (top - bottom),
                        -(farPlane + nearPlane) / (farPlane - nearPlane), 1}};
    }

    constexpr float operator()(int row, int column) const { return values[column * 4 + row]; }
    explicit constexpr operator const float*() const { return values.data(); }
    constexpr const float* data() const { return values.data(); }

    constexpr Matrix4 operator*(const Matrix4& other) const {
        std::array<float, 16> product{};
        for (int column = 0; column < 4; ++column)
            for (int row = 0; row < 4; ++row)
                for (int k = 0; k < 4; ++k)
                    product[column * 4 + row] += (*this)(row, k) * other(k, column);
        return Matrix4{product};
    }

    /// @brief Transforme un point 3D (coordonnée homogène w=1).
    constexpr Vector3 transformPoint(Vector3 point) const {
        const float x = (*this)(0, 0) * point.x + (*this)(0, 1) * point.y +
                        (*this)(0, 2) * point.z + (*this)(0, 3);
        const float y = (*this)(1, 0) * point.x + (*this)(1, 1) * point.y +
                        (*this)(1, 2) * point.z + (*this)(1, 3);
        const float z = (*this)(2, 0) * point.x + (*this)(2, 1) * point.y +
                        (*this)(2, 2) * point.z + (*this)(2, 3);
        const float w = (*this)(3, 0) * point.x + (*this)(3, 1) * point.y +
                        (*this)(3, 2) * point.z + (*this)(3, 3);
        return w == 0.0f || w == 1.0f ? Vector3{x, y, z} : Vector3{x / w, y / w, z / w};
    }

private:
    std::array<float, 16> values;
};

} // namespace moteur

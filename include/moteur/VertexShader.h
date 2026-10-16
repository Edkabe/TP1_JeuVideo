#pragma once
#include "moteur/Shader.h"
namespace moteur {
class VertexShader final : public Shader {
public:
    VertexShader() : Shader(GL_VERTEX_SHADER) {}
};
}

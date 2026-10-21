#pragma once

#include "moteur/Color.h"
#include "moteur/Font.h"
#include "moteur/Program.h"
#include "moteur/VAO.h"
#include "moteur/VBO.h"
#include "moteur/Vector2.h"
#include <memory>
#include <string>

namespace moteur {
// Contrôle texte 2D dessiné dans le viewport OpenGL courant.
class Label final {
public:
    Label() = default;
    Label(std::shared_ptr<Font> font, std::string text = {});
    ~Label();
    Label(const Label&) = delete;
    Label& operator=(const Label&) = delete;

    bool setFont(std::shared_ptr<Font> font);
    bool setText(std::string text);
    void setPosition(Vector2 position) { position_ = position; }
    void setColor(Color color) { color_ = color; }

    const std::shared_ptr<Font>& getFont() const { return font_; }
    const std::string& getText() const { return text_; }
    Vector2 getPosition() const { return position_; }
    Color getColor() const { return color_; }
    Vector2 getDimensions() const;
    const std::string& getError() const { return error_; }

    // Les objets OpenGL détenus par l'étiquette doivent être détruits avec le contexte actif.
    bool render();
    void release();

private:
    bool createGraphics();
    bool updateTexture();

    std::shared_ptr<Font> font_;
    std::string text_;
    Vector2 position_{};
    Color color_{};
    std::string error_;
    GLuint texture_ = 0;
    int textureWidth_ = 0;
    int textureHeight_ = 0;
    bool textureDirty_ = true;
    Shader vertexShader_{GL_VERTEX_SHADER};
    Shader fragmentShader_{GL_FRAGMENT_SHADER};
    Program program_;
    VAO vao_;
    VBO vbo_;
    GLint viewportUniform_ = -1;
    GLint positionUniform_ = -1;
    GLint sizeUniform_ = -1;
    GLint colorUniform_ = -1;
    GLint samplerUniform_ = -1;
};
}

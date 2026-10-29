#pragma once

#include "moteur/Color.h"
#include "moteur/EventObserver.h"
#include "moteur/Program.h"
#include "moteur/VAO.h"
#include "moteur/VBO.h"
#include "moteur/VertexShader.h"
#include <SDL3/SDL.h>
#include <array>
#include <cstddef>
#include <string>

namespace moteur {

// Case carrée dessinée en pixels, depuis le coin supérieur gauche de la fenêtre.
class CheckBox final : public EventObserver {
public:
    CheckBox() = default;
    ~CheckBox() = default;

    CheckBox(const CheckBox&) = delete;
    CheckBox& operator=(const CheckBox&) = delete;

    // À appeler lorsque le contexte OpenGL est actif.
    bool initialize();
    void render(float viewportWidth, float viewportHeight);

    void setPosition(float x, float y) { position = {x, y}; }
    void setSize(float value) { size = value > 0.0f ? value : 0.0f; }
    void setBorderColor(Color value) { borderColor = value; }
    void setBackgroundColor(Color value) { backgroundColor = value; }
    void setCheckColor(Color value) { checkColor = value; }

    float getX() const { return position[0]; }
    float getY() const { return position[1]; }
    float getSize() const { return size; }
    bool isChecked() const { return checked; }
    void setChecked(bool value) { checked = value; }
    const std::string& getError() const { return error; }

    // Retourne vrai si le clic gauche a touché la case et modifié son état.
    bool handleEvent(const SDL_Event& event);
    // Adapte l'événement de scène au gestionnaire historique de la case.
    void onEvent(const Event& event) override { handleEvent(event.getSDLEvent()); }

private:
    struct Vertex {
        float x, y;
        float red, green, blue, alpha;
    };

    void addTriangle(float ax, float ay, float bx, float by, float cx, float cy,
                     Color color, std::array<Vertex, 42>& vertices, std::size_t& count) const;
    void addRectangle(float x, float y, float width, float height, Color color,
                      std::array<Vertex, 42>& vertices, std::size_t& count) const;
    void addLine(float x1, float y1, float x2, float y2, float thickness, Color color,
                 std::array<Vertex, 42>& vertices, std::size_t& count) const;

    std::array<float, 2> position{24.0f, 24.0f};
    float size = 32.0f;
    Color borderColor{0.12f, 0.12f, 0.14f, 1.0f};
    Color backgroundColor{0.92f, 0.92f, 0.92f, 1.0f};
    Color checkColor{0.10f, 0.48f, 0.24f, 1.0f};
    bool checked = false;
    bool initialized = false;
    GLint resolutionLocation = -1;
    std::string error;
    VertexShader vertexShader;
    Shader fragmentShader{GL_FRAGMENT_SHADER};
    Program program;
    VAO vao;
    VBO vbo;
};

}

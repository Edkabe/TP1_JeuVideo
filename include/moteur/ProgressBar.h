#pragma once
#include "moteur/VisualComponent.h"
#include "moteur/Color.h"
#include "moteur/Vector2.h"
#include "moteur/Program.h"
#include "moteur/VAO.h"

namespace moteur {
// Controle horizontal, en pixels du viewport depuis son coin haut gauche.
// Detruire/liberer les objets graphiques avant de fermer le contexte OpenGL.
class ProgressBar final : public VisualComponent {
public:
    bool setPosition(Vector2 value);
    bool setDimensions(Vector2 value);
    Vector2 getPosition() const { return position; }
    Vector2 getDimensions() const { return dimensions; }
    // Refuse les bornes non finies ou minimum >= maximum. Borne la valeur actuelle.
    bool setRange(float minimum, float maximum);
    bool setValue(float value);
    bool advance(float amount);
    float getMinimum() const { return minimum; }
    float getMaximum() const { return maximum; }
    float getValue() const { return value; }
    float getProgress() const;
    void setBorderColor(Color color) { border = color; }
    void setBackgroundColor(Color color) { background = color; }
    void setProgressColor(Color color) { progress = color; }
    bool setBorderWidth(float width);
    bool render() override;
    void release() override;
    const std::string& getError() const { return error; }
private:
    bool createGraphics();
    Vector2 position{}, dimensions{240, 24};
    float minimum = 0, value = 0, maximum = 100, borderWidth = 2;
    Color border{0.7f,0.7f,0.7f}, background{0.12f,0.12f,0.16f}, progress{0.15f,0.7f,0.35f};
    Program program;
    VAO vao;
    std::string error;
};
}

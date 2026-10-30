#pragma once
#include "moteur/VisualComponent.h"

#include "moteur/Color.h"
#include "moteur/EventObserver.h"
#include "moteur/Program.h"
#include "moteur/VAO.h"
#include "moteur/Vector2.h"
#include <string>

namespace moteur {

// Curseur horizontal en pixels. Une scène garde l'objet vivant et l'inscrit
// aux événements souris de son EventManager.
class Slider final : public VisualComponent, public EventObserver {
public:
    Slider() = default;
    ~Slider() override = default;

    Slider(const Slider&) = delete;
    Slider& operator=(const Slider&) = delete;

    bool setPosition(Vector2 value);
    bool setDimensions(Vector2 value);
    bool setRange(float minimum, float maximum, float increment);
    bool setMinimum(float value);
    bool setMaximum(float value);
    bool setIncrement(float value);
    bool setValue(float value);

    Vector2 getPosition() const { return position; }
    Vector2 getDimensions() const { return dimensions; }
    float getMinimum() const { return minimum; }
    float getMaximum() const { return maximum; }
    float getIncrement() const { return increment; }
    float getValue() const { return value; }

    void setBorderColor(Color color) { borderColor = color; }
    void setBackgroundColor(Color color) { backgroundColor = color; }
    void setProgressColor(Color color) { progressColor = color; }
    void setHoverColor(Color color) { hoverColor = color; }

    // Rend la barre et sa poignée dans le viewport OpenGL courant.
    bool render() override;
    void onEvent(const Event& event) override;
    void release() override;
    bool isHovered() const { return hovered; }
    bool isDragging() const { return dragging; }
    const std::string& getError() const { return error; }

private:
    float thumbWidth() const;
    float valueRatio() const;
    float thumbLeft() const;
    bool containsThumb(Vector2 point) const;
    void updateFromMouse(float x);
    bool createGraphics();

    Vector2 position{24.0f, 72.0f};
    Vector2 dimensions{240.0f, 28.0f};
    float minimum = 0.0f;
    float maximum = 100.0f;
    float increment = 1.0f;
    float value = 50.0f;
    Color borderColor{0.08f, 0.10f, 0.14f, 1.0f};
    Color backgroundColor{0.82f, 0.85f, 0.90f, 1.0f};
    Color progressColor{0.10f, 0.48f, 0.78f, 1.0f};
    Color hoverColor{0.20f, 0.72f, 0.96f, 1.0f};
    bool hovered = false;
    bool dragging = false;
    float dragOffset = 0.0f;
    Program program;
    VAO vao;
    std::string error;
};

} // namespace moteur

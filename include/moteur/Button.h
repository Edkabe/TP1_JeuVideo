#pragma once
#include "moteur/EventObserver.h"
#include "moteur/Label.h"
#include <functional>

namespace moteur {
// Inscrire le meme shared_ptr<Button> aux mouvements, boutons et pertes de focus
// du gestionnaire de la scene. Celui-ci ne conserve que des references faibles.
class Button final : public EventObserver {
public:
    Button(std::shared_ptr<Font> font = {}, std::string title = {});
    bool setPosition(Vector2 value);
    bool setDimensions(Vector2 value);
    Vector2 getPosition() const { return position; }
    Vector2 getDimensions() const { return dimensions; }
    bool setTitle(std::string value);
    bool setFont(std::shared_ptr<Font> value);
    void setTextColor(Color value) { label.setColor(value); }
    void setBorderColor(Color value) { border = value; }
    void setBackgroundColor(Color value) { background = value; }
    void setHoverColor(Color value) { hover = value; }
    bool setBorderWidth(float value);
    const Label& getLabel() const { return label; }
    bool isHovered() const { return hovered; }
    bool isPressed() const { return pressed; }
    // Evenement de clic : appui gauche puis relachement a l'interieur.
    void setOnClick(std::function<void()> handler) { onClick = std::move(handler); }
    void onEvent(const Event& event) override;
    bool render();
    void release();
    const std::string& getError() const { return error; }
private:
    bool contains(Vector2 point) const;
    void centerTitle();
    bool createGraphics();
    Vector2 position{}, dimensions{180, 48};
    Color border{0.7f,0.7f,0.7f}, background{0.15f,0.2f,0.3f}, hover{0.25f,0.4f,0.6f};
    float borderWidth = 2;
    bool hovered = false, pressed = false;
    Label label;
    Program program;
    VAO vao;
    std::function<void()> onClick;
    std::string error;
};
}

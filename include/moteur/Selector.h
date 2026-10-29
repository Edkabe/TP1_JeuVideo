#pragma once
#include "moteur/Button.h"
#include <vector>
#include <limits>

namespace moteur {
// Inscrire le selecteur (pas ses boutons internes) au gestionnaire de la scene.
class Selector final : public EventObserver {
public:
    static constexpr size_t noSelection = (std::numeric_limits<size_t>::max)();
    explicit Selector(std::shared_ptr<Font> font = {}, std::vector<std::string> items = {});
    bool setItems(std::vector<std::string> items);
    const std::vector<std::string>& getItems() const { return items; }
    size_t getSelectedIndex() const { return selected; }
    // nullptr si vide. Pointeur emprunte, invalide apres remplacement de la liste.
    const std::string* getSelectedItem() const;
    bool setSelectedIndex(size_t index);
    // Navigation bornee, sans retour circulaire. false si aucun changement.
    bool previous();
    bool next();
    bool setPosition(Vector2 value);
    bool setDimensions(Vector2 value);
    Vector2 getPosition() const { return position; }
    Vector2 getDimensions() const { return dimensions; }
    bool setFont(std::shared_ptr<Font> font);
    void setTextColor(Color color);
    void setBorderColor(Color color);
    void setBackgroundColor(Color color);
    void setHoverColor(Color color);
    const Label& getLabel() const { return label; }
    const Button& getPreviousButton() const { return previousButton; }
    const Button& getNextButton() const { return nextButton; }
    void onEvent(const Event& event) override;
    bool render();
    void release();
    const std::string& getError() const { return error; }
private:
    void layout();
    void centerLabel();
    std::vector<std::string> items;
    size_t selected = noSelection;
    Vector2 position{}, dimensions{300, 48};
    Label label;
    Button previousButton, nextButton;
    std::string error;
};
}

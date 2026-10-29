#include "moteur/Selector.h"
#include <algorithm>
#include <cmath>

namespace moteur {
Selector::Selector(std::shared_ptr<Font> font, std::vector<std::string> values)
    : label(font), previousButton(font, "<"), nextButton(std::move(font), ">") {
    previousButton.setOnClick([this] { previous(); });
    nextButton.setOnClick([this] { next(); });
    setItems(std::move(values));
    layout();
}
const std::string* Selector::getSelectedItem() const {
    return selected < items.size() ? &items[selected] : nullptr;
}
bool Selector::setItems(std::vector<std::string> values) {
    error.clear();
    if (!label.setText(values.empty() ? "" : values.front())) { error = label.getError(); return false; }
    items = std::move(values);
    selected = items.empty() ? noSelection : 0;
    layout(); // Annule un appui eventuel sur l'ancienne liste.
    return true;
}
bool Selector::setSelectedIndex(size_t index) {
    error.clear();
    if (index >= items.size()) { error = "Indice de selection hors limites."; return false; }
    if (!label.setText(items[index])) { error = label.getError(); return false; }
    selected = index; centerLabel(); return true;
}
bool Selector::previous() {
    error.clear();
    if (items.empty() || selected == 0) return false;
    return setSelectedIndex(selected - 1);
}
bool Selector::next() {
    error.clear();
    if (items.empty() || selected >= items.size() - 1) return false;
    return setSelectedIndex(selected + 1);
}
void Selector::centerLabel() {
    label.setPosition(position + (dimensions - label.getDimensions()) * .5f);
}
void Selector::layout() {
    const float width = (std::min)(dimensions.y, dimensions.x / 3.0f);
    previousButton.setPosition(position); previousButton.setDimensions({width, dimensions.y});
    nextButton.setPosition({position.x + dimensions.x - width, position.y});
    nextButton.setDimensions({width, dimensions.y});
    centerLabel();
}
bool Selector::setPosition(Vector2 candidate) {
    error.clear();
    if (!std::isfinite(candidate.x) || !std::isfinite(candidate.y)) { error = "Position invalide."; return false; }
    position = candidate; layout(); return true;
}
bool Selector::setDimensions(Vector2 candidate) {
    error.clear();
    if (!std::isfinite(candidate.x) || !std::isfinite(candidate.y) || candidate.x < 0 || candidate.y < 0) {
        error = "Dimensions invalides."; return false;
    }
    dimensions = candidate; layout(); return true;
}
bool Selector::setFont(std::shared_ptr<Font> font) {
    error.clear();
    if (font && !font->isLoaded()) { error = "Police non chargee."; return false; }
    if (!label.setFont(font) || !previousButton.setFont(font) || !nextButton.setFont(std::move(font))) {
        error = "Affectation de la police impossible."; return false;
    }
    centerLabel(); return true;
}
void Selector::setTextColor(Color color) { label.setColor(color); previousButton.setTextColor(color); nextButton.setTextColor(color); }
void Selector::setBorderColor(Color color) { previousButton.setBorderColor(color); nextButton.setBorderColor(color); }
void Selector::setBackgroundColor(Color color) { previousButton.setBackgroundColor(color); nextButton.setBackgroundColor(color); }
void Selector::setHoverColor(Color color) { previousButton.setHoverColor(color); nextButton.setHoverColor(color); }
void Selector::onEvent(const Event& event) { previousButton.onEvent(event); nextButton.onEvent(event); }
bool Selector::render() {
    error.clear();
    if (dimensions.x == 0 || dimensions.y == 0) return true;
    centerLabel();
    if (!previousButton.render()) { error = previousButton.getError(); return false; }
    if (!nextButton.render()) { error = nextButton.getError(); return false; }
    // Label conserve ses liaisons et son melange. Le selecteur encadre aussi
    // profondeur/culling/equation pour afficher le texte comme un controle 2D.
    const auto depth = glIsEnabled(GL_DEPTH_TEST), cull = glIsEnabled(GL_CULL_FACE);
    GLint rgb = 0, alpha = 0;
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &rgb); glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &alpha);
    glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE); glBlendEquation(GL_FUNC_ADD);
    const bool success = label.render();
    glBlendEquationSeparate(rgb, alpha);
    if (depth) glEnable(GL_DEPTH_TEST);
    if (cull) glEnable(GL_CULL_FACE);
    if (!success) error = label.getError();
    return success;
}
void Selector::release() { label.release(); previousButton.release(); nextButton.release(); }
}

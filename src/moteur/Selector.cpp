#include "moteur/Selector.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>

namespace moteur {
namespace {
Vector2 selectorMousePosition(float x, float y, SDL_WindowID windowId) {
    SDL_Window* window = SDL_GetWindowFromID(windowId);
    if (!window || !SDL_GL_GetCurrentContext()) return {x, y};
    int logicalWidth = 0, logicalHeight = 0, pixelWidth = 0, pixelHeight = 0;
    if (!SDL_GetWindowSize(window, &logicalWidth, &logicalHeight) ||
        logicalWidth <= 0 || logicalHeight <= 0 ||
        !SDL_GetWindowSizeInPixels(window, &pixelWidth, &pixelHeight)) return {x, y};
    // Les boutons internes utilisent les coordonnées du viewport OpenGL.
    // Appliquer exactement la même conversion évite que les clics sur la
    // valeur et ceux sur les flèches ciblent des zones différentes, surtout
    // quand la fenêtre est mise à l'échelle par Windows.
    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    return {x * pixelWidth / logicalWidth - viewport[0],
            y * pixelHeight / logicalHeight - (pixelHeight - viewport[1] - viewport[3])};
}
}

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
void Selector::onEvent(const Event& event) {
    previousButton.onEvent(event);
    nextButton.onEvent(event);

    const auto& sdl = event.getSDLEvent();
    if (sdl.type == SDL_EVENT_WINDOW_FOCUS_LOST || sdl.type == SDL_EVENT_WINDOW_MOUSE_LEAVE) {
        pressedDirection = 0;
        return;
    }
    if ((sdl.type != SDL_EVENT_MOUSE_BUTTON_DOWN && sdl.type != SDL_EVENT_MOUSE_BUTTON_UP) ||
        sdl.button.button != SDL_BUTTON_LEFT) return;

    const Vector2 point = selectorMousePosition(sdl.button.x, sdl.button.y, sdl.button.windowID);
    const float arrowWidth = (std::min)(dimensions.y, dimensions.x / 3.0f);
    const float centerLeft = position.x + arrowWidth;
    const float centerRight = position.x + dimensions.x - arrowWidth;
    const float middle = position.x + dimensions.x * 0.5f;
    const bool inCenter = point.x >= centerLeft && point.x < centerRight &&
        point.y >= position.y && point.y < position.y + dimensions.y;

    if (sdl.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        pressedDirection = inCenter ? (point.x < middle ? -1 : 1) : 0;
        return;
    }

    const int direction = pressedDirection;
    pressedDirection = 0;
    if (!inCenter || direction == 0 || (point.x < middle ? -1 : 1) != direction) return;
    if (direction < 0) previous();
    else next();
}
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

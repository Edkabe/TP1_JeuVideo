#include "moteur/DemoGameScene.h"
#include "moteur/Engine.h"
#include "moteur/Localization.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <utility>

namespace moteur {
namespace {
constexpr float roundDuration = 30.0f;
constexpr Uint32 mouseEvents[] = {
    SDL_EVENT_MOUSE_MOTION,
    SDL_EVENT_MOUSE_BUTTON_DOWN,
    SDL_EVENT_MOUSE_BUTTON_UP,
    SDL_EVENT_WINDOW_FOCUS_LOST,
    SDL_EVENT_WINDOW_MOUSE_LEAVE
};

class CallbackObserver final : public EventObserver {
public:
    explicit CallbackObserver(std::function<void(const Event&)> callback)
        : callback(std::move(callback)) {}
    void onEvent(const Event& event) override { callback(event); }
private:
    std::function<void(const Event&)> callback;
};

std::string scoreText(const std::string& language, int score) {
    return std::string(localized(language, "Cibles touchées : ", "Targets hit: ")) + std::to_string(score);
}

std::string timeText(const std::string& language, float seconds) {
    return std::string(localized(language, "Temps : ", "Time: ")) +
        std::to_string(static_cast<int>(std::ceil((std::max)(0.0f, seconds)))) + " s";
}
}

DemoGameScene::DemoGameScene(SceneManager& manager, std::filesystem::path file, std::string destination)
    : scenes(manager), fontFile(std::move(file)), returnScene(std::move(destination)) {}

bool DemoGameScene::load() {
    if (loaded) return true;
    error.clear();
    if (!SDL_GL_GetCurrentContext() || !scenes.hasScene(returnScene)) {
        error = "Contexte ou scene de retour indisponible.";
        return false;
    }

    auto& engine = Engine::getInstance();
    auto& resources = engine.getResourceManager();
    const auto& settings = engine.getSettings();
    sensitivity = settings.getSensitivity();
    fieldOfView = settings.getFov();
    const auto language = settings.getLanguage();
    const auto key = "game:font:" + fontFile.u8string();
    auto font = resources.get<Font>(key);
    if (!font && resources.contains(key)) { error = "Nom de police utilise par une autre ressource."; return false; }
    if (!font) font = std::make_shared<Font>(24.0f, key);
    if (!font->isLoaded() && !font->load(fontFile.u8string())) { error = font->getError(); return false; }
    if (!resources.contains(key) && !resources.add(font)) { error = "Ajout de la police impossible."; return false; }

    title = std::make_unique<Label>(font, localized(language, "Tir aux cibles", "Target practice"));
    instructions = std::make_unique<Label>(font,
        localized(language, "Déplace le viseur avec la souris et clique les cibles avant la fin du temps.",
                  "Move the crosshair with the mouse and click targets before time runs out."));
    scoreLabel = std::make_unique<Label>(font);
    timeLabel = std::make_unique<Label>(font);
    reticle = std::make_unique<Label>(font, "+");
    reticle->setColor({1.0f, 0.88f, 0.18f, 1.0f});
    progress = std::make_unique<ProgressBar>();
    target = std::make_shared<Button>(font, localized(language, "CIBLE", "TARGET"));
    back = std::make_shared<Button>(font, localized(language, "Retour au menu", "Back to menu"));
    restart = std::make_shared<Button>(font, localized(language, "Recommencer", "Restart"));

    for (const auto& button : {target, back, restart}) {
        button->setBorderColor({0.12f, 0.62f, 0.68f, 1.0f});
        button->setBackgroundColor({0.06f, 0.22f, 0.28f, 1.0f});
        button->setHoverColor({0.10f, 0.42f, 0.50f, 1.0f});
        button->setTextColor({1.0f, 1.0f, 1.0f, 1.0f});
    }
    target->setBorderColor({0.92f, 0.38f, 0.22f, 1.0f});
    target->setBackgroundColor({0.66f, 0.13f, 0.10f, 1.0f});
    target->setHoverColor({0.82f, 0.22f, 0.15f, 1.0f});
    back->setOnClick([this] {
        if (!scenes.setActiveScene(returnScene)) error = scenes.getError();
    });
    restart->setOnClick([this] { startGame(); });
    inputObserver = std::make_shared<CallbackObserver>([this](const Event& event) { handleGameInput(event); });

    for (Uint32 type : mouseEvents) {
        if (!getEventManager().addObserver(back, type) ||
            !getEventManager().addObserver(restart, type) ||
            !getEventManager().addObserver(inputObserver, type)) {
            error = "Inscription d'un controle aux evenements impossible.";
            unload();
            return false;
        }
    }

    int width = 0, height = 0;
    engine.getWindow().getPixelSize(width, height);
    reticlePosition = {width * 0.5f, height * 0.5f};
    loaded = true;
    layout(width, height);
    startGame();
    return true;
}

void DemoGameScene::layout(int width, int height) {
    if (!loaded || width <= 0 || height <= 0) return;
    const float w = static_cast<float>(width), h = static_cast<float>(height);
    const auto& language = Engine::getInstance().getSettings().getLanguage();
    const float radians = fieldOfView * 0.00872664625997f;
    const float fovScale = std::tan(radians) / std::tan(0.78539816339f);
    const float targetSide = std::clamp(88.0f / fovScale, 48.0f, 128.0f);
    const Vector2 targetSize{targetSide, targetSide};
    if (target->getDimensions() != targetSize) target->setDimensions(targetSize);

    title->setPosition({(w - title->getDimensions().x) * 0.5f, h * 0.035f});
    scoreLabel->setText(scoreText(language, score));
    timeLabel->setText(timeText(language, timeRemaining));
    scoreLabel->setPosition({28.0f, h * 0.04f});
    timeLabel->setPosition({w - timeLabel->getDimensions().x - 28.0f, h * 0.04f});
    instructions->setPosition({(w - instructions->getDimensions().x) * 0.5f, h * 0.105f});
    progress->setDimensions({(std::max)(80.0f, w - 56.0f), 18.0f});
    progress->setPosition({28.0f, h * 0.17f});

    reticlePosition.x = std::clamp(reticlePosition.x, 0.0f, w);
    reticlePosition.y = std::clamp(reticlePosition.y, h * 0.20f, h - 82.0f);
    reticle->setPosition({reticlePosition.x - reticle->getDimensions().x * 0.5f,
                          reticlePosition.y - reticle->getDimensions().y * 0.5f});

    const float buttonY = h - 64.0f;
    const Vector2 backSize{std::min(220.0f, w * 0.42f), 44.0f};
    const Vector2 restartSize{std::min(190.0f, w * 0.38f), 44.0f};
    const Vector2 backPosition{24.0f, buttonY};
    const Vector2 restartPosition{w - restartSize.x - 24.0f, buttonY};
    if (back->getDimensions() != backSize) back->setDimensions(backSize);
    if (back->getPosition() != backPosition) back->setPosition(backPosition);
    if (restart->getDimensions() != restartSize) restart->setDimensions(restartSize);
    if (restart->getPosition() != restartPosition) restart->setPosition(restartPosition);

    targetPosition.x = std::clamp(targetPosition.x, 12.0f, (std::max)(12.0f, w - targetSize.x - 12.0f));
    targetPosition.y = std::clamp(targetPosition.y, h * 0.22f,
                                  (std::max)(h * 0.22f, buttonY - targetSize.y - 12.0f));
    target->setPosition(targetPosition);
}

void DemoGameScene::startGame() {
    if (!loaded) return;
    score = 0;
    timeRemaining = roundDuration;
    gameOver = false;
    const auto& language = Engine::getInstance().getSettings().getLanguage();
    instructions->setText(localized(language,
        "Déplace le viseur avec la souris et clique les cibles avant la fin du temps.",
        "Move the crosshair with the mouse and click targets before time runs out."));
    progress->setRange(0.0f, roundDuration);
    progress->setValue(roundDuration);
    moveTarget();
}

void DemoGameScene::moveTarget() {
    if (!target) return;
    int width = 0, height = 0;
    if (!Engine::getInstance().getWindow().getPixelSize(width, height)) return;
    const auto size = target->getDimensions();
    const float minX = 12.0f;
    const float maxX = (std::max)(minX, static_cast<float>(width) - size.x - 12.0f);
    const float minY = static_cast<float>(height) * 0.22f;
    const float maxY = (std::max)(minY, static_cast<float>(height) - 64.0f - size.y - 12.0f);
    std::uniform_real_distribution<float> x(minX, maxX), y(minY, maxY);
    targetPosition = {x(random), y(random)};
    target->setPosition(targetPosition);
}

void DemoGameScene::handleGameInput(const Event& event) {
    if (!loaded) return;
    const auto& sdl = event.getSDLEvent();
    if (sdl.type == SDL_EVENT_WINDOW_FOCUS_LOST || sdl.type == SDL_EVENT_WINDOW_MOUSE_LEAVE) return;
    if (sdl.type == SDL_EVENT_MOUSE_MOTION) {
        SDL_Window* window = Engine::getInstance().getWindow().getSDLWindow();
        if (!window) return;
        int logicalWidth = 0, logicalHeight = 0, pixelWidth = 0, pixelHeight = 0;
        if (!SDL_GetWindowSize(window, &logicalWidth, &logicalHeight) ||
            logicalWidth <= 0 || logicalHeight <= 0 ||
            !SDL_GetWindowSizeInPixels(window, &pixelWidth, &pixelHeight)) return;
        reticlePosition.x += sdl.motion.xrel * sensitivity * pixelWidth / logicalWidth;
        reticlePosition.y += sdl.motion.yrel * sensitivity * pixelHeight / logicalHeight;
        int width = 0, height = 0;
        if (Engine::getInstance().getWindow().getPixelSize(width, height)) {
            reticlePosition.x = std::clamp(reticlePosition.x, 0.0f, static_cast<float>(width));
            reticlePosition.y = std::clamp(reticlePosition.y, height * 0.20f, static_cast<float>(height) - 82.0f);
        }
        return;
    }
    if (sdl.type == SDL_EVENT_MOUSE_BUTTON_DOWN && sdl.button.button == SDL_BUTTON_LEFT &&
        !gameOver && reticleHitsTarget()) {
        ++score;
        moveTarget();
    }
}

bool DemoGameScene::reticleHitsTarget() const {
    const auto size = target ? target->getDimensions() : Vector2{};
    return size.x > 0.0f && size.y > 0.0f &&
        reticlePosition.x >= targetPosition.x && reticlePosition.x <= targetPosition.x + size.x &&
        reticlePosition.y >= targetPosition.y && reticlePosition.y <= targetPosition.y + size.y;
}

void DemoGameScene::update(float deltaTime) {
    if (!loaded || !std::isfinite(deltaTime) || deltaTime < 0.0f || gameOver) return;
    timeRemaining = (std::max)(0.0f, timeRemaining - deltaTime);
    progress->setValue(timeRemaining);
    if (timeRemaining <= 0.0f) {
        gameOver = true;
        const auto& language = Engine::getInstance().getSettings().getLanguage();
        instructions->setText(std::string(localized(language, "Temps écoulé — score final : ", "Time's up — final score: ")) +
                              std::to_string(score));
    }
}

void DemoGameScene::render() {
    if (!loaded) return;
    auto& engine = Engine::getInstance();
    int width = 0, height = 0;
    if (!engine.getWindow().getPixelSize(width, height)) { error = SDL_GetError(); engine.stop(); return; }
    if (width <= 0 || height <= 0) return;
    glViewport(0, 0, width, height);
    glClearColor(0.035f, 0.12f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBlendEquation(GL_FUNC_ADD);
    layout(width, height);
    if (!title->render()) error = title->getError();
    else if (!scoreLabel->render()) error = scoreLabel->getError();
    else if (!timeLabel->render()) error = timeLabel->getError();
    else if (!instructions->render()) error = instructions->getError();
    else if (!progress->render()) error = progress->getError();
    else if (!target->render()) error = target->getError();
    else if (!reticle->render()) error = reticle->getError();
    else if (!back->render()) error = back->getError();
    else if (!restart->render()) error = restart->getError();
    else return;
    engine.stop();
}

void DemoGameScene::unload() {
    getEventManager().removeObserver(inputObserver);
    getEventManager().removeObserver(back);
    getEventManager().removeObserver(target);
    getEventManager().removeObserver(restart);
    for (const auto& control : {back, target, restart}) if (control) control->release();
    if (progress) progress->release();
    for (const auto& label : {title.get(), instructions.get(), scoreLabel.get(), timeLabel.get(), reticle.get()})
        if (label) label->release();
    inputObserver.reset();
    back.reset(); target.reset(); restart.reset();
    progress.reset(); title.reset(); instructions.reset(); scoreLabel.reset(); timeLabel.reset(); reticle.reset();
    loaded = false;
}

} // namespace moteur

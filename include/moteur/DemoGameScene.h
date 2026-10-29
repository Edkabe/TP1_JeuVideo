#pragma once
#include "moteur/IScene.h"
#include "moteur/SceneManager.h"
#include "moteur/Button.h"
#include "moteur/ProgressBar.h"
#include <filesystem>
#include <random>

namespace moteur {
// Mini-jeu chronométré dont la visée utilise les paramètres de sensibilité et de FOV.
class DemoGameScene final : public IScene {
public:
    DemoGameScene(SceneManager& scenes,std::filesystem::path fontFile,std::string returnScene);
    bool load() override;
    void unload() override;
    void update(float deltaTime) override;
    void render() override;
    bool isLoaded() const { return loaded; }
    const Button* getReturnButton() const { return back.get(); }
    const Button* getRestartButton() const { return restart.get(); }
    const std::string& getError() const { return error; }
    int getScore() const { return score; }
    float getTimeRemaining() const { return timeRemaining; }
    Vector2 getTargetPosition() const { return targetPosition; }
    Vector2 getTargetDimensions() const { return target ? target->getDimensions() : Vector2{}; }
    Vector2 getReticlePosition() const { return reticlePosition; }
private:
    void layout(int width,int height);
    void startGame();
    void moveTarget();
    void handleGameInput(const Event& event);
    bool reticleHitsTarget() const;
    SceneManager& scenes;
    std::filesystem::path fontFile;
    std::string returnScene;
    std::unique_ptr<Label> title,instructions,scoreLabel,timeLabel,reticle;
    std::unique_ptr<ProgressBar> progress;
    std::shared_ptr<Button> back,target,restart;
    std::shared_ptr<EventObserver> inputObserver;
    Vector2 targetPosition{},reticlePosition{};
    std::mt19937 random{std::random_device{}()};
    float sensitivity = 1.0f;
    float fieldOfView = 90.0f;
    float timeRemaining = 30.0f;
    int score = 0;
    bool gameOver = false;
    bool loaded = false;
    std::string error;
};
}

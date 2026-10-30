#include "moteur/SceneManager.h"
#include <utility>

namespace moteur {

SceneManager::~SceneManager() { clear(); }

bool SceneManager::addScene(std::string name, std::unique_ptr<IScene> scene) {
    error.clear();
    if (name.empty()) {
        error = "Le nom d'une scene ne peut pas etre vide.";
        return false;
    }
    if (!scene) {
        error = "Impossible d'ajouter une scene nulle.";
        return false;
    }
    if (scenes.find(name) != scenes.end()) {
        error = "Une scene porte deja ce nom : " + name;
        return false;
    }
    scenes.emplace(std::move(name), std::move(scene));
    return true;
}

bool SceneManager::removeScene(const std::string& name) {
    error.clear();
    const auto found = scenes.find(name);
    if (found == scenes.end()) {
        error = "Scene introuvable : " + name;
        return false;
    }
    if (activeName == name) {
        found->second->unload();
        activeName.clear();
    }
    scenes.erase(found);
    return true;
}

bool SceneManager::setActiveScene(const std::string& name) {
    error.clear();
    const auto target = scenes.find(name);
    if (target == scenes.end()) {
        error = "Impossible d'activer une scene absente : " + name;
        return false;
    }
    if (activeName == name) return true;

    // Charger la destination avant de quitter la scène courante évite de se
    // retrouver sans scène active si son chargement échoue.
    if (!target->second->load()) {
        error = target->second->getError();
        if (error.empty()) error = "Le chargement de la scene a echoue : " + name;
        target->second->unload(); // Nettoie les ressources partiellement chargées.
        return false;
    }

    if (!activeName.empty()) {
        const auto current = scenes.find(activeName);
        if (current != scenes.end()) current->second->unload();
    }
    activeName = name;
    return true;
}

void SceneManager::clear() {
    if (!activeName.empty()) {
        const auto current = scenes.find(activeName);
        if (current != scenes.end()) current->second->unload();
        activeName.clear();
    }
    scenes.clear();
    error.clear();
}

IScene* SceneManager::getActiveScene() {
    const auto found = scenes.find(activeName);
    return found == scenes.end() ? nullptr : found->second.get();
}

const IScene* SceneManager::getActiveScene() const {
    const auto found = scenes.find(activeName);
    return found == scenes.end() ? nullptr : found->second.get();
}

const std::string& SceneManager::getActiveSceneError() const {
    static const std::string empty;
    const IScene* scene = getActiveScene();
    return scene ? scene->getError() : empty;
}

bool SceneManager::hasScene(const std::string& name) const {
    return scenes.find(name) != scenes.end();
}

void SceneManager::dispatchEvent(const Event& event) {
    if (IScene* scene = getActiveScene()) scene->dispatchEvent(event);
}

void SceneManager::update(float deltaTime) {
    if (IScene* scene = getActiveScene()) scene->update(deltaTime);
}

void SceneManager::render() {
    if (IScene* scene = getActiveScene()) scene->render();
}

}

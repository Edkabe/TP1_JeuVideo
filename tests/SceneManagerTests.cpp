#include "moteur/SceneManager.h"
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
int checks = 0;

void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}

struct SceneProbe {
    bool loadSucceeds = true;
    int loads = 0;
    int unloads = 0;
    int updates = 0;
    int renders = 0;
};

class TestScene final : public moteur::IScene {
public:
    explicit TestScene(std::shared_ptr<SceneProbe> state) : state(std::move(state)) {}
    bool load() override { ++state->loads; return state->loadSucceeds; }
    void unload() override { ++state->unloads; }
    void update(float) override { ++state->updates; }
    void render() override { ++state->renders; }
private:
    std::shared_ptr<SceneProbe> state;
};

class TestObserver final : public moteur::EventObserver {
public:
    void onEvent(const moteur::Event&) override { ++calls; }
    int calls = 0;
};
}

int main() {
    try {
        moteur::SceneManager manager;
        check(manager.getActiveScene() == nullptr, "Aucune scene active au depart");

        auto firstState = std::make_shared<SceneProbe>();
        auto first = std::make_unique<TestScene>(firstState);
        TestScene* firstPointer = first.get();
        check(manager.addScene("menu", std::move(first)), "Ajout d'une scene par nom");
        check(!manager.addScene("menu", std::make_unique<TestScene>(firstState)), "Nom duplique refuse");
        check(!manager.addScene("", std::make_unique<TestScene>(firstState)), "Nom vide refuse");
        check(!manager.addScene("nulle", nullptr), "Scene nulle refusee");
        check(manager.setActiveScene("menu"), "Activation de la premiere scene");
        check(manager.getActiveScene() == firstPointer && manager.getActiveSceneName() == "menu",
              "La scene active est accessible par nom");
        check(firstState->loads == 1 && firstState->unloads == 0, "Chargement a l'activation");

        auto observer = std::make_shared<TestObserver>();
        check(firstPointer->getEventManager().addObserver(observer, SDL_EVENT_KEY_DOWN),
              "Abonnement propre a la scene");
        SDL_Event keyEvent{};
        keyEvent.type = SDL_EVENT_KEY_DOWN;
        manager.dispatchEvent(moteur::Event(keyEvent));
        check(observer->calls == 1, "Evenement distribue par le gestionnaire actif");
        manager.update(0.016f);
        manager.render();
        check(firstState->updates == 1 && firstState->renders == 1, "Mise a jour et rendu de la scene active");

        check(!manager.setActiveScene("inconnue"), "Scene absente signalee");
        check(manager.getActiveScene() == firstPointer && firstState->unloads == 0,
              "Une scene absente ne desactive pas la scene courante");

        auto failedState = std::make_shared<SceneProbe>();
        failedState->loadSucceeds = false;
        check(manager.addScene("echouee", std::make_unique<TestScene>(failedState)), "Ajout de la scene de test");
        check(!manager.setActiveScene("echouee"), "Echec de chargement signale");
        check(manager.getActiveScene() == firstPointer && firstState->unloads == 0,
              "La scene courante reste active apres un echec");
        check(failedState->loads == 1 && failedState->unloads == 1,
              "Nettoyage de la destination partiellement chargee");

        auto secondState = std::make_shared<SceneProbe>();
        check(manager.addScene("jeu", std::make_unique<TestScene>(secondState)), "Ajout de la deuxieme scene");
        check(manager.setActiveScene("jeu"), "Transition vers la deuxieme scene");
        check(firstState->unloads == 1 && secondState->loads == 1 &&
              manager.getActiveSceneName() == "jeu", "Transition de chargement et dechargement");
        manager.dispatchEvent(moteur::Event(keyEvent));
        check(observer->calls == 1, "Les evenements restent propres a chaque scene");
        check(manager.removeScene("jeu"), "Retrait par nom de la scene active");
        check(secondState->unloads == 1 && manager.getActiveScene() == nullptr,
              "Le retrait de la scene active la decharge");
        check(!manager.removeScene("absente"), "Retrait d'une scene absente signale");

        auto finalState = std::make_shared<SceneProbe>();
        check(manager.addScene("fin", std::make_unique<TestScene>(finalState)), "Ajout avant clear");
        check(manager.setActiveScene("fin"), "Activation avant clear");
        manager.clear();
        check(finalState->unloads == 1 && manager.getActiveScene() == nullptr && !manager.hasScene("fin"),
              "clear decharge et retire toutes les scenes");

        std::cout << checks << " verifications reussies.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "ECHEC : " << error.what() << '\n';
        return 1;
    }
}

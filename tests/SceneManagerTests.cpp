#include "moteur/SceneManager.h"
#include "moteur/CheckBox.h"
#include "moteur/Selector.h"
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
    int events = 0;
    bool partialObserverRemoved = false;
};

class ProbeObserver final : public moteur::EventObserver {
public:
    explicit ProbeObserver(std::shared_ptr<SceneProbe> state) : state(std::move(state)) {}
    void onEvent(const moteur::Event&) override { ++state->events; }
private:
    std::shared_ptr<SceneProbe> state;
};

class TestScene final : public moteur::IScene {
public:
    explicit TestScene(std::shared_ptr<SceneProbe> state) : state(std::move(state)) {}
    bool load() override {
        ++state->loads;
        observer = std::make_shared<ProbeObserver>(state);
        if (!getEventManager().addObserver(observer, SDL_EVENT_KEY_DOWN)) {
            error = "Inscription de test impossible.";
            return false;
        }
        if (!state->loadSucceeds) error = "Diagnostic load conserve.";
        return state->loadSucceeds;
    }
    void unload() override {
        ++state->unloads;
        const bool removed = getEventManager().removeObserver(observer);
        if (!state->loadSucceeds) state->partialObserverRemoved = removed;
        observer.reset();
    }
    void update(float) override { ++state->updates; }
    void render() override { ++state->renders; }
    const std::string& getError() const override { return error; }
private:
    std::shared_ptr<SceneProbe> state;
    std::shared_ptr<ProbeObserver> observer;
    std::string error;
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

        // La case respecte le contrat d'observateur de la scène qui la possède.
        auto checkBox = std::make_shared<moteur::CheckBox>();
        checkBox->setPosition(10.0f, 20.0f);
        checkBox->setSize(24.0f);
        check(firstPointer->getEventManager().addObserver(checkBox, SDL_EVENT_MOUSE_BUTTON_DOWN),
              "Case inscrite comme observateur de sa scene");
        SDL_Event mouseEvent{};
        mouseEvent.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        mouseEvent.button.button = SDL_BUTTON_LEFT;
        mouseEvent.button.x = 18.0f;
        mouseEvent.button.y = 28.0f;
        manager.dispatchEvent(moteur::Event(mouseEvent));
        check(checkBox->isChecked(), "Clic gauche dans la case change son etat via la scene");
        mouseEvent.button.x = 80.0f;
        mouseEvent.button.y = 80.0f;
        manager.dispatchEvent(moteur::Event(mouseEvent));
        check(checkBox->isChecked(), "Clic hors de la case laisse son etat intact");
        mouseEvent.button.x = 18.0f;
        mouseEvent.button.y = 28.0f;
        mouseEvent.button.button = SDL_BUTTON_RIGHT;
        manager.dispatchEvent(moteur::Event(mouseEvent));
        check(checkBox->isChecked(), "Clic droit ne change pas la case");

        // Cliquer sur la valeur du selecteur navigue aussi, pas seulement sur les fleches.
        moteur::Selector selector({}, {"Français", "English"});
        selector.setPosition({100.0f, 100.0f});
        selector.setDimensions({300.0f, 48.0f});
        SDL_Event selectorClick{};
        selectorClick.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        selectorClick.button.button = SDL_BUTTON_LEFT;
        selectorClick.button.x = 280.0f;
        selectorClick.button.y = 124.0f;
        selector.onEvent(moteur::Event(selectorClick));
        selectorClick.type = SDL_EVENT_MOUSE_BUTTON_UP;
        selector.onEvent(moteur::Event(selectorClick));
        check(selector.getSelectedIndex() == 1, "Clic sur la valeur passe a l'option suivante");
        selectorClick.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        selectorClick.button.x = 200.0f;
        selector.onEvent(moteur::Event(selectorClick));
        selectorClick.type = SDL_EVENT_MOUSE_BUTTON_UP;
        selector.onEvent(moteur::Event(selectorClick));
        check(selector.getSelectedIndex() == 0, "Clic sur l'autre moitie passe a l'option precedente");

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
        check(manager.getError() == "Diagnostic load conserve.", "Le diagnostic du load remonte au gestionnaire");
        check(manager.getActiveScene() == firstPointer && firstState->unloads == 0,
              "La scene courante reste active apres un echec");
        check(failedState->loads == 1 && failedState->unloads == 1,
              "Nettoyage de la destination partiellement chargee");
        manager.dispatchEvent(moteur::Event(keyEvent));
        manager.update(0.016f);
        manager.render();
        check(failedState->events == 0 && failedState->updates == 0 && failedState->renders == 0 &&
              firstState->events == 2 && firstState->updates == 2 && firstState->renders == 2,
              "Apres un load rate, seuls les evenements, update et render de l'ancienne scene active sont executes");
        check(failedState->partialObserverRemoved,
              "Un observateur partiellement inscrit est retire apres un echec de load");

        auto secondState = std::make_shared<SceneProbe>();
        check(manager.addScene("jeu", std::make_unique<TestScene>(secondState)), "Ajout de la deuxieme scene");
        check(manager.setActiveScene("jeu"), "Transition vers la deuxieme scene");
        check(firstState->unloads == 1 && secondState->loads == 1 &&
              manager.getActiveSceneName() == "jeu", "Transition de chargement et dechargement");
        manager.dispatchEvent(moteur::Event(keyEvent));
        manager.update(0.016f);
        manager.render();
        check(firstState->events == 2 && firstState->updates == 2 && firstState->renders == 2 &&
              secondState->events == 1 && secondState->updates == 1 && secondState->renders == 1,
              "Apres une transition reussie, seule la nouvelle scene active est notifiee et executee");
        check(observer->calls == 2, "L'observateur de l'ancienne scene n'est notifie que tant qu'elle reste active");
        mouseEvent.button.button = SDL_BUTTON_LEFT;
        manager.dispatchEvent(moteur::Event(mouseEvent));
        check(checkBox->isChecked(), "La case de l'ancienne scene ne recoit plus les evenements");
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

#include "moteur/Settings.h"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {
int checks = 0;
void check(bool value, const char* message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}
}

int main() {
    const auto directory = std::filesystem::temp_directory_path() / "tp1-settings-tests";
    const auto file = directory / "settings.bin";
    std::error_code ec;
    std::filesystem::remove_all(directory, ec);
    try {
        {
            // Première instance: sans fichier, elle expose et écrit les défauts.
            moteur::Settings initial(file);
            check(std::filesystem::exists(file), "Le fichier de defauts est cree a l'instanciation");
            check(initial.getLanguage() == "fr" && initial.getWidth() == 960 && initial.getHeight() == 540,
                  "Langue et resolution par defaut");
            check(!initial.isFullscreen() && initial.isVSyncEnabled(), "Plein ecran et VSync par defaut");
            check(initial.getSensitivity() == 1.0f && initial.getFov() == 90.0f, "Sensibilite et FOV par defaut");

            check(initial.setLanguage("en"), "Modification de la langue");
            check(initial.setResolution(1920, 1080), "Modification de la resolution");
            initial.setFullscreen(true);
            initial.setVSync(false);
            check(initial.setSensitivity(2.25f) && initial.setFov(105.0f), "Modification sensibilite et FOV");
            check(!initial.setResolution(0, 1080) && initial.getWidth() == 1920, "Resolution invalide refusee");
            check(!initial.setSensitivity(std::nanf("")) && initial.getSensitivity() == 2.25f,
                  "Sensibilite non finie refusee");
            check(!initial.setFov(180.0f) && initial.getFov() == 105.0f, "FOV invalide refuse");
            check(initial.save(), "Sauvegarde explicite");
        }
        {
            // Nouvelle instance pour prouver que les valeurs viennent bien du disque.
            moteur::Settings reloaded(file);
            check(reloaded.getLanguage() == "en" && reloaded.getWidth() == 1920 && reloaded.getHeight() == 1080,
                  "Langue et resolution rechargees apres redemarrage");
            check(reloaded.isFullscreen() && !reloaded.isVSyncEnabled(), "Booleens recharges apres redemarrage");
            check(reloaded.getSensitivity() == 2.25f && reloaded.getFov() == 105.0f,
                  "Sensibilite et FOV recharges apres redemarrage");
        }
        {
            // Une signature corrompue fait revenir à un état sûr sans exposer de valeurs partielles.
            std::filesystem::resize_file(file, 1);
            moteur::Settings recovered(file);
            check(recovered.getLanguage() == "fr" && recovered.getWidth() == 960 && recovered.getFov() == 90.0f,
                  "Valeurs par defaut si fichier invalide");
        }
        std::filesystem::remove_all(directory, ec);
        std::cout << checks << " verifications reussies.\n";
        return 0;
    } catch (const std::exception& error) {
        std::filesystem::remove_all(directory, ec);
        std::cerr << "ECHEC : " << error.what() << '\n';
        return 1;
    }
}

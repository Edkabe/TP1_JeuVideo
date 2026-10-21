#pragma once

#include <filesystem>
#include <string>
#include <cstdint>

namespace moteur {

/// @brief Paramètres de l'application, persistés dans un petit fichier binaire.
class Settings {
public:
    /// @brief Charge le fichier à la construction; crée les valeurs par défaut s'il manque.
    explicit Settings(std::filesystem::path file = "settings.bin");

    const std::string& getLanguage() const { return language; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isFullscreen() const { return fullscreen; }
    bool isVSyncEnabled() const { return vsync; }
    float getSensitivity() const { return sensitivity; }
    float getFov() const { return fov; }
    std::uint32_t getProgramBinaryFormat() const { return programBinaryFormat; }
    void setProgramBinaryFormat(std::uint32_t format) { programBinaryFormat = format; }
    const std::filesystem::path& getFilePath() const { return filePath; }
    const std::string& getError() const { return error; }

    // Les setters refusent une valeur invalide et laissent alors l'ancienne intacte.
    bool setLanguage(const std::string& value);
    bool setResolution(int newWidth, int newHeight);
    void setFullscreen(bool value) { fullscreen = value; }
    void setVSync(bool value) { vsync = value; }
    bool setSensitivity(float value);
    bool setFov(float value);

    /// @brief Relit le fichier; une lecture invalide rétablit les valeurs par défaut.
    bool load();
    /// @brief Écrit tous les champs dans le format binaire versionné.
    bool save();
    void resetToDefaults();

private:
    std::filesystem::path filePath;
    std::string language;
    int width = 0;
    int height = 0;
    bool fullscreen = false;
    bool vsync = false;
    float sensitivity = 0.0f;
    float fov = 0.0f;
    std::uint32_t programBinaryFormat = 0; // 0 : aucun format encore memorise.
    std::string error;
};

}

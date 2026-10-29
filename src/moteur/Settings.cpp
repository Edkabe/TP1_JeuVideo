#include "moteur/Settings.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <utility>

namespace moteur {
namespace {
// La signature et la version permettent de détecter un mauvais fichier et
// de faire évoluer le format sans dépendre du remplissage mémoire d'une struct.
constexpr std::array<char, 8> signature{{'T','P','1','S','E','T','0','1'}};
constexpr std::uint32_t formatVersion = 2;
constexpr std::uint32_t maxLanguageLength = 16;

bool writeU32(std::ostream& out, std::uint32_t value) {
    for (unsigned shift = 0; shift < 32; shift += 8)
        out.put(static_cast<char>((value >> shift) & 0xff));
    return static_cast<bool>(out);
}
bool readU32(std::istream& in, std::uint32_t& value) {
    value = 0;
    for (unsigned shift = 0; shift < 32; shift += 8) {
        const int byte = in.get();
        if (byte == std::char_traits<char>::eof()) return false;
        value |= static_cast<std::uint32_t>(static_cast<unsigned char>(byte)) << shift;
    }
    return true;
}
bool writeFloat(std::ostream& out, float value) {
    static_assert(sizeof(float) == sizeof(std::uint32_t), "Le format requiert un float 32 bits.");
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return writeU32(out, bits);
}
bool readFloat(std::istream& in, float& value) {
    std::uint32_t bits = 0;
    if (!readU32(in, bits)) return false;
    std::memcpy(&value, &bits, sizeof(value));
    return true;
}
bool validLanguage(const std::string& value) {
    return !value.empty() && value.size() <= maxLanguageLength &&
           value.find('\0') == std::string::npos;
}
bool validResolution(int width, int height) {
    return width >= 320 && width <= 16384 && height >= 540 && height <= 16384;
}
bool validSensitivity(float value) {
    return std::isfinite(value) && value >= 0.01f && value <= 100.0f;
}
bool validFov(float value) {
    return std::isfinite(value) && value >= 30.0f && value <= 120.0f;
}
}

Settings::Settings(std::filesystem::path file) : filePath(std::move(file)) {
    resetToDefaults();
    // Un fichier absent signifie une première exécution: conserver les défauts
    // en mémoire et les écrire immédiatement pour les prochains lancements.
    std::error_code ec;
    const bool exists = std::filesystem::exists(filePath, ec);
    if (ec) {
        error = "Impossible de vérifier le fichier de paramètres : " + ec.message();
    } else if (!exists) {
        save();
    } else {
        load();
    }
}

void Settings::resetToDefaults() {
    language = "fr";
    width = 960;
    height = 540;
    fullscreen = false;
    vsync = true;
    sensitivity = 1.0f;
    fov = 90.0f;
    programBinaryFormat = 0;
}

bool Settings::setLanguage(const std::string& value) {
    if (!validLanguage(value)) return false;
    language = value;
    return true;
}
bool Settings::setResolution(int newWidth, int newHeight) {
    if (!validResolution(newWidth, newHeight)) return false;
    width = newWidth;
    height = newHeight;
    return true;
}
bool Settings::setSensitivity(float value) {
    if (!validSensitivity(value)) return false;
    sensitivity = value;
    return true;
}
bool Settings::setFov(float value) {
    if (!validFov(value)) return false;
    fov = value;
    return true;
}

bool Settings::load() {
    error.clear();
    std::ifstream in(filePath, std::ios::binary);
    if (!in) { error = "Impossible d'ouvrir le fichier de paramètres."; return false; }

    std::array<char, signature.size()> fileSignature{};
    in.read(fileSignature.data(), static_cast<std::streamsize>(fileSignature.size()));
    std::uint32_t version = 0, languageLength = 0, rawWidth = 0, rawHeight = 0;
    std::uint32_t rawFullscreen = 0, rawVsync = 0;
    std::string loadedLanguage;
    float loadedSensitivity = 0.0f, loadedFov = 0.0f;
    std::uint32_t loadedBinaryFormat = 0;
    const bool headerValid = in && fileSignature == signature && readU32(in, version) && (version == 1 || version == formatVersion);
    bool valid = headerValid && readU32(in, languageLength) && languageLength > 0 &&
        languageLength <= maxLanguageLength;
    if (valid) {
        loadedLanguage.resize(languageLength);
        in.read(loadedLanguage.data(), static_cast<std::streamsize>(languageLength));
        valid = static_cast<bool>(in) && readU32(in, rawWidth) && readU32(in, rawHeight) &&
            readU32(in, rawFullscreen) && readU32(in, rawVsync) &&
            readFloat(in, loadedSensitivity) && readFloat(in, loadedFov);
        if (valid && version >= 2) valid = readU32(in, loadedBinaryFormat);
    }
    // Une seule représentation canonique des booléens et aucune donnée
    // supplémentaire après le dernier champ simplifient la validation.
    if (valid) valid = in.peek() == std::char_traits<char>::eof() &&
        rawFullscreen <= 1 && rawVsync <= 1 && validLanguage(loadedLanguage) &&
        validResolution(static_cast<std::int32_t>(rawWidth), static_cast<std::int32_t>(rawHeight)) &&
        validSensitivity(loadedSensitivity) && validFov(loadedFov);
    if (!valid) {
        resetToDefaults();
        error = "Fichier de paramètres invalide ou incompatible; valeurs par défaut utilisées.";
        return false;
    }
    language = std::move(loadedLanguage);
    width = static_cast<std::int32_t>(rawWidth);
    height = static_cast<std::int32_t>(rawHeight);
    fullscreen = rawFullscreen != 0;
    vsync = rawVsync != 0;
    sensitivity = loadedSensitivity;
    fov = loadedFov;
    programBinaryFormat = loadedBinaryFormat;
    return true;
}

bool Settings::save() {
    error.clear();
    if (!validLanguage(language) || !validResolution(width, height) ||
        !validSensitivity(sensitivity) || !validFov(fov)) {
        error = "Les paramètres en mémoire contiennent une valeur invalide.";
        return false;
    }
    std::error_code ec;
    const auto parent = filePath.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, ec);
        if (ec) { error = "Impossible de créer le dossier des paramètres : " + ec.message(); return false; }
    }
    std::ofstream out(filePath, std::ios::binary | std::ios::trunc);
    if (!out) {
        error = "Impossible d'ouvrir le fichier de paramètres en écriture.";
        return false;
    }
    out.write(signature.data(), static_cast<std::streamsize>(signature.size()));
    const bool written = out && writeU32(out, formatVersion) &&
        writeU32(out, static_cast<std::uint32_t>(language.size()));
    if (written) out.write(language.data(), static_cast<std::streamsize>(language.size()));
    const bool fieldsWritten = written && out &&
        writeU32(out, static_cast<std::uint32_t>(width)) &&
        writeU32(out, static_cast<std::uint32_t>(height)) &&
        writeU32(out, fullscreen ? 1u : 0u) && writeU32(out, vsync ? 1u : 0u) &&
        writeFloat(out, sensitivity) && writeFloat(out, fov) && writeU32(out, programBinaryFormat);
    out.flush();
    if (!fieldsWritten || !out) { error = "Écriture du fichier de paramètres échouée."; return false; }
    return true;
}

}

#pragma once
#include "moteur/Program.h"
#include "moteur/Settings.h"
#include <filesystem>

namespace moteur {
class ProgramCache {
public:
    struct Source { GLenum type; std::filesystem::path file; };
    // Contexte OpenGL courant requis. Un cache absent/invalide n'est pas fatal.
    bool load(Program& program, const std::vector<Source>& sources,
              const std::filesystem::path& cacheFile, Settings& settings);
    bool wasLoadedFromCache() const { return cached; }
    const std::string& getError() const { return error; }
    // Cache/parametres non sauvegardes : le programme reste utilisable.
    const std::string& getWarning() const { return warning; }
private:
    bool cached = false;
    std::string error, warning;
};
}

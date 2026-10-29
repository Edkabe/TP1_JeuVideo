#pragma once

#include <string>

namespace moteur {

inline bool isEnglish(const std::string& language) {
    return language.size() >= 2 && language[0] == 'e' && language[1] == 'n';
}

inline const char* localized(const std::string& language,
                             const char* french, const char* english) {
    return isEnglish(language) ? english : french;
}

} // namespace moteur

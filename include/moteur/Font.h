#pragma once
#include "moteur/Resource.h"
#include <SDL3_ttf/SDL_ttf.h>

namespace moteur {
class Font final : public Resource {
public:
    explicit Font(float pointSize = 24.0f, std::string title = {})
        : Resource(std::move(title)), pointSize(pointSize) {}
    ~Font() override;
    bool load(const std::string& path) override;
    void release() override;
    float getPointSize() const { return pointSize; }
    int getHeight() const;
    explicit operator TTF_Font*() const { return handle; }
private:
    TTF_Font* handle = nullptr;
    float pointSize;
    bool ttfInitialized = false;
};
}

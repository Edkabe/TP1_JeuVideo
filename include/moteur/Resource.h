#pragma once
#include <string>
#include <utility>

namespace moteur {
class Resource {
public:
    virtual ~Resource() = default;
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    virtual bool load(const std::string& path) = 0;
    virtual void release() = 0;
    bool isLoaded() const { return loaded; }
    const std::string& getTitle() const { return title; }
    const std::string& getError() const { return error; }
protected:
    explicit Resource(std::string title = {}) : title(std::move(title)) {}
    bool loaded = false;
    std::string error;
private:
    friend class ResourceManager;
    std::string title;
};
}

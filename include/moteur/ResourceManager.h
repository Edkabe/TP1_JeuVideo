#pragma once
#include "moteur/Resource.h"
#include <memory>
#include <unordered_map>
#include <type_traits>

namespace moteur {
class ResourceManager {
public:
    bool add(const std::string& name, std::shared_ptr<Resource> resource);
    bool add(std::shared_ptr<Resource> resource);
    template<class T = Resource>
    std::shared_ptr<T> get(const std::string& name) const {
        static_assert(std::is_base_of_v<Resource, T>, "T doit heriter de Resource");
        const auto found = resources.find(name);
        return found == resources.end() ? nullptr : std::dynamic_pointer_cast<T>(found->second);
    }
    bool contains(const std::string& name) const;
    bool remove(const std::string& name);
    void clear();
    std::size_t size() const { return resources.size(); }
private:
    std::unordered_map<std::string, std::shared_ptr<Resource>> resources;
};
}

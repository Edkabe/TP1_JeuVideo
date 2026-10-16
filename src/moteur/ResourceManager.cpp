#include "moteur/ResourceManager.h"
#include <utility>
namespace moteur {
    bool ResourceManager::add(const std::string& name, std::shared_ptr<Resource> resource) {
        if (name.empty() || !resource) return false;
        const auto inserted = resources.emplace(name, resource).second;
        // Le premier nom devient le titre si aucun titre n'a été fourni.
        // Les alias existants restent possibles et ne changent pas ce titre.
        if (inserted && resource->title.empty()) resource->title = name;
        return inserted;
    }
    bool ResourceManager::add(std::shared_ptr<Resource> resource) {
        if (!resource) return false;
        const auto title = resource->getTitle();
        return add(title, std::move(resource));
    }
    bool ResourceManager::contains(const std::string& name) const { return resources.find(name) != resources.end(); }
    bool ResourceManager::remove(const std::string& name) { return resources.erase(name) != 0; }
    void ResourceManager::clear() { resources.clear(); }
}

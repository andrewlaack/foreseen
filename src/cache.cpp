#include "../include/cache.hpp"

#include <cassert>
#include <mutex>
#include <optional>

#include "../include/shared.hpp"
#include "../include/site.hpp"

std::optional<Site> Cache::getSite(const std::string& site) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = cache.find(site);

    if (it != cache.end()) {
        return it->second;
    }

    return std::nullopt;
}

void Cache::addSite(std::string address, Site site) {
    std::lock_guard<std::mutex> lock(mutex);
    assert(address.find("gemini://") != std::string::npos);

    auto it = cache.find(address);
    if (it != cache.end()) {
        it->second = std::move(site);
        return;
    }
    cache.emplace(address, std::move(site));

    evictionQueue.push_front(address);
    if ((int)evictionQueue.size() > CACHE_SIZE) {
        cache.erase(evictionQueue.back());
        evictionQueue.pop_back();
    }
}

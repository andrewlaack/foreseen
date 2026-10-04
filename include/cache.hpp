// cache for prior sites and such.
#include <deque>
#include <mutex>
#include <optional>
#include <unordered_map>

#include "site.hpp"

class Cache {
   private:
    void evict();
    std::unordered_map<std::string, Site> cache;
    std::mutex mutex;
    std::deque<std::string> evictionQueue;

   public:
    std::optional<Site> getSite(const std::string& site);
    void addSite(std::string address, Site site);
};

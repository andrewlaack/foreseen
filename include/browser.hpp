#pragma once

#include <atomic>
#include <cstddef>
#include <thread>
#include <unordered_map>
#include <vector>
#include "line.hpp"
#include "identity-manager.hpp"
#include "site.hpp"
#include "cache.hpp"
#include "link.hpp"
#include "utils.hpp"

enum GoToSiteResult {
    SITE_LOADED,
    SITE_LOAD_FAILED,
    FETCHING_SITE_ASYNC
};


class Browser {
    private:
        IdentityManager identityManager = IdentityManager{};
        std::vector<std::thread> threads;
        std::vector<std::atomic<bool>> done;
        std::unordered_map<std::string, int> previousStatusCodes;
        std::vector<Link*> siteHistory;
        std::thread openThread;
        int previousIdx = -1;
        Cache* visitedCache;
        Cache* preFetchCache;
        Site* currentSite;
        std::vector<Line*> lines;
        std::vector<std::size_t> links; // these point to line indices
        void tryCacheTargets();
        Site* findInCacheAndPromoteIfRelevant(std::string& urlString);
    public:
        Browser();
        void setLinksOfCurrentLines();
        ~Browser();

        // if background is set and the site is found in cache it will go there right away, returning successfully.
        GoToSiteResult goToSite(std::string url, bool addToHistory = true, bool refresh = false, bool background = false);
        void setDone(int threadIdx);
        void refresh();
        Site* getCurrentSite();
        std::string tryDownloadPage(std::string destinationDir = "") noexcept;
        Link* getCurrentLink();
        std::optional<uri> getPriorUri();
        std::vector<std::pair<std::string, TextRender>> renderSite();
        std::vector<Line*> toLines(Site* site);
        bool followLinkNumber(int linkToFollow);
        std::vector<Link>* getLinkLines();
        Identity getIdentity(uri uriInput);
        void justCacheSite(Link link);
        void goBack();
        void goForward();
};

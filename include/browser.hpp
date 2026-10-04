#pragma once

#include <atomic>
#include <cstddef>
#include <thread>
#include <unordered_map>
#include <vector>

#include "cache.hpp"
#include "identity-manager.hpp"
#include "line.hpp"
#include "link.hpp"
#include "site.hpp"
#include "utils.hpp"

const std::string OPENED_EXT_TXT = "Externally opened";

enum SiteLoadResult {
    SITE_LOADED,  // These are valid status codes so 1X, 2X, and 3e
    SITE_TEMPORARY_FAILURE,
    SITE_PERMANENT_FAILURE,
    SITE_REQUIRES_CERTIFICATE,
    SITE_REJECTED_CERTIFICATE,
    SITE_INVALID_CERTIFICATE,
    SITE_UNEXPECTED_STATUS_CODE,
    LINK_DOES_NOT_EXIST  // only for link following
};

struct SiteLoadPair {  // TODO: Set this up.
    SiteLoadResult result;
    std::string metaLine;
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
    std::vector<std::size_t> links;  // these point to line indices
    void tryCacheTargets();
    Site* findInCacheAndPromoteIfRelevant(std::string& urlString);

   public:
    Browser();
    void setLinksOfCurrentLines();
    ~Browser();
    SiteLoadPair goToSite(std::string url, bool addToHistory = true,
                          bool refresh = false);
    void setDone(int threadIdx);
    void refresh();
    Site* getCurrentSite();
    std::string tryDownloadPage(std::string destinationDir = "") noexcept;
    Link* getCurrentLink();
    std::optional<uri> getPriorUri();
    std::vector<std::pair<std::string, TextRender>> renderSite();
    std::vector<Line*> toLines(Site* site);
    SiteLoadPair followLinkNumber(int linkToFollow);
    std::vector<Link>* getLinkLines();
    Identity getIdentity(uri uriInput);
    void justCacheSite(Link link);
    void goBack();
    void goForward();
};

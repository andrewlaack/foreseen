#pragma once

#include "link.hpp"
#include "site.hpp"

class GeminiClient {
    Site* getNetworkedSite(Link link, std::string crtPath, std::string keyPath);

   public:
    Site* fetchSite(Link link, std::string crtPath = "",
                    std::string keyPath = "");
};

#pragma once

#include <string>
#include <unordered_map>

#include "../vendor/uri.hpp"

struct Identity {
    std::string keyPath = "";
    std::string crtPath = "";
};

class IdentityManager {
   public:
    Identity getIdentityForURI(uri destination);
};

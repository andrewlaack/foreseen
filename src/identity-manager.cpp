#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include "../vendor/uri.hpp"
#include "../include/identity-manager.hpp"
#include "../include/utils.hpp"

Identity IdentityManager::getIdentityForURI(uri destination) {

    if(destination.get_scheme() != "gemini") {
        return Identity{"", ""};
    }

    namespace fs = std::filesystem;
    fs::path certDir = fs::path(getHome()) / ".gemi";
    std::string host = destination.get_host();
    std::string name = host;
    if (destination.get_port() && destination.get_port() != 1965) {
        name += ":" + std::to_string(destination.get_port());
    }
    fs::path dir = certDir / name;

    try {
        if (fs::exists(dir / "client.crt") && fs::exists(dir / "client.key")) {
            return Identity{(dir / "client.key").string(), (dir / "client.crt").string()};
        }
    }
    catch (const fs::filesystem_error& e){
        // this can happen if the domain is stupidly long.
        // in such cases, there are bigger issues.
    }
    return Identity{"", ""};
}

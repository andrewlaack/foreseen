#include "../include/browser.hpp"
#include <cassert>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h>
#include "../include/site.hpp"
#include "../include/errors.hpp"
#include "../include/gemini-client.hpp"
#include "../include/utils.hpp"
#include "../include/identity-manager.hpp"
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <utility>
#include <vector>


std::string Browser::tryDownloadPage(std::string downloadDir) noexcept {
    std::string body = currentSite->getBody();
    Link* current = getCurrentLink();
    assert(current != nullptr); // calling download page should always happen from a page...

    std::string destination = encodeAsFilename(current->getLinkDestination());

    try {
        if(downloadDir != "") {
            std::filesystem::path pth = std::filesystem::path(downloadDir);
            std::filesystem::create_directories(pth);
            destination = pth / destination;
        }
        writeStringToFile(body, destination);
    } catch (...) {
        return "";
    }

    return destination;
}

void dispatch(std::vector<Link>* targets, Browser& b, int threadIdx) {
    std::vector<Link>& refT = *targets;
    for(int i =  0 ; i < (int)refT.size() && i < SITE_CACHE_LIMIT; ++i) {
        auto& target = refT[i];

        auto id = b.getIdentity(target.getLinkDestination());
        if(id.crtPath != "" || id.keyPath != "") {
            continue; // don't try to prefetch for domains we normally pass a cert to.
        }
        b.justCacheSite(target);
    }
    b.setDone(threadIdx);
    delete targets;
}

void Browser::setDone(int threadIdx) {
    done[threadIdx] = true;
}

void Browser::tryCacheTargets() {
    bool dispatched = false;
    for(int i = 0; i < THREAD_NUM && dispatched == false; ++i) {
        if(done[i]) {
            if(threads[i].joinable()) {
                threads[i].join();
            }
            auto* lls = getLinkLines();
            done[i] = false;
            threads[i] = std::thread(dispatch, lls, std::ref(*this), i);
            dispatched = true;
        }
    }

}

Site* Browser::findInCacheAndPromoteIfRelevant(std::string& urlString) {
        Site*  site = nullptr;
        std::optional<Site> cachedSite = visitedCache->getSite(urlString);
        if(cachedSite != std::nullopt) {
            site = new Site(*cachedSite);
        }
        if(site == nullptr) {
            std::optional<Site> cached = preFetchCache->getSite(urlString);
            if(cached != std::nullopt) {
                site = new Site(*cached);
                visitedCache->addSite(urlString, *cached);
            }
        }

        return site;
}

void Browser::refresh() {
    goToSite(getPriorUri().value().to_string(), false, true);
}

bool Browser::goToSite(std::string url, bool addToHistory, bool refresh) {

    Link* prior = nullptr;

    if((int)siteHistory.size() > previousIdx && previousIdx >= 0) {
        prior = siteHistory[previousIdx];
    }

    auto client = GeminiClient{};

    Link* destination = nullptr;

    if(prior == nullptr) {
        destination = new Link{url};
    } else {
        destination = new Link{url,prior->getLinkDestination()};
    }

    std::string urlString = destination->getLinkDestination().to_string();
    std::string scheme  = destination->getLinkDestination().get_scheme();
    if(scheme != "gemini" && scheme != "file" && scheme != "about") { //  TODO: Should  I use about or just a fs file?
        if(openThread.joinable()) {
            openThread.join();
        }

        // this doesn't have to be blocking...
        // my browser hangs very often so yea.
        openThread = std::thread(openUrl,  urlString);

        delete destination;
        return true;
    }


    Site* site = nullptr;

    Identity id = identityManager.getIdentityForURI(destination->getLinkDestination());

    if(urlString.find("gemini://") != std::string::npos && !refresh) {
        if(id.crtPath != "" && id.keyPath != "") {
            if(!addToHistory) {
                site = findInCacheAndPromoteIfRelevant(urlString);
            }
        } else {
            site = findInCacheAndPromoteIfRelevant(urlString);
        }
    }
    
    if(site == nullptr) {
        site = client.fetchSite(*destination, id.crtPath, id.keyPath);
    }

    if(site == nullptr || site->getUnreachable()) {
        if(site != nullptr) {
            delete site;
        }
        delete destination;
        return false;
    }

    int sc = site->getStatusCode();
    if (sc < 10 || sc >= 40) {
        delete site;
        delete destination;
        return false;
    }

    if(addToHistory) {
        while((int)siteHistory.size() > previousIdx + 1) {
            delete siteHistory[siteHistory.size() -  1];
            siteHistory.pop_back();
        }
        siteHistory.push_back(destination);
        previousIdx = siteHistory.size() - 1;
    }

    if(currentSite != nullptr) {
        delete currentSite;
    }
    currentSite = site;

    if(urlString.find("gemini://") != std::string::npos) {
        int sc = site->getStatusCode();
        if(sc >= 20 && sc <= 29) {
            visitedCache->addSite(urlString, *site);
        }
    }

    for(auto* line: lines) {
        delete line;
    }

    lines = toLines(site);

    setLinksOfCurrentLines();
    previousStatusCodes[destination->getLinkDestination().to_string()] = site->getStatusCode();
    if (!addToHistory) {
        delete destination;
    }

    tryCacheTargets();
    return true;
}

Identity Browser::getIdentity(uri uriInput) {
    return this->identityManager.getIdentityForURI(uriInput);
}


void Browser::justCacheSite(Link link) {
    auto client = GeminiClient{};

    std::string urlString = link.getLinkDestination().to_string();

    if (visitedCache->getSite(urlString) || preFetchCache->getSite(urlString)) {
        return;
    }

    Site* site = nullptr;

    if(urlString.find("gemini://") != std::string::npos) {
        site = client.fetchSite(link);
    } 

    if(site != nullptr) {
        int sc = site->getStatusCode();
        if(sc >= 20 && sc <= 29) {
            preFetchCache->addSite(urlString, *site);
        }
        delete site;
    }

    return;
}


void Browser::setLinksOfCurrentLines() {
    links = std::vector<std::size_t> {};
    for(std::size_t i = 0; i < lines.size(); ++i) {
        if(lines[i]->type() == LINK) {
            links.push_back(i);
        }
    }
}


// TODO: This is a pure function.
std::vector<Line*> Browser::toLines(Site* site) {

    auto lines = stringToList(site->getBody());

    std::vector<Line*> res{};

    int lc = 1;

    bool isPreformatted = false;
    for(auto line: lines) {
        res.push_back(lineToLine(line, getPriorUri(), lc, isPreformatted));
        if (res[res.size()-1]->type() == FORMAT_SWITCH) {
            isPreformatted = !isPreformatted;
        }
        if(res[res.size() - 1]->type() == LINK) {
            lc += 1;
        }
    }
    return res;
}


Browser::Browser() : threads(THREAD_NUM), done(THREAD_NUM){
    currentSite = nullptr;
    visitedCache = new Cache{};
    preFetchCache = new Cache{};
    for (auto& d : done) {
        d = true;
    }

    // this ensures some nice invariants about the browser, like always having at least one valid page.
    bool start = goToSite("about://newtab");
    if(!start) {
        throw std::runtime_error("Browser unexpectedly failed to start.");
    }
}

// TODO: SHould add more stuff here too, like the links stuff.
Browser::~Browser() {

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    if (openThread.joinable()) {
        openThread.join();
    }

    delete visitedCache;
    delete preFetchCache;
    delete currentSite;

    for (auto* line : lines) {
        delete line;
    }

    for (auto* link : siteHistory) {
        delete link;
    }
}


Site* Browser::getCurrentSite() {
    return currentSite;
}

std::vector<std::pair<std::string, TextRender>> Browser::renderSite() {

    std::vector<std::pair<std::string, TextRender>> res{};
    res.reserve(lines.size());

    for(auto* line: lines) {
        // TODO: Don't special case this; define an interface.
        if(line->type() == PREFORMATTED) {
            std::pair<std::string,TextRender> cp {line->textToDraw(), TextRender {line->getColor(), line->isBold(), false}};
            res.push_back(cp);
        } else {
            std::pair<std::string,TextRender> cp {line->textToDraw(), TextRender {line->getColor(), line->isBold()}};
            res.push_back(cp);
        }
    }
    return res;
}


std::optional<uri> Browser::getPriorUri() {
    if(previousIdx < (int)siteHistory.size() && previousIdx >= 0) {
        return siteHistory[previousIdx]->getLinkDestination();
    }
    return std::nullopt;

}

std::vector<Link>* Browser::getLinkLines() {
    std::vector<Link>* res = new std::vector<Link> {};
    for(auto& ln : links) {
        res->push_back(*dynamic_cast<Link*>(lines[ln]));
    }
    return res;
}


bool Browser::followLinkNumber(int linkToFollow) {
    if((int)links.size() > linkToFollow-1 && linkToFollow-1 >= 0) {
        std::size_t pos = links[linkToFollow-1];
        if(lines.size() > pos) {
            Line* ptr = lines[pos];
            Link* ptrLnk = dynamic_cast<Link*>(ptr);
            bool res = goToSite(ptrLnk->getLinkDestination().to_string(), true);
            return res;
        }
    }
    return false;
}

void Browser::goBack() {

    int original = previousIdx;

    // we track this because sometimes sites do this:
        // start site
        // input something
        // redirect back to start site
    // and in such cases, I'd expect back and forward to treat the same site as one site, 
    // but only in cases where they are right next to each other without any other 2X status code sites
    // between them. 

    std::string starting = getCurrentLink()->getLinkDestination().to_string();

    previousIdx -= 1;

    if((int)siteHistory.size() > previousIdx && previousIdx >= 0) {

        int prSC = previousStatusCodes[siteHistory[previousIdx]->getLinkDestination().to_string()];
        std::string current = siteHistory[previousIdx]->getLinkDestination().to_string();
        while(!(prSC >= 20 && prSC <= 29) || starting == current) {
            previousIdx -= 1;
            if((int)siteHistory.size() > previousIdx && previousIdx >= 0) {
                prSC = previousStatusCodes[siteHistory[previousIdx]->getLinkDestination().to_string()];
                current = siteHistory[previousIdx]->getLinkDestination().to_string();
            } else {
                previousIdx = original;
                return; // safely fail with rollback
            }
        }
        if(!goToSite(this->siteHistory[previousIdx]->getLinkDestination().to_string(), false)) {
            previousIdx = original;
        }

    } else {
        previousIdx = original;
    }
}

void Browser::goForward() {

    int original = previousIdx;

    std::string starting = getCurrentLink()->getLinkDestination().to_string();

    previousIdx += 1;

    if((int)siteHistory.size() > previousIdx && previousIdx >= 0) {

        int prSC = previousStatusCodes[siteHistory[previousIdx]->getLinkDestination().to_string()];
        std::string current = siteHistory[previousIdx]->getLinkDestination().to_string();

        while(!(prSC >= 20 && prSC <= 29) || starting == current) {
            previousIdx += 1;
            if((int)siteHistory.size() > previousIdx && previousIdx >= 0) {
                prSC = previousStatusCodes[siteHistory[previousIdx]->getLinkDestination().to_string()];
                current = siteHistory[previousIdx]->getLinkDestination().to_string();
            } else {
                previousIdx = original; // fail safely
                return;
            }
        }

        if(!goToSite(this->siteHistory[previousIdx]->getLinkDestination().to_string(), false)) {
            previousIdx = original;
        }
    } else {
        previousIdx = original;
    }
}

Link* Browser::getCurrentLink() {
    if(previousIdx >= 0 && previousIdx < (int)siteHistory.size()) {
        return this->siteHistory[previousIdx];
    }
    return nullptr;
}


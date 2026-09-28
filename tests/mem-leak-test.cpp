#include "../include/browser.hpp"
#include <cstdlib>
#include <thread>
#include <vector>


void traversal() {
    Browser b{};
    b.goToSite("gemini://laack.co", true);

    for(int i = 0; i < 100; ++i) {
        auto* lls = b.getLinkLines();
        std::size_t count = lls->size();
        delete lls;
        if(count == 0) {
            b.goBack();
        } else if(rand() % 5 == 0) {
            b.goForward();
        } else if(rand() % 5 == 0) {
            b.goBack();
        } else {
            b.followLinkNumber(rand() % count + 1);
        }
    }

}

// NOTE: When running this you probably want to run something like:
// `watch pkill {browser}`
// because it'll probably try to follow http links that are found. 

int main() {
    std::vector<std::thread*> vec {};
    for(int x = 0; x < 10; ++x) {
        auto* t = new std::thread (&traversal);
        vec.push_back(t);
    }
    for(auto& th : vec) {
        th->join();
        delete th;
    }
}

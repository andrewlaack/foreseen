#include "../include/browser.hpp"
#include <cstdlib>
#include <thread>
#include <vector>
#include <ctime>
#include <unistd.h>

std::string gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}

void traversal() {
    Browser b{};
    b.goToSite("gemini://laack.co", true);

    for(int i = 0; i < 1000; ++i) {
        auto* lls = b.getLinkLines();
        std::size_t count = lls->size();
        delete lls;
        if(count == 0 && rand() % 5 == 0) {
            b.goBack();
        } else if(rand() % 5 == 0) {
            b.goForward();
        } else if(rand() % 5 == 0) {
            b.goBack();
        } else if (rand() % 5 == 0){
            b.goToSite(std::string{"gemini://"} + gen_random(rand() % 400) + ".com");
        }
        else {
            if(count == 0) {
                count = 10;
            }
            b.followLinkNumber(((rand() % count) + 1) + (rand() % 5));
        }
    }
}

// NOTE: When running this you probably want to run something like:
// `watch pkill {browser}`
// because it'll probably try to follow http links that are found. 

int main() {
    std::vector<std::thread*> vec {};
    for(int x = 0; x < 100; ++x) {
        auto* t = new std::thread (&traversal);
        vec.push_back(t);
    }
    for(auto& th : vec) {
        th->join();
        delete th;
    }
}

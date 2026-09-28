#include "../include/browser.hpp"
#include <cstdlib>
#include <iostream>
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

    for(int i = 0; i < 10000; ++i) {

        auto* lls = b.getLinkLines();
        std::size_t count = lls->size();
        delete lls;

        if(b.getCurrentLink() != nullptr) {
            std::cout << "CURRENTLY  AT: " << b.getCurrentLink()->getLinkDestination().to_string() << std::endl;
        }

        if(count == 0 && rand() % 5 == 0) {
            std::cout << "GOING BACK" << std::endl;
            b.goBack();
        } else if(rand() % 5 == 0) {
            std::cout << "GOING FORWARD" << std::endl;
            b.goForward();
        } else if(rand() % 5 == 0) {
            std::cout << "GOING BACK" << std::endl;
            b.goBack();
        } else if (rand() % 5 == 0){
            if(rand() % 5 == 0) {
                std::string dst = std::string{"gemini://"} + gen_random(rand() % 5000) + ".com";
                std::cout << "TRAVELLING TO: " << dst << std::endl;
                b.goToSite(dst);
            } else {
                std::string dst = std::string{"gemini://"} + gen_random(rand() % 15) + ".com";
                std::cout << "TRAVELLING TO: " << dst << std::endl;
                b.goToSite(dst);
            }

        } else if (rand() % 10 == 0) {
            Destination destination = handleDestinationResolution(gen_random(rand() % 50), false); 
            if(rand() % 2 == 0) {
                destination = handleDestinationResolution(gen_random(rand() % 5), false);
            }
            switch (destination.t) {
                case NO_DESTINATION:
                    break;
                case NUMBER_DESTINATION:
                    std::cout << "LINK NUMBER: " << destination.linkNumber << std::endl;
                    b.followLinkNumber(destination.linkNumber);
                    break;
                case STRING_DESTINATION:
                    std::cout << "DESTINATION: " << destination.destination << std::endl;
                    b.goToSite(destination.destination);
                    break;
            }
        }
        else {
            if(count == 0) {
                count = 10;
            }
            int num = ((rand() % count) + 1) + (rand() % 5);
            std::cout << "LINK NUMBER(2): " << num << std::endl;
            b.followLinkNumber(num);
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

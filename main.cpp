#include <iostream>
#include "Downloader.h"

int main() {
    //initialize the downloader
    Downloader downloader("https://cominsyslab.com/link.txt");

    try {
        //start
        downloader.start();
    } catch (std::string &error) {
        std::cout << "\aError: " << error << std::endl;
    }

    return 0;
}

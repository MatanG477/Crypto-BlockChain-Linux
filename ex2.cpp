#include <iostream>
#include <string>
#include "infra.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage:\n"
                  << "  " << argv[0] << " --height <height>\n"
                  << "  " << argv[0] << " --hash <hash>\n";
        return 1;
    }

    std::string option = argv[1];
    load_db();

    if (option == "--height") {
        try {
            int height = std::stoi(argv[2]);
            find_block_by_height(height);
        } catch (const std::exception& e) {
            std::cerr << "Invalid height value.\n";
            return 1;
        }
    } else if (option == "--hash") {
        std::string hash = argv[2];
        find_block_by_hash(hash.c_str());
    } else {
        std::cout << "Unknown option: " << option << "\n";
        std::cout << "Usage:\n"
                  << "  " << argv[0] << " --height <height>\n"
                  << "  " << argv[0] << " --hash <hash>\n";
        return 1;
    }

    return 0;
}
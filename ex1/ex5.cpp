#include <iostream>
#include <string>
#include "infra.h"

int main() {
    int choice;
    std::string input_hash;
    int input_height;

    // Load the blockchain data once at the start
    load_db();

    while (true) {
        std::cout << "\nChoose an option:\n";
        std::cout << "1. print db\n";
        std::cout << "2. Print block by hash\n";
        std::cout << "3. Print block by height\n";
        std::cout << "4. Export data to csv\n";
        std::cout << "5. Refresh data\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        std::cin.ignore(); // Clear newline from buffer

        switch (choice) {
            case 1:
                print_db();
                break;
            case 2:
                std::cout << "Enter block hash: ";
                std::getline(std::cin, input_hash);
                find_block_by_hash(input_hash.c_str());
                break;
            case 3:
                std::cout << "Enter block height: ";
                std::cin >> input_height;
                std::cin.ignore(); // Clear newline
                find_block_by_height(input_height);
                break;
            case 4:
                export_to_csv();
                break;
            case 5:
                refresh_data();
                // After refreshing, reload the blockchain data
                load_db();
                break;
            case 0:
                std::cout << "Goodbye!" << std::endl;
                return 0;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
}

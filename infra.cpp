#include <iostream>
#include "infra.h"
#include <fstream>    
#include <string>     
#include <vector>
#include <algorithm>
#include <cstdlib>

std::vector<Block> blockchain;

int countBlocks(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    int count = 0;

    if (!file.is_open()) {
        std::cerr << "Failed to open " << filename << "\n";
        return 0;
    }

    while (std::getline(file, line)) {
        // Trim leading spaces (optional)
        size_t pos = line.find("\"hash\":");
        if (pos != std::string::npos) {
            ++count;
        }
    }

    file.close();
    return count;
}

std::string cleanLine(const std::string& line) {
    // Find the colon and take the value part
    size_t colonPos = line.find(":");
    if (colonPos == std::string::npos) {
        return "";  // Return empty if colon is not found
    }

    std::string value = line.substr(colonPos + 1);

    // Remove leading spaces and trim
    value.erase(remove(value.begin(), value.end(), ' '), value.end());
    value.erase(remove(value.begin(), value.end(), '"'), value.end());
    value.erase(remove(value.begin(), value.end(), ','), value.end());

    return value;
}

// Loads all blocks from info.txt into the global blockchain vector.
// Assumes each block is represented by 6 or 7 lines (if 'received_time' exists).
void load_db() {
    // Open the info.txt file for reading
    std::ifstream file("info.txt");
    std::string line;
    std::vector<std::string> blockLines;
    blockchain.clear(); // Clear previous data

    while (std::getline(file, line)) {
        if (line.empty()) continue; // Skip empty lines
        blockLines.push_back(line);

        if (blockLines.size() == 6) {
            Block b;
            try {
                b.hash = cleanLine(blockLines[0]);
                b.height = std::stoi(cleanLine(blockLines[1]));
                b.total = std::stoll(cleanLine(blockLines[2]));
                b.time = cleanLine(blockLines[3]);
                b.relayed_by = cleanLine(blockLines[4]);
                b.prev_block = cleanLine(blockLines[5]);
            } catch (...) {
                // Skip block if there is a parsing error
                blockLines.clear();
                continue;
            }
            blockchain.push_back(b); // Add the block to the vector
            blockLines.clear();
        }
    }
    file.close();
}

// Prints all blocks in the blockchain in the required format (no quotes around values).
// Prints an arrow between blocks for visual separation.
void print_db() {
    if (blockchain.empty()) {
        std::cout << "Blockchain is empty. Please run load_db() first." << std::endl;
        return;
    }
    for (size_t i = 0; i < blockchain.size(); ++i) {
        const auto& b = blockchain[i];
        // Print all block fields, one per line
        std::cout << "hash: " << b.hash << std::endl;
        std::cout << "height: " << b.height << std::endl;
        std::cout << "total: " << b.total << std::endl;
        std::cout << "time: " << b.time << std::endl;
        std::cout << "relayed_by: " << b.relayed_by << std::endl;
        std::cout << "prev_block: " << b.prev_block << std::endl;
        // Print arrow only if not the last block
        if (i != blockchain.size() - 1) {
            std::cout << "    |\n    v\n" << std::endl;
        }
    }
}

// Finds and prints a block by its hash value.
// Assumes blockchain is already loaded into memory.
void find_block_by_hash(const char* hash) {
    for (const auto& b : blockchain) {
        if (b.hash == hash) {
            std::cout << "hash: " << b.hash << std::endl;
            std::cout << "height: " << b.height << std::endl;
            std::cout << "total: " << b.total << std::endl;
            std::cout << "time: " << b.time << std::endl;
            std::cout << "relayed_by: " << b.relayed_by << std::endl;
            std::cout << "prev_block: " << b.prev_block << std::endl;
            return;
        }
    }
    std::cout << "Block with hash '" << hash << "' not found." << std::endl;
}

// Finds and prints a block by its height value.
// Assumes blockchain is already loaded into memory.
void find_block_by_height(int height) {
    for (const auto& b : blockchain) {
        if (b.height == height) {
            std::cout << "hash: " << b.hash << std::endl;
            std::cout << "height: " << b.height << std::endl;
            std::cout << "total: " << b.total << std::endl;
            std::cout << "time: " << b.time << std::endl;
            std::cout << "relayed_by: " << b.relayed_by << std::endl;
            std::cout << "prev_block: " << b.prev_block << std::endl;
            return;
        }
    }
    std::cout << "Block with height '" << height << "' not found." << std::endl;
}

// Exports all blocks to a CSV file named infoutput.csv.
// Assumes blockchain is already loaded into memory.
void export_to_csv() {
    std::ofstream output("infoutput.csv");
    if (!output.is_open()) {
        std::cerr << "Failed to create infoutput.csv" << std::endl;
        return;
    }
    // Write CSV header
    output << "hash,height,total,time,relayed_by,prev_block\n";
    for (const auto& b : blockchain) {
        output << b.hash << ","
               << b.height << ","
               << b.total << ","
               << b.time << ","
               << b.relayed_by << ","
               << b.prev_block << "\n";
    }
    output.close();
    std::cout << "Data exported to infoutput.csv successfully!" << std::endl;
}

void refresh_data() {
    int blockCount = blockchain.size(); // Count how many blocks were loaded

    std::cout << "Found " << blockCount << " blocks." << std::endl;

    // Prepare the command string with the block count as an argument
    std::string command = "./blockchain1.sh " + std::to_string(blockCount);

    // Run the script
    int result = system(command.c_str());

    if (result == 0) {
        std::cout << "Script executed successfully." << std::endl;
    } else {
        std::cout << "Script failed with code: " << result << std::endl;
    }
}
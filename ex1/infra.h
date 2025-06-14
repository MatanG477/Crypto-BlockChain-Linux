#ifndef INFRA_H
#define INFRA_H

#include <string>
#include <vector>
#include <cstdint>  // Add this at the top

struct Block {
    std::string hash;
    int height;
    int64_t total;  // Changed from int to int64_t
    std::string time;
    std::string relayed_by;
    std::string prev_block;
};

extern std::vector<Block> blockchain;

#ifdef __cplusplus
extern "C" {
#endif

std::string cleanLine(const std::string& line);
int countBlocks(const std::string& filename);
void load_db();  // Removed the argument x
void print_db();
void find_block_by_hash(const char* hash);
void find_block_by_height(int height);
void export_to_csv();
void refresh_data();

#ifdef __cplusplus
}
#endif

#endif // INFRA_H

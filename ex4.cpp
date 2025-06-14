#include <iostream>
#include <cstdlib>
#include "infra.h"

int main() {
    load_db(); // Load the blocks into the blockchain vector
    refresh_data();
    return 0;
}
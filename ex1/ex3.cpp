#include "infra.h"

int main() {
    load_db();           // Always load the database before exporting
    export_to_csv();     // Export all blocks to CSV file
    return 0;
}

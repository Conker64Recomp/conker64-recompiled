#include <iostream>
#include <cstdint>
#include <vector>

// Representación básica del espacio de memoria virtual de la N64 (8 MB RDRAM)
uint8_t rdram[8 * 1024 * 1024] = { 0 };

int main(int argc, char** argv) {
    std::cout << "========================================" << std::endl;
    std::cout << " Conker's Bad Fur Day: Recompiled (PC)" << std::endl;
    std::cout << " Version: Native Windows x64 Build" << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << "[Init] Allocating 8MB N64 RDRAM space... OK" << std::endl;
    std::cout << "[Init] Preparing Recomp execution loop..." << std::endl;

    return 0;
}

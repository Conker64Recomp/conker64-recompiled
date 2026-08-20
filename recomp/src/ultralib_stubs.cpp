#include <iostream>
#include <cstdint>

// Stubs iniciales para las funciones del sistema operativo de N64 (libultra)
// Estas funciones luego se conectarán a SDL2, XInput y RT64 para DirectX12/Vulkan.

extern "C" {

void osInitialize(void) {
    std::cout << "[N64 OS] osInitialize called" << std::endl;
}

void osViSetMode(void* mode) {
    (void)mode;
}

void osViBlack(uint8_t active) {
    (void)active;
}

void osViSwapBuffer(void* vaddr) {
    (void)vaddr;
}

}

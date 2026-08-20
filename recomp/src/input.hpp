#pragma once

#include <cstdint>
#include <SDL.h>
#include <iostream>
#include <cmath>

namespace N64 {

// Estructura oficial del mando de Nintendo 64 (libultra / OSContPad)
struct OSContPad {
    uint16_t button;    // Bitmask con los botones A, B, Z, Start, D-Pad, L, R, C-Buttons
    int8_t   stick_x;   // Rango: -128 a 127 (Rango util N64 aprox: -80 a 80)
    int8_t   stick_y;   // Rango: -128 a 127
    uint8_t  err_no;
};

// Definición de bits de botones de N64
namespace Buttons {
    constexpr uint16_t CONT_A      = 0x8000;
    constexpr uint16_t CONT_B      = 0x4000;
    constexpr uint16_t CONT_G      = 0x2000; // Gatillo Z
    constexpr uint16_t CONT_START  = 0x1000;
    constexpr uint16_t CONT_UP     = 0x0800;
    constexpr uint16_t CONT_DOWN   = 0x0400;
    constexpr uint16_t CONT_LEFT   = 0x0200;
    constexpr uint16_t CONT_RIGHT  = 0x0100;
    constexpr uint16_t CONT_L      = 0x0020;
    constexpr uint16_t CONT_R      = 0x0010;
    constexpr uint16_t CONT_E      = 0x0008; // C-Down
    constexpr uint16_t CONT_D      = 0x0004; // C-Up
    constexpr uint16_t CONT_C      = 0x0002; // C-Left
    constexpr uint16_t CONT_F      = 0x0001; // C-Right
}

class InputManager {
public:
    static InputManager& getInstance() {
        static InputManager instance;
        return instance;
    }

    void init() {
        // Buscar y abrir el primer mando conectado (Xbox / PS / Genérico)
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            if (SDL_IsGameController(i)) {
                controller = SDL_GameControllerOpen(i);
                if (controller) {
                    std::cout << "[Input] Gamepad detected: " << SDL_GameControllerName(controller) << std::endl;
                    break;
                }
            }
        }
        if (!controller) {
            std::cout << "[Input] No gamepad found. Keyboard mapping active (WASD / Arrows / Space / X / C)." << std::endl;
        }
    }

    void shutdown() {
        if (controller) {
            SDL_GameControllerClose(controller);
            controller = nullptr;
        }
    }

    // Actualizar estado del mando N64 combinando Teclado + Mando Físico
    void poll(OSContPad& pad) {
        pad.button = 0;
        pad.stick_x = 0;
        pad.stick_y = 0;
        pad.err_no = 0;

        // 1. Mapeo por Teclado (Fallback)
        const uint8_t* keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_X]) pad.button |= Buttons::CONT_A;
        if (keys[SDL_SCANCODE_C] || keys[SDL_SCANCODE_Z])     pad.button |= Buttons::CONT_B;
        if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_Q]) pad.button |= Buttons::CONT_G; // Z Trigger
        if (keys[SDL_SCANCODE_RETURN])                        pad.button |= Buttons::CONT_START;
        if (keys[SDL_SCANCODE_E])                             pad.button |= Buttons::CONT_R;
        if (keys[SDL_SCANCODE_R])                             pad.button |= Buttons::CONT_L;

        // D-Pad
        if (keys[SDL_SCANCODE_UP])    pad.button |= Buttons::CONT_UP;
        if (keys[SDL_SCANCODE_DOWN])  pad.button |= Buttons::CONT_DOWN;
        if (keys[SDL_SCANCODE_LEFT])  pad.button |= Buttons::CONT_LEFT;
        if (keys[SDL_SCANCODE_RIGHT]) pad.button |= Buttons::CONT_RIGHT;

        // C-Buttons (IJKL)
        if (keys[SDL_SCANCODE_I]) pad.button |= Buttons::CONT_D; // C-Up
        if (keys[SDL_SCANCODE_K]) pad.button |= Buttons::CONT_E; // C-Down
        if (keys[SDL_SCANCODE_J]) pad.button |= Buttons::CONT_C; // C-Left
        if (keys[SDL_SCANCODE_L]) pad.button |= Buttons::CONT_F; // C-Right

        // Analog Stick con WASD
        int sx = 0;
        int sy = 0;
        if (keys[SDL_SCANCODE_W]) sy += 80;
        if (keys[SDL_SCANCODE_S]) sy -= 80;
        if (keys[SDL_SCANCODE_A]) sx -= 80;
        if (keys[SDL_SCANCODE_D]) sx += 80;

        // 2. Mapeo por Gamepad de Xbox/PlayStation
        if (controller && SDL_GameControllerGetAttached(controller)) {
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) pad.button |= Buttons::CONT_A;
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X)) pad.button |= Buttons::CONT_B;
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START)) pad.button |= Buttons::CONT_START;
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) pad.button |= Buttons::CONT_R;
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) pad.button |= Buttons::CONT_L;

            // Gatillo Z asignado a Left Trigger o Right Trigger
            int16_t lt = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
            int16_t rt = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
            if (lt > 16000 || rt > 16000) pad.button |= Buttons::CONT_G;

            // D-Pad del mando
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP))    pad.button |= Buttons::CONT_UP;
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN))  pad.button |= Buttons::CONT_DOWN;
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT))  pad.button |= Buttons::CONT_LEFT;
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) pad.button |= Buttons::CONT_RIGHT;

            // Stick analógico izquierdo
            int16_t axisX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
            int16_t axisY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
            
            // Zona muerta
            if (std::abs(axisX) > 4000) sx = (axisX * 80) / 32767;
            if (std::abs(axisY) > 4000) sy = (-axisY * 80) / 32767; // Invertir eje Y para N64
        }

        // Clamp
        if (sx > 80) sx = 80;
        if (sx < -80) sx = -80;
        if (sy > 80) sy = 80;
        if (sy < -80) sy = -80;

        pad.stick_x = static_cast<int8_t>(sx);
        pad.stick_y = static_cast<int8_t>(sy);
    }

private:
    InputManager() : controller(nullptr) {}
    SDL_GameController* controller;
};

} // namespace N64

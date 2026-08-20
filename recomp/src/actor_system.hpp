#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include "input.hpp"
#include "save_system.hpp"

namespace N64 {

struct ActorState {
    float posX, posY, posZ;
    float velX, velY, velZ;
    float rotY, rotX;
    float animFrame;
    bool isGrounded;
    int health;
    int money;
};

class ActorManager {
public:
    static ActorManager& getInstance() {
        static ActorManager instance;
        return instance;
    }

    void init() {
        player.posX = 0.0f;
        player.posY = 0.0f;
        player.posZ = 0.0f;
        player.velX = 0.0f;
        player.velY = 0.0f;
        player.velZ = 0.0f;
        player.rotY = 0.0f;
        player.rotX = 0.0f;
        player.animFrame = 0.0f;
        player.isGrounded = true;
        player.health = 6; // 6 barras de chocolate
        player.money = 0;
        std::cout << "[ActorManager] Player Actor (Conker) initialized at (0, 0, 0)." << std::endl;
    }

    // Actualiza físicas, gravedad y movimiento del jugador a 60 FPS
    void updatePlayer(const OSContPad& pad, float dt) {
        // 1. Controles analógicos (WASD / Stick)
        float inputX = pad.stick_x / 80.0f;
        float inputY = pad.stick_y / 80.0f;

        // Zona muerta
        if (std::abs(inputX) < 0.15f) inputX = 0.0f;
        if (std::abs(inputY) < 0.15f) inputY = 0.0f;

        float moveSpeed = 3.5f;
        player.velX = inputX * moveSpeed;
        player.velZ = -inputY * moveSpeed;

        // Rotación del personaje hacia donde camina
        if (std::abs(inputX) > 0.1f || std::abs(inputY) > 0.1f) {
            player.rotY = std::atan2(inputX, inputY) * 180.0f / 3.14159265f;
            player.animFrame += dt * 12.0f; // Animación de trote
        } else {
            player.animFrame = 0.0f; // Idle
        }

        // 2. Salto con botón A (Espacio / Botón A del Mando)
        if ((pad.button & Buttons::CONT_A) && player.isGrounded) {
            player.velY = 5.5f;
            player.isGrounded = false;
        }

        // 3. Gravedad y suelo
        if (!player.isGrounded) {
            player.velY -= 12.0f * dt; // Gravedad
        }

        // Aplicar velocidades
        player.posX += player.velX * dt;
        player.posY += player.velY * dt;
        player.posZ += player.velZ * dt;

        // Colisión simple con el suelo
        if (player.posY <= 0.0f) {
            player.posY = 0.0f;
            player.velY = 0.0f;
            player.isGrounded = true;
        }
    }

    const ActorState& getPlayer() const { return player; }

private:
    ActorManager() = default;
    ActorState player{};
};

} // namespace N64

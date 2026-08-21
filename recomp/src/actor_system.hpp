#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <string>
#include "input.hpp"
#include "save_system.hpp"

namespace N64 {

enum class AnimState {
    IDLE = 0,
    WALK = 1,
    RUN = 2,
    JUMP = 3,
    FALL = 4,
    HOVER = 5,   // Helicopter tail float (Rareware classic)
    ATTACK = 6,  // Tail spin / Frying pan swing
    CROUCH = 7
};

// Collider Types
struct AABBCollider {
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;
    std::string name;

    bool contains(float x, float z) const {
        return (x >= minX && x <= maxX && z >= minZ && z <= maxZ);
    }
};

struct CylinderCollider {
    float posX, posZ;
    float radius;
    float minY, maxY;
    std::string name;

    bool checkCollision(float px, float pz, float pr, float& outPushX, float& outPushZ) const {
        float dx = px - posX;
        float dz = pz - posZ;
        float distSq = dx * dx + dz * dz;
        float minDist = radius + pr;
        if (distSq < minDist * minDist && distSq > 1e-6f) {
            float dist = std::sqrt(distSq);
            float overlap = minDist - dist;
            outPushX = (dx / dist) * overlap;
            outPushZ = (dz / dist) * overlap;
            return true;
        }
        return false;
    }
};

struct ActorState {
    static constexpr float kHoverDuration = 2.0f;

    // Kinematic coordinates
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    float velX = 0.0f, velY = 0.0f, velZ = 0.0f;
    float rotY = 0.0f, rotX = 0.0f;
    float speed = 0.0f;

    // Animation & Skeletal parameters
    AnimState animState = AnimState::IDLE;
    float animTime = 0.0f;
    float legAngle = 0.0f;       // Swings +/- during run
    float armAngle = 0.0f;       // Swings opposite to legs
    float tailAngle = 0.0f;      // Idle sway / jump arch
    float tailSpinAngle = 0.0f;  // Helicopter spin (360 deg)
    float bobY = 0.0f;           // Vertical running/breathing bounce
    float bodyLean = 0.0f;       // Forward running lean

    // Gameplay states
    bool isGrounded = true;
    bool isHovering = false;
    bool isAttacking = false;
    float attackTimer = 0.0f;
    float hoverTimeLeft = kHoverDuration;

    int health = 6;              // 6 chocolate bars
    int money = 0;               // Dollar wads
};

class ActorManager {
public:
    static ActorManager& getInstance() {
        static ActorManager instance;
        return instance;
    }

    void init() {
        player = ActorState{};
        player.health = 6;
        player.money = 0;
        prevJumpHeld = false;
        prevAttackHeld = false;
        initColliders();
        std::cout << "[ActorManager] 3D physics world initialized." << std::endl;
        std::cout << "[ActorManager] " << boxes.size() << " AABB platforms, "
                  << cylinders.size() << " cylinder obstacles." << std::endl;
    }

    void initColliders() {
        boxes.clear();
        cylinders.clear();

        boxes.push_back({ -1.4f, 1.4f,  0.0f, 0.25f,  3.6f, 6.4f, "Context Sensitive Pad" });
        boxes.push_back({ -7.25f, -3.75f, 0.0f, 1.0f, 2.25f, 5.75f, "Rock Platform (Left)" });
        boxes.push_back({  3.75f,  7.25f, 0.0f, 1.5f, 2.25f, 5.75f, "Stepped Platform (Right)" });

        cylinders.push_back({ -5.5f, 4.0f, 0.75f, 0.0f, 2.5f, "Beer Barrel" });
        cylinders.push_back({  6.0f, 8.0f, 0.45f, 0.0f, 4.0f, "Tree (Right)" });
        cylinders.push_back({ -6.0f, 8.0f, 0.45f, 0.0f, 4.0f, "Tree (Left)" });
    }

    // Altura del terreno bajo (x, z) para unos pies a `feetY`.
    //
    // Antes ignoraba la Y del jugador y devolvia el techo de CUALQUIER caja que
    // contuviera (x,z): acercarse al lateral de una plataforma teletransportaba
    // al jugador encima, y era imposible pasar por debajo de nada.
    float getGroundHeight(float x, float z, float feetY) const {
        float best = 0.0f; // plano del suelo
        for (const auto& box : boxes) {
            if (!box.contains(x, z)) continue;
            if (box.maxY > feetY + kStepHeight) continue; // es pared, no suelo
            if (box.maxY > best) best = box.maxY;
        }
        return best;
    }

    // Empuje horizontal fuera de las caras verticales de las plataformas.
    // Corregir un solo eje (el de menor penetracion) produce deslizamiento
    // natural a lo largo del muro.
    void resolveBoxCollisions(float& px, float py, float& pz,
                              float& velX, float& velZ, float radius) const {
        const float feet = py;
        const float head = py + kPlayerHeight;

        for (const auto& box : boxes) {
            if (box.maxY <= feet + kStepHeight) continue; // se puede subir encima
            if (box.minY >= head) continue;               // se pasa por debajo

            const float minX = box.minX - radius, maxX = box.maxX + radius;
            const float minZ = box.minZ - radius, maxZ = box.maxZ + radius;
            if (px <= minX || px >= maxX || pz <= minZ || pz >= maxZ) continue;

            const float dLeft  = px - minX;
            const float dRight = maxX - px;
            const float dBack  = pz - minZ;
            const float dFront = maxZ - pz;
            const float m = std::min({ dLeft, dRight, dBack, dFront });

            if (m == dLeft)       { px = minX; if (velX > 0.0f) velX = 0.0f; }
            else if (m == dRight) { px = maxX; if (velX < 0.0f) velX = 0.0f; }
            else if (m == dBack)  { pz = minZ; if (velZ > 0.0f) velZ = 0.0f; }
            else                  { pz = maxZ; if (velZ < 0.0f) velZ = 0.0f; }
        }
    }

    void resolveCylinderCollisions(float& px, float& pz,
                                   float& velX, float& velZ, float radius) const {
        for (const auto& cyl : cylinders) {
            float pushX = 0.0f, pushZ = 0.0f;
            if (!cyl.checkCollision(px, pz, radius, pushX, pushZ)) continue;

            px += pushX;
            pz += pushZ;

            // Anula la componente de velocidad que entra en el obstaculo y deja
            // intacta la tangencial, para que se deslice en vez de pegarse.
            float len = std::sqrt(pushX * pushX + pushZ * pushZ);
            if (len < 1e-6f) continue;
            float nx = pushX / len, nz = pushZ / len;
            float into = velX * nx + velZ * nz;
            if (into < 0.0f) {
                velX -= into * nx;
                velZ -= into * nz;
            }
        }
    }

    // Actualiza fisicas, velocidad, gravedad, colisiones y animaciones.
    //
    // `cameraYawDeg` orienta el input respecto a la camara: antes el stick
    // movia siempre en ejes de mundo, asi que al girar la camara con Q/E
    // "adelante" dejaba de ser adelante.
    void updatePlayer(const OSContPad& pad, float dt, float cameraYawDeg) {
        float inputX = pad.stick_x / 80.0f;
        float inputY = pad.stick_y / 80.0f;

        // Zona muerta radial (la anterior era por eje y deformaba las diagonales)
        float mag = std::sqrt(inputX * inputX + inputY * inputY);
        if (mag < kDeadzone) {
            inputX = inputY = 0.0f;
        } else {
            float scaled = std::min(1.0f, (mag - kDeadzone) / (1.0f - kDeadzone));
            inputX = (inputX / mag) * scaled;
            inputY = (inputY / mag) * scaled;
        }

        // Base de camara: adelante = (-sin, 0, cos), derecha = (cos, 0, sin)
        const float yawRad = cameraYawDeg * 3.14159265f / 180.0f;
        const float cosY = std::cos(yawRad), sinY = std::sin(yawRad);
        const float moveX = -sinY * inputY + cosY * inputX;
        const float moveZ =  cosY * inputY + sinY * inputX;

        // ── 1. ACELERACION HORIZONTAL ─────────────────────────────────────────
        const float maxRunSpeed = 5.5f;
        float targetVelX = moveX * maxRunSpeed;
        float targetVelZ = moveZ * maxRunSpeed;

        float accel = player.isGrounded ? 18.0f : 8.0f;
        player.velX += (targetVelX - player.velX) * accel * dt;
        player.velZ += (targetVelZ - player.velZ) * accel * dt;
        player.speed = std::sqrt(player.velX * player.velX + player.velZ * player.velZ);

        if (player.speed > 0.2f) {
            float targetRot = std::atan2(player.velX, player.velZ) * 180.0f / 3.14159265f;
            float diff = targetRot - player.rotY;
            while (diff < -180.0f) diff += 360.0f;
            while (diff >  180.0f) diff -= 360.0f;
            player.rotY += diff * std::min(1.0f, 15.0f * dt);
        }

        // ── 2. SALTO Y PLANEO CON COLA ────────────────────────────────────────
        const bool jumpHeld = (pad.button & Buttons::CONT_A) != 0;
        const bool jumpPressed = jumpHeld && !prevJumpHeld;  // flanco de subida

        // Sin deteccion de flanco, mantener A pulsado rebotaba en bucle al
        // tocar suelo cada frame.
        if (jumpPressed && player.isGrounded) {
            player.velY = 7.2f;
            player.isGrounded = false;
            player.isHovering = false;
            player.hoverTimeLeft = ActorState::kHoverDuration;
        }
        else if (jumpHeld && !player.isGrounded && !player.isHovering &&
                 player.velY < 1.0f && player.hoverTimeLeft > 0.1f) {
            player.isHovering = true;
        }
        else if (!jumpHeld && player.isHovering) {
            player.isHovering = false;
        }
        prevJumpHeld = jumpHeld;

        // ── 3. ATAQUE / ACCION DE CONTEXTO ───────────────────────────────────
        const bool attackHeld = (pad.button & Buttons::CONT_B) != 0;
        if (attackHeld && !prevAttackHeld && !player.isAttacking) {
            player.isAttacking = true;
            player.attackTimer = 0.35f;
        }
        prevAttackHeld = attackHeld;

        if (player.isAttacking) {
            player.attackTimer -= dt;
            if (player.attackTimer <= 0.0f) player.isAttacking = false;
        }

        // ── 4. GRAVEDAD Y CINEMATICA DE PLANEO ───────────────────────────────
        if (!player.isGrounded) {
            if (player.isHovering) {
                player.velY = -1.2f;
                player.hoverTimeLeft -= dt;
                if (player.hoverTimeLeft <= 0.0f) player.isHovering = false;
            } else {
                player.velY -= 17.5f * dt;
            }
        }

        // ── 5. INTEGRACION Y COLISIONES ──────────────────────────────────────
        player.posX += player.velX * dt;
        player.posZ += player.velZ * dt;
        player.posY += player.velY * dt;

        resolveCylinderCollisions(player.posX, player.posZ, player.velX, player.velZ, kPlayerRadius);
        resolveBoxCollisions(player.posX, player.posY, player.posZ,
                             player.velX, player.velZ, kPlayerRadius);

        player.posX = std::clamp(player.posX, -kWorldBound, kWorldBound);
        player.posZ = std::clamp(player.posZ, -kWorldBound, kWorldBound);

        float groundY = getGroundHeight(player.posX, player.posZ, player.posY);

        if (player.posY <= groundY) {
            player.posY = groundY;
            player.velY = 0.0f;
            player.isGrounded = true;
            player.isHovering = false;
            player.hoverTimeLeft = ActorState::kHoverDuration;
        } else if (player.posY > groundY + 0.1f) {
            player.isGrounded = false;
        }

        // ── 6. MAQUINA DE ESTADOS DE ANIMACION ───────────────────────────────
        player.animTime += dt;

        if (player.isAttacking) {
            player.animState = AnimState::ATTACK;
            player.tailSpinAngle += 1800.0f * dt;
            player.armAngle = 60.0f;
            player.legAngle = 0.0f;
            player.bobY = 0.0f;
            player.bodyLean = 0.0f;
        }
        else if (!player.isGrounded) {
            if (player.isHovering) {
                player.animState = AnimState::HOVER;
                player.tailSpinAngle += 1440.0f * dt;
                player.legAngle = 20.0f * std::sin(player.animTime * 6.0f);
                player.armAngle = -35.0f;
                player.bobY = 0.04f * std::sin(player.animTime * 12.0f);
                player.bodyLean = 10.0f;
            } else {
                player.animState = (player.velY > 0.0f) ? AnimState::JUMP : AnimState::FALL;
                player.tailAngle = (player.velY > 0.0f) ? 35.0f : -25.0f;
                player.legAngle = -20.0f;
                player.armAngle = 45.0f;
                player.bobY = 0.0f;
                player.bodyLean = 5.0f;
            }
        }
        else if (player.speed > 0.3f) {
            player.animState = (player.speed > 3.0f) ? AnimState::RUN : AnimState::WALK;
            float strideFreq = (player.animState == AnimState::RUN) ? 14.0f : 8.0f;
            float strideAmp  = (player.animState == AnimState::RUN) ? 38.0f : 22.0f;

            player.legAngle = std::sin(player.animTime * strideFreq) * strideAmp;
            player.armAngle = -player.legAngle * 0.9f;
            player.tailAngle = std::sin(player.animTime * strideFreq * 0.5f) * 20.0f;
            player.bobY = std::abs(std::sin(player.animTime * strideFreq)) * 0.09f;
            player.bodyLean = (player.animState == AnimState::RUN) ? 14.0f : 6.0f;
        }
        else {
            player.animState = AnimState::IDLE;
            player.legAngle = 0.0f;
            player.armAngle = 0.0f;
            player.tailAngle = std::sin(player.animTime * 2.2f) * 16.0f;
            player.bobY = std::sin(player.animTime * 3.0f) * 0.025f;
            player.bodyLean = 0.0f;
        }
    }

    const ActorState& getPlayer() const { return player; }

private:
    ActorManager() = default;

    static constexpr float kPlayerRadius = 0.42f;
    static constexpr float kPlayerHeight = 1.60f;
    static constexpr float kStepHeight   = 0.35f;
    static constexpr float kDeadzone     = 0.15f;
    static constexpr float kWorldBound   = 25.0f;

    ActorState player{};
    std::vector<AABBCollider> boxes;
    std::vector<CylinderCollider> cylinders;

    bool prevJumpHeld = false;
    bool prevAttackHeld = false;
};

} // namespace N64

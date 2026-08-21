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
    float hoverTimeLeft = 2.5f;   // Max hover duration

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
        initColliders();
        std::cout << "[ActorManager] 3D Physics World & Rareware Collision Engine initialized." << std::endl;
        std::cout << "[ActorManager] Registered " << boxes.size() << " AABB platforms, " 
                  << cylinders.size() << " cylinder obstacles." << std::endl;
    }

    void initColliders() {
        boxes.clear();
        cylinders.clear();

        // 1. Red B Context Sensitive Pad
        boxes.push_back({ -1.4f, 1.4f,  0.0f, 0.25f,  3.6f, 6.4f, "Context Sensitive Pad" });

        // 2. Elevated Rock/Wood Platform (Left)
        boxes.push_back({ -7.25f, -3.75f, 0.0f, 1.0f, 2.25f, 5.75f, "Rock Platform (Left)" });

        // 3. Stepped Platform (Right)
        boxes.push_back({  3.75f,  7.25f, 0.0f, 1.5f, 2.25f, 5.75f, "Stepped Platform (Right)" });

        // 4. Tavern Beer Barrel (Obstacle)
        cylinders.push_back({ -5.5f, 4.0f, 0.75f, 0.0f, 2.5f, "Beer Barrel" });

        // 5. Trees in environment
        cylinders.push_back({  6.0f, 8.0f, 0.45f, 0.0f, 4.0f, "Tree (Right)" });
        cylinders.push_back({ -6.0f, 8.0f, 0.45f, 0.0f, 4.0f, "Tree (Left)" });
    }

    // Resolves terrain height under point (x, z)
    float getGroundHeight(float x, float z) const {
        float maxH = 0.0f; // Default floor plane

        for (const auto& box : boxes) {
            if (box.contains(x, z)) {
                if (box.maxY > maxH) {
                    maxH = box.maxY;
                }
            }
        }
        return maxH;
    }

    // Resolves horizontal collisions with obstacles & walls (sliding vector projection)
    void resolveHorizontalCollisions(float& px, float& pz, float radius) {
        // Check cylinder obstacles (Barrels, Trees)
        for (const auto& cyl : cylinders) {
            float pushX = 0.0f, pushZ = 0.0f;
            if (cyl.checkCollision(px, pz, radius, pushX, pushZ)) {
                px += pushX;
                pz += pushZ;
            }
        }

        // World boundaries
        const float boundX = 25.0f;
        const float boundZ = 25.0f;
        px = std::clamp(px, -boundX, boundX);
        pz = std::clamp(pz, -boundZ, boundZ);
    }

    // Updates physics, velocity, gravity, collisions and animations at 60 FPS
    void updatePlayer(const OSContPad& pad, float dt) {
        float inputX = pad.stick_x / 80.0f;
        float inputY = pad.stick_y / 80.0f;

        // Deadzone
        if (std::abs(inputX) < 0.15f) inputX = 0.0f;
        if (std::abs(inputY) < 0.15f) inputY = 0.0f;

        float targetSpeed = std::sqrt(inputX * inputX + inputY * inputY);
        float maxRunSpeed = 5.5f;

        // ── 1. HORIZONTAL ACCELERATION & MOVEMENT ──────────────────────────────
        float targetVelX = inputX * maxRunSpeed;
        float targetVelZ = inputY * maxRunSpeed;

        float accel = player.isGrounded ? 18.0f : 8.0f;
        player.velX += (targetVelX - player.velX) * accel * dt;
        player.velZ += (targetVelZ - player.velZ) * accel * dt;
        player.speed = std::sqrt(player.velX * player.velX + player.velZ * player.velZ);

        // Rotation towards movement direction
        if (player.speed > 0.2f) {
            float targetRot = std::atan2(player.velX, player.velZ) * 180.0f / 3.14159265f;
            // Smooth angular interpolation
            float diff = targetRot - player.rotY;
            while (diff < -180.0f) diff += 360.0f;
            while (diff >  180.0f) diff -= 360.0f;
            player.rotY += diff * 15.0f * dt;
        }

        // ── 2. JUMP & HELICOPTER HOVER (Signature Rareware Move) ───────────────
        bool jumpPressed = (pad.button & Buttons::CONT_A) != 0;
        
        if (jumpPressed) {
            if (player.isGrounded) {
                // First Jump impulse
                player.velY = 7.2f;
                player.isGrounded = false;
                player.isHovering = false;
                player.hoverTimeLeft = 2.0f;
            } else if (!player.isHovering && player.velY < 1.0f && player.hoverTimeLeft > 0.1f) {
                // Activate Tail-Copter (Hover) in midair!
                player.isHovering = true;
            }
        } else {
            if (player.isHovering) {
                player.isHovering = false;
            }
        }

        // ── 3. ATTACK / CONTEXT ACTION (Button B / X) ──────────────────────────
        bool attackPressed = (pad.button & Buttons::CONT_B) != 0;
        if (attackPressed && !player.isAttacking) {
            player.isAttacking = true;
            player.attackTimer = 0.35f;
        }
        if (player.isAttacking) {
            player.attackTimer -= dt;
            if (player.attackTimer <= 0.0f) {
                player.isAttacking = false;
            }
        }

        // ── 4. GRAVITY & HOVER KINEMATICS ─────────────────────────────────────
        if (!player.isGrounded) {
            if (player.isHovering) {
                // Helicopter hover slows downward fall to gentle float
                player.velY = -1.2f;
                player.hoverTimeLeft -= dt;
                if (player.hoverTimeLeft <= 0.0f) {
                    player.isHovering = false;
                }
            } else {
                // Standard N64 gravity
                player.velY -= 17.5f * dt;
            }
        }

        // ── 5. INTEGRATE POSITION & RESOLVE COLLISIONS ─────────────────────────
        player.posX += player.velX * dt;
        player.posZ += player.velZ * dt;
        player.posY += player.velY * dt;

        // Resolve horizontal obstacles (push out of barrels/trees/walls)
        resolveHorizontalCollisions(player.posX, player.posZ, 0.42f);

        // Resolve ground height & step-up ledges
        float groundY = getGroundHeight(player.posX, player.posZ);

        if (player.posY <= groundY) {
            // Land on ground / platform
            player.posY = groundY;
            player.velY = 0.0f;
            player.isGrounded = true;
            player.isHovering = false;
            player.hoverTimeLeft = 2.0f;
        } else {
            // Still in air
            if (player.posY > groundY + 0.1f) {
                player.isGrounded = false;
            }
        }

        // ── 6. SKELETAL ANIMATION STATE MACHINE ────────────────────────────────
        player.animTime += dt;

        if (player.isAttacking) {
            player.animState = AnimState::ATTACK;
            player.tailSpinAngle += 1800.0f * dt; // Rapid 360 spin
            player.armAngle = 60.0f;
            player.legAngle = 0.0f;
            player.bobY = 0.0f;
            player.bodyLean = 0.0f;
        }
        else if (!player.isGrounded) {
            if (player.isHovering) {
                player.animState = AnimState::HOVER;
                player.tailSpinAngle += 1440.0f * dt; // 4 full spins per sec
                player.legAngle = 20.0f * std::sin(player.animTime * 6.0f);
                player.armAngle = -35.0f; // Arms spread for balance
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
            player.armAngle = -player.legAngle * 0.9f; // Opposing arm swing
            player.tailAngle = std::sin(player.animTime * strideFreq * 0.5f) * 20.0f;
            player.bobY = std::abs(std::sin(player.animTime * strideFreq)) * 0.09f;
            player.bodyLean = (player.animState == AnimState::RUN) ? 14.0f : 6.0f;
        }
        else {
            // IDLE Breathing & subtle tail swish
            player.animState = AnimState::IDLE;
            player.legAngle = 0.0f;
            player.armAngle = 0.0f;
            player.tailAngle = std::sin(player.animTime * 2.2f) * 16.0f;
            player.bobY = std::sin(player.animTime * 3.0f) * 0.025f; // Breathing
            player.bodyLean = 0.0f;
        }
    }

    const ActorState& getPlayer() const { return player; }

private:
    ActorManager() = default;
    ActorState player{};
    std::vector<AABBCollider> boxes;
    std::vector<CylinderCollider> cylinders;
};

} // namespace N64

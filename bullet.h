#pragma once
#include <SFML/Graphics.hpp>
#include "zombie.h"
#include <cmath>

class Bullet {
private:
    sf::CircleShape shape;
    Zombie* target;
    float speed;
    int damage;
    bool active;
    static constexpr float HIT_RANGE = 10.0f;

public:
    Bullet(sf::Vector2f startPosition, Zombie* target, float speed = 500.0f, int damage = 1)
        : target(target), speed(speed), damage(damage), active(true) {
        shape.setRadius(5);
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(startPosition);
    }

     void update(float deltaTime) {
        if (!active || !target || target->isDead()) 
            return;

        // Current positions
        sf::Vector2f curPos   = shape.getPosition();
        sf::Vector2f targetPos = target->getPosition();
        
        // Vector toward the target
        sf::Vector2f dir = targetPos - curPos;
        float dist = std::hypot(dir.x, dir.y);
        if (dist == 0.f) {
            // Already exactly on top: hit immediately
            target->takeDamage(damage);
            active = false;
            return;
        }
        
        // If we're close enough to hit in this step, register it now
        float moveStep = speed * deltaTime;
        if (dist <= moveStep || shape.getGlobalBounds().intersects(target->getBounds())) {
            target->takeDamage(damage);
            active = false;
            return;
        }

        // Otherwise, move toward the target
        dir /= dist;  // normalize
        shape.move(dir * moveStep);
    }


    void draw(sf::RenderWindow& window) const {
        if (active) window.draw(shape);
    }

    bool isActive() const { return active; }
};
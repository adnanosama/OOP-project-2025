#include <SFML/Graphics.hpp>
#include "zombie.h"
#include <cmath>
#include <iostream>

Zombie::Zombie(sf::Texture& texture, const std::vector<sf::Vector2f>& path)
    : path(path), speed(50.0f), health(10), currentTargetIndex(0) {
    sprite.setTexture(texture);
    sprite.setScale(1.0f, 1.0f);
    sprite.setPosition(path[0]);
}

void Zombie::takeDamage(int damage) {
        health -= damage;
        if (health <= 0) {
            health = 0;
        }
}

bool Zombie::reachedEnd() const {
    return currentTargetIndex >= path.size();
}

void Zombie::update(float deltaTime) {
    if (isDead() || reachedEnd()) {
        return;
    }
    sf::Vector2f position = sprite.getPosition();
    sf::Vector2f target = path[currentTargetIndex];
    sf::Vector2f direction = target - position;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (distance != 0) {
        direction /= distance;
    }
    sprite.move(direction * speed * deltaTime);
    if (distance < speed * deltaTime) {
        currentTargetIndex++;
    }
}

void Zombie::draw(sf::RenderWindow& window) {
    if (!isDead()) {
        window.draw(sprite);
    }
}

FastZombie::FastZombie(sf::Texture& texture, const std::vector<sf::Vector2f>& path)
    : Zombie(texture, path) {
        speed = 100.0f;
        health = 5;
        sprite.setTexture(texture);
        sprite.setScale(1.0f, 1.0f);
        sprite.setPosition(path[0]);
}

StrongZombie::StrongZombie(sf::Texture& texture, const std::vector<sf::Vector2f>& path)
    : Zombie(texture, path) {
    speed = 30.0f;
    health = 20;
    sprite.setTexture(texture);
    sprite.setScale(1.0f, 1.0f);
    sprite.setPosition(path[0]);
}
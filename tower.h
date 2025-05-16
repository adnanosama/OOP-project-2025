// tower.h
#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "bullet.h"
#include "zombie.h"

class Tower {
    sf::Vector2f pos;
    float cooldown;
    float range;
    float attackSpeed;
    sf::Sprite sprite;
    std::vector<Bullet> bullets;

public:
    Tower(sf::Vector2f position, const sf::Texture& tex, float range = 300.f, float atkSpd = 1.5f)
      : pos(position), cooldown(0), range(range), attackSpeed(atkSpd) {
        sprite.setTexture(tex);
        sprite.setOrigin(tex.getSize().x / 2.f, tex.getSize().y / 2.f);
        sprite.setPosition(pos);
        float sX = Map::tileSize / float(tex.getSize().x);
        float sY = Map::tileSize / float(tex.getSize().y);
        float u = std::min(sX, sY);
        sprite.setScale(u*1.5, u*1.5);
    }

    void update(float dt, const std::vector<Zombie*>& zombies, bool allowFire) {
        Zombie* target = nullptr;
        float minD = range;

        for (auto z : zombies) {
            if (z->isDead()) continue;
            float dx = pos.x - z->getPosition().x;
            float dy = pos.y - z->getPosition().y;
            float d = std::hypot(dx, dy);
            if (d < minD) {
                minD = d;
                target = z;
            }
        }

        
        if (allowFire && target && cooldown <= 0.f) {
        bullets.emplace_back(pos, target);
        cooldown = cooldown;
        }

        for (auto& b : bullets) b.update(dt);
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [](auto& b){ return !b.isActive(); }),
            bullets.end());
    }


    void draw(sf::RenderWindow& w) const {
        w.draw(sprite);
        for (auto& b : bullets) b.draw(w);
    }

    // Accessors needed by main.cpp
    sf::Vector2f getPosition() const { return pos; }
    float getRange() const       { return range; }
};
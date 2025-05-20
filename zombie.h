#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>

class Zombie {
    protected:
    sf::Sprite sprite;
    std::vector<sf::Vector2f> path;
    float speed;
    int health;
    int currentTargetIndex;
    sf::Sound* hitSound = nullptr;
    sf::Sound* dieSound = nullptr;

    public:
    Zombie(sf::Texture& texture, const std::vector<sf::Vector2f>& path, sf::Sound* hitSound = nullptr, sf::Sound* dieSound = nullptr);
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    sf::FloatRect getBounds() const { 
        sf::FloatRect b = sprite.getGlobalBounds();

        // compute 20% margins
        float dx = b.width  * 0.2f;
        float dy = b.height * 0.2f;

        // inset the rectangle
        b.left   += dx;
        b.top    += dy;
        b.width  -= 2*dx;
        b.height -= 2*dy;
        return b; 
    }
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    void takeDamage(int damage);
    bool isDead() const { return health <= 0; } 
    bool reachedEnd() const;
    void setSounds(sf::Sound* hit, sf::Sound* die) { hitSound = hit; dieSound = die; }
};

class FastZombie : public Zombie {
    public:
    FastZombie(sf::Texture& texture, const std::vector<sf::Vector2f>& path, sf::Sound* hitSound, sf::Sound* dieSound);
};

class StrongZombie : public Zombie {
    public:
    StrongZombie(sf::Texture& texture, const std::vector<sf::Vector2f>& path, sf::Sound* hitSound, sf::Sound* dieSound);
};
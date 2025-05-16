#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>
#include "wave.h"
#include <string>
using namespace std;

Waves::Waves(sf::Texture& regularTexture, sf::Texture& fastTexture, sf::Texture& strongTexture, const std::vector<sf::Vector2f>& path)
    : regularTexture(regularTexture), fastTexture(fastTexture), strongTexture(strongTexture), path(path) {
        waves = {
            {5,0,0},
            {6,1,0},
            {6,2,1},
            {6,3,2},
            {7,3,3}
        };
        if (!font.loadFromFile("fonts/BarlowCondensed-Regular.ttf")) {
        std::cerr << "Failed to load font\n";
        }
        gameMessage.setFont(font);
        gameMessage.setCharacterSize(60);
        gameMessage.setFillColor(sf::Color::White);
        gameMessage.setPosition(400, 450);
        messageBackground.setSize(sf::Vector2f(400, 100));
        messageBackground.setFillColor(sf::Color(0,0,0,180));
        messageBackground.setPosition(300, 450);
        messageBackground.setOutlineThickness(5);
        messageBackground.setOutlineColor(sf::Color::White);

        prepareNextWave();
}

void Waves::prepareNextWave() {
    if (currentWave >= waves.size()) return; 

    spawnQueue.clear();
    const auto& wave = waves[currentWave];
    for (int i = 0; i < wave.regularCount; ++i) {
        spawnQueue.push_back("regular");
    }
    for (int i = 0; i < wave.fastCount; ++i) {
        spawnQueue.push_back("fast");
    }
    for (int i = 0; i < wave.strongCount; ++i) {
        spawnQueue.push_back("strong");
    }
    zombiesSpawned = 0;
    spawnTimer = 0.0f;
}

void Waves::update(float deltaTime) {
    if (isFinished()) return;
    spawnTimer += deltaTime;

    if (zombiesSpawned < spawnQueue.size() && spawnTimer >= 1.0f) {
        const string& type = spawnQueue[zombiesSpawned];
        if (type == "regular") {
            zombies.emplace_back(make_unique<Zombie>(regularTexture, path));
        } else if (type == "fast") {
            zombies.emplace_back(make_unique<FastZombie>(fastTexture, path));
        } else if (type == "strong") {
            zombies.emplace_back(make_unique<StrongZombie>(strongTexture, path));
        }
        spawnTimer = 0.0f;
        zombiesSpawned++;
    }

    for (auto& zombie : zombies) {
        zombie->update(deltaTime);
    }

    zombies.erase(
        std::remove_if(zombies.begin(), zombies.end(),
        [](const std::unique_ptr<Zombie>& z) {
            return z->isDead();
        }),
    zombies.end());
    
    if (zombiesSpawned == spawnQueue.size() && zombies.empty() && !showPopup) {
        popupTimer = 2.0f;
        showPopup = true;

    if (currentWave >= waves.size() - 1) {
        gameMessage.setString("You won!");
    } else {
        gameMessage.setString("Wave " + std::to_string(currentWave + 1) + " complete!");
    }
    }   

    if (showPopup) {
        popupTimer -= deltaTime;
        if (popupTimer <= 0.0f) {
            showPopup = false;
            currentWave++;
            prepareNextWave();
        }
    }
}



void Waves::drawZombies(sf::RenderWindow& window) {
    for (const auto& zombie : zombies) {
        zombie->draw(window);
    }

    if (showPopup) {
        window.draw(gameMessage);
        window.draw(messageBackground);
    }
}

bool Waves::allZombiesDead() const {
    return zombies.empty();
}

bool Waves::isFinished() const {
    return currentWave >= waves.size() && zombies.empty();
}

vector<Zombie*> Waves::getZombies() const {
    vector<Zombie*> zombiePtrs;
    for (const auto& z : zombies) {
        zombiePtrs.push_back(z.get());
    }
    return zombiePtrs;
}
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <iostream>
#include "wave.h"
#include <string>
using namespace std;

Waves::Waves(sf::Texture& regularTexture, sf::Texture& fastTexture, sf::Texture& strongTexture, const std::vector<sf::Vector2f>& path,
             sf::Sound* zombieHitSound, sf::Sound* zombieDieSound)
    : regularTexture(regularTexture), fastTexture(fastTexture), strongTexture(strongTexture), path(path),
      zombieHitSound(zombieHitSound), zombieDieSound(zombieDieSound) {
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
        gameMessage.setPosition(300, 450);
        messageBackground.setSize(sf::Vector2f(600, 200));
        messageBackground.setFillColor(sf::Color(0,0,0,180));
        messageBackground.setPosition(200, 400);
        messageBackground.setOutlineThickness(5);
        messageBackground.setOutlineColor(sf::Color::White);
        breachWarning.setFont(font);
        breachWarning.setCharacterSize(50);
        breachWarning.setFillColor(sf::Color::Red);
        breachWarning.setPosition(20, 800);
        waveMessage.setFont(font);
        waveMessage.setCharacterSize(50);
        waveMessage.setFillColor(sf::Color::Red);
        waveMessage.setPosition(575, 50);
        updateBreachWarning();
        updateWaveMessage();
        prepareNextWave();
}

void Waves::prepareNextWave() {
    if (currentWave >= waves.size()) return; 

    spawnQueue.clear();
    const auto& wave = waves[currentWave];
    for (int i = 0; i < wave.strongCount; ++i) {
        spawnQueue.push_back("strong");
    }
    for (int i = 0; i < wave.regularCount; ++i) {
        spawnQueue.push_back("regular");
    }
    for (int i = 0; i < wave.fastCount; ++i) {
        spawnQueue.push_back("fast");
    }
    zombiesSpawned = 0;
    spawnTimer = 0.0f;
}

void Waves::update(float deltaTime) {
    if (isFinished()) return;
    spawnTimer += deltaTime;

    if (zombiesSpawned < spawnQueue.size() && spawnTimer >= 1.0f) {
        const std::string& type = spawnQueue[zombiesSpawned];
        if (type == "strong") {
            zombies.emplace_back(std::make_unique<StrongZombie>(strongTexture, path, zombieHitSound, zombieDieSound));
        } else if (type == "regular") {
            zombies.emplace_back(std::make_unique<Zombie>(regularTexture, path, zombieHitSound, zombieDieSound));
        } else if (type == "fast") {
            zombies.emplace_back(std::make_unique<FastZombie>(fastTexture, path, zombieHitSound, zombieDieSound));
        }
        spawnTimer = 0.0f;
        zombiesSpawned++;
    }

    for (auto& zombie : zombies) {
        zombie->update(deltaTime);

        if (zombie->reachedEnd() && !zombie->isDead()) {
            zombiesend++;
            zombie->takeDamage(1000);
            updateBreachWarning();
        }
    }

    if (zombiesend >= 3) {
        gameOver = true;
        gameMessage.setString("Game Over! :()");
        return;
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
        gameMessage.setString("You won! :)");
    } else {
        gameMessage.setString("Wave " + std::to_string(currentWave + 1) + " complete!");
    }
    }   

    if (showPopup) {
        popupTimer -= deltaTime;
        if (popupTimer <= 0.0f) {
            showPopup = false;
            currentWave++;
            updateWaveMessage();
            prepareNextWave();
        }
    }
}



void Waves::drawZombies(sf::RenderWindow& window) {
    for (const auto& zombie : zombies) {
        zombie->draw(window);
    }

    if (showPopup) {
        window.draw(messageBackground);
        window.draw(gameMessage);
    }
    if (gameOver) {
        window.draw(messageBackground);
        window.draw(gameMessage);
    }
    window.draw(breachWarning);
    window.draw(waveMessage);
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

void Waves::updateBreachWarning() {
    int remainingZombies = 3 - zombiesend;
    if (remainingZombies < 0) {
        remainingZombies = 0;
    }
    breachWarning.setString("Chest Health: " + std::to_string(remainingZombies));
}

void Waves::updateWaveMessage() {
    int wavescompleted = currentWave;
    if (wavescompleted < 0) {
        wavescompleted = 0;
    }
    waveMessage.setString("Waves Completed: " + std::to_string(wavescompleted) + " / " + std::to_string(waves.size()));
}
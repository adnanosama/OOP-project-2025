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
        // Do not show "You won!" here; let main game handle win message
        gameMessage.setString(""); // Or leave empty, or set to a different message if you want
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
    // Draw all zombies first
    for (const auto& zombie : zombies) {
        zombie->draw(window);
    }

    // Store current view for restoration later
    sf::View currentView = window.getView();
    sf::Vector2u winSize = window.getSize();
    
    // Switch to UI view for HUD elements
    sf::View uiView(sf::FloatRect(0, 0, winSize.x, winSize.y));
    window.setView(uiView);

    // Draw wave completion popup if active and has message
    if (showPopup && !gameMessage.getString().isEmpty()) {
        // Create message box
        sf::FloatRect textBounds = gameMessage.getLocalBounds();
        float padding = 30.f;
        sf::Vector2f boxSize(textBounds.width + padding*2, textBounds.height + padding*2);
        
        messageBackground.setSize(boxSize);
        messageBackground.setPosition((winSize.x - boxSize.x)/2, (winSize.y - boxSize.y)/2);
        messageBackground.setFillColor(sf::Color(0, 0, 0, 220));
        messageBackground.setOutlineThickness(2.f);
        messageBackground.setOutlineColor(sf::Color::White);

        // Center text in box
        gameMessage.setPosition(
            (winSize.x - textBounds.width)/2,
            (winSize.y - textBounds.height)/2 - 10
        );

        window.draw(messageBackground);
        window.draw(gameMessage);
    }

    // Update and draw HUD positions
    breachWarning.setPosition(20, winSize.y - 70);  // Bottom left
    waveMessage.setPosition(winSize.x - waveMessage.getLocalBounds().width - 20, 20);  // Top right
    
    window.draw(breachWarning);
    window.draw(waveMessage);

    // Restore original view
    window.setView(currentView);
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
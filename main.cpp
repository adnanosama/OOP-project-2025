#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "map.h"
#include "zombie.h"
#include "wave.h"
#include "tower.h"
#include "bullet.h"
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <cmath>

using namespace std;

const float worldW = Map::width * Map::tileSize;
const float worldH = Map::height * Map::tileSize;

sf::View calculateView(sf::RenderWindow& window) {
    float winW = window.getSize().x;
    float winH = window.getSize().y;
    float winRatio = winW / winH;
    float worldRatio = worldW / worldH;

    float viewW, viewH;
    if (winRatio > worldRatio) {
        viewH = worldH;
        viewW = worldH * winRatio;
    } else {
        viewW = worldW;
        viewH = worldW / winRatio;
    }

    return sf::View(sf::FloatRect((worldW - viewW) / 2.f, (worldH - viewH) / 2.f, viewW, viewH));
}

string showMainMenu(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.loadFromFile("fonts/BarlowCondensed-Regular.ttf")) {
        std::cerr << "Failed to load font\n";
        return "exit";
    }

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("resources/background.png")) {
        std::cerr << "Failed to load background texture\n";
        return "exit";
    }

    sf::Sprite background(backgroundTexture);
    sf::Text title("Tower Defense Simulator", font, 60);
    title.setPosition(100, 200);
    title.setFillColor(sf::Color::White);

    sf::Text startButton("Start Game", font, 40);
    startButton.setPosition(100, 400);
    startButton.setFillColor(sf::Color::Green);

    sf::Text exitButton("Exit Game", font, 40);
    exitButton.setPosition(100, 500);
    exitButton.setFillColor(sf::Color::Red);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) return "exit";
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                if (startButton.getGlobalBounds().contains(mousePos)) return "start";
                if (exitButton.getGlobalBounds().contains(mousePos)) return "exit";
            }
        }

        window.clear();
        window.draw(background);
        window.draw(title);
        window.draw(startButton);
        window.draw(exitButton);
        window.display();
    }

    return "exit";
}

void runGame(sf::RenderWindow& window) {
    sf::View gameView = calculateView(window);
    window.setView(gameView);

    sf::Texture grassTexture, towerTexture, pathTexture, chestTexture;
    sf::Texture regZombieTexture, fastZombieTexture, strongZombieTexture;

    if (!grassTexture.loadFromFile("resources/grass.jpeg") ||
        !towerTexture.loadFromFile("resources/tower.png") ||
        !pathTexture.loadFromFile("resources/path.jpg") ||
        !chestTexture.loadFromFile("resources/chest.jpg") ||
        !regZombieTexture.loadFromFile("resources/zombie.png") ||
        !fastZombieTexture.loadFromFile("resources/fastzombie.png") ||
        !strongZombieTexture.loadFromFile("resources/strongzombie.png")) {
        std::cerr << "Failed to load textures!" << std::endl;
        return;
    }

    grassTexture.setRepeated(true);
    pathTexture.setRepeated(true);

    const int grassRepeatFactor = 4;
    sf::RectangleShape grassBg;
    grassBg.setSize(sf::Vector2f(gameView.getSize()));
    grassBg.setTexture(&grassTexture);
    grassBg.setTextureRect(sf::IntRect(0, 0, gameView.getSize().x * grassRepeatFactor, gameView.getSize().y * grassRepeatFactor));
    grassBg.setPosition(gameView.getCenter() - gameView.getSize() / 2.f);

    const int tileSize = Map::tileSize;
    vector<sf::Vector2f> path = {
        {3 * tileSize, 0 * tileSize},
        {3 * tileSize, 1 * tileSize},
        {3 * tileSize, 2 * tileSize},
        {3 * tileSize, 3 * tileSize},
        {4 * tileSize, 4 * tileSize},
        {5 * tileSize, 5 * tileSize},
        {5 * tileSize, 6 * tileSize},
        {5 * tileSize, 7 * tileSize},
        {5 * tileSize, 8 * tileSize}
    };

    Waves waves(regZombieTexture, fastZombieTexture, strongZombieTexture, path);
    vector<Tower> towers = {
        Tower({1 * tileSize + tileSize/2, 3 * tileSize + tileSize/2}, towerTexture, 300.0f, 1.5f),
        Tower({7 * tileSize + tileSize/2, 6 * tileSize + tileSize/2}, towerTexture, 300.0f, 1.5f)
    };

    sf::Clock clock; 
    std::vector<bool> towerClicked(towers.size(), false);
    std::vector<float> towerClickCooldown(towers.size(), 0.f);
    const float customCooldown = 0.3f; //cooldown value

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || 
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                gameView = calculateView(window);
                window.setView(gameView);
                grassBg.setSize(gameView.getSize());
                grassBg.setTextureRect(sf::IntRect(0, 0, gameView.getSize().x * grassRepeatFactor, gameView.getSize().y * grassRepeatFactor));
                grassBg.setPosition(gameView.getCenter() - gameView.getSize() / 2.f);
            } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f clickPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                for (size_t i = 0; i < towers.size(); ++i) {
                    sf::FloatRect towerBounds(
                        towers[i].getPosition().x - towerTexture.getSize().x / 2.f,
                        towers[i].getPosition().y - towerTexture.getSize().y / 2.f,
                        towerTexture.getSize().x,
                        towerTexture.getSize().y
                    );
                    if (towerBounds.contains(clickPos) && towerClickCooldown[i] <= 0.f) {
                        towerClicked[i] = true;
                        towerClickCooldown[i] = customCooldown;
                    }
                }
            }
        }

        float deltaTime = clock.restart().asSeconds();

        // Update cooldowns
        for (size_t i = 0; i < towerClickCooldown.size(); ++i) {
            if (towerClickCooldown[i] > 0.f)
                towerClickCooldown[i] -= deltaTime;
        }

        waves.update(deltaTime);
        auto zombies = waves.getZombies();

        for (size_t i = 0; i < towers.size(); ++i) {
            bool canShoot = false;

            if (towerClicked[i]) {
                for (auto z : zombies) {
                    if (z->isDead()) continue;
                    float dx = towers[i].getPosition().x - z->getPosition().x;
                    float dy = towers[i].getPosition().y - z->getPosition().y;
                    float dist = hypot(dx, dy);
                    if (dist < towers[i].getRange()) {
                        canShoot = true;
                        break;
                    }
                }
            }

            towers[i].update(deltaTime, zombies, canShoot);
            towerClicked[i] = false;
        }

        window.clear();
        window.setView(gameView);
        window.draw(grassBg);
        Map::drawMap(window, grassTexture, towerTexture, pathTexture, chestTexture);
        waves.drawZombies(window);
        for (auto& tower : towers) tower.draw(window);
        window.display();
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 900), "Tower Defense", sf::Style::Default);
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        string result = showMainMenu(window);
        if (result == "start") {
            runGame(window);
        } else if (result == "exit") {
            window.close();
        }
    }

    return 0;
}

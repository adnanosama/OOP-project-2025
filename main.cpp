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

string showMainMenu(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.loadFromFile("fonts/BarlowCondensed-Regular.ttf")) {
        std::cerr << "Failed to load font\n";
        return "exit";
    }

    sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));

    sf::Text title("Tower Defense Simulator", font, 60);
    title.setPosition(100, 200);
    title.setFillColor(sf::Color::White);

    sf::Text startbutton("Start Game", font, 32);
    startbutton.setPosition(100, 400);
    startbutton.setFillColor(sf::Color::Green);

    sf::Text exitbutton("Exit Game", font, 32);
    exitbutton.setPosition(100, 500);
    exitbutton.setFillColor(sf::Color::Red);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                return "exit";
            }
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords(
                    sf::Mouse::getPosition(window)
                );
                if (startbutton.getGlobalBounds().contains(mousePos)) {
                    return "start";
                }
                if (exitbutton.getGlobalBounds().contains(mousePos)) {
                    return "exit";
                }
            }
        }

        window.clear();
        window.draw(overlay);
        window.draw(title);
        window.draw(startbutton);
        window.draw(exitbutton);
        window.display();
    }
    return "exit";
}

void runGame(sf::RenderWindow& window) {
    bool shootingActive = false;

    // Load textures
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
    vector<Tower> towers;
    const sf::Vector2f tower1Pos(1 * tileSize + tileSize/2, 3 * tileSize + tileSize/2);
    const sf::Vector2f tower2Pos(7 * tileSize + tileSize/2, 7 * tileSize + tileSize/2);
    towers.emplace_back(tower1Pos, towerTexture, 300.0f, 1.5f);
    towers.emplace_back(tower2Pos, towerTexture, 300.0f, 1.5f);

    sf::Clock clock;
    bool damage = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
               (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape)) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::E) {
                shootingActive = true;
            }
            if (event.type == sf::Event::KeyReleased &&
                event.key.code == sf::Keyboard::E) {
                shootingActive = false;
            }
        }

        float deltaTime = clock.restart().asSeconds();
        waves.update(deltaTime);
        auto zombies = waves.getZombies();

        int bestTower = -1;
        float bestDist = numeric_limits<float>::max();
        for (int i = 0; i < (int)towers.size(); ++i) {
            float minD = numeric_limits<float>::max();
            for (auto z : zombies) {
                if (z->isDead()) continue;
                float dx = towers[i].getPosition().x - z->getPosition().x;
                float dy = towers[i].getPosition().y - z->getPosition().y;
                float d  = hypot(dx, dy);
                if (d < towers[i].getRange() && d < minD)
                    minD = d;
            }
            if (minD < bestDist) {
                bestDist  = minD;
                bestTower = (minD < numeric_limits<float>::max() ? i : -1);
            }
        }

        // Only bestTower may shoot when E is down
        for (int i = 0; i < (int)towers.size(); ++i) {
            bool canShoot = shootingActive && (i == bestTower);
            towers[i].update(deltaTime, zombies, canShoot);
        }

        if (damage) {
            for (Zombie* z : zombies) {
                z->takeDamage(1);
            }
            damage = false;
        }

        window.clear();
        Map::drawMap(window, grassTexture, towerTexture, pathTexture, chestTexture);
        waves.drawZombies(window);
        for (auto& tower : towers) {
            tower.draw(window);
        }
        window.display();
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 900), "Tower Defense");
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

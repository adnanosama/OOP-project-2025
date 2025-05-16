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

//——— Maintain aspect ratio on resize ———
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

    return sf::View(
      sf::FloatRect((worldW - viewW) / 2.f,
                    (worldH - viewH) / 2.f,
                    viewW, viewH)
    );
}

//——— Main Menu with scaled background ———
string showMainMenu(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.loadFromFile("fonts/BarlowCondensed-Regular.ttf")) {
        cerr << "Failed to load font\n";
        return "exit";
    }

    sf::Texture bgTex;
    if (!bgTex.loadFromFile("resources/background.JPG")) {
        cerr << "Failed to load background texture\n";
        return "exit";
    }
    sf::Sprite bg(bgTex);

    // scale to window
    auto winSize = window.getSize();
    auto texSize = bgTex.getSize();
    bg.setScale(
      float(winSize.x) / texSize.x,
      float(winSize.y) / texSize.y
    );

    sf::Text title("Tower Defense Simulator", font, 60);
    title.setPosition(100, 200);
    title.setFillColor(sf::Color::White);

    sf::Text startB("Start Game", font, 40);
    startB.setPosition(100, 400);
    startB.setFillColor(sf::Color::Green);

    sf::Text exitB("Exit Game", font, 40);
    exitB.setPosition(100, 500);
    exitB.setFillColor(sf::Color::Red);

    while (window.isOpen()) {
        sf::Event evt;
        while (window.pollEvent(evt)) {
            if (evt.type == sf::Event::Closed) return "exit";
            if (evt.type == sf::Event::MouseButtonPressed &&
                evt.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f m = window.mapPixelToCoords(
                  sf::Mouse::getPosition(window)
                );
                if (startB.getGlobalBounds().contains(m)) return "start";
                if (exitB.getGlobalBounds().contains(m)) return "exit";
            }
        }
        window.clear();
        window.draw(bg);
        window.draw(title);
        window.draw(startB);
        window.draw(exitB);
        window.display();
    }
    return "exit";
}

//——— Game Over alert ———
string showGameOver(sf::RenderWindow& window, sf::View& gameView) {
    // reuse same font
    sf::Font font;
    font.loadFromFile("fonts/BarlowCondensed-Regular.ttf");

    sf::Text over("Game Over!", font, 80);
    over.setFillColor(sf::Color::Red);
    over.setPosition(worldW/2 - 200, worldH/2 - 100);

    sf::Text info("Press R to Restart  |  Esc to Exit", font, 30);
    info.setFillColor(sf::Color::White);
    info.setPosition(worldW/2 - 250, worldH/2 + 20);

    window.setView(gameView);
    while (window.isOpen()) {
        sf::Event evt;
        while (window.pollEvent(evt)) {
            if (evt.type == sf::Event::Closed) return "exit";
            if (evt.type == sf::Event::KeyPressed) {
                if (evt.key.code == sf::Keyboard::R) return "restart";
                if (evt.key.code == sf::Keyboard::Escape) return "exit";
            }
        }
        window.clear();
        window.setView(gameView);
        window.draw(over);
        window.draw(info);
        window.display();
    }
    return "exit";
}

//——— The merged Run Logic ———
void runGame(sf::RenderWindow& window) {
    sf::View gameView = calculateView(window);
    window.setView(gameView);

    // load textures
    sf::Texture grassT, towerT, pathT, chestT;
    sf::Texture regZ, fastZ, strongZ;
    if (!grassT.loadFromFile("resources/grass.jpeg") ||
        !towerT.loadFromFile("resources/tower.png") ||
        !pathT.loadFromFile("resources/path.jpg") ||
        !chestT.loadFromFile("resources/chest.jpg") ||
        !regZ.loadFromFile("resources/zombie.png") ||
        !fastZ.loadFromFile("resources/fastzombie.png") ||
        !strongZ.loadFromFile("resources/strongzombie.png"))
    {
        cerr << "Failed to load textures!" << endl;
        return;
    }
    grassT.setRepeated(true);
    pathT.setRepeated(true);

    // repeated grass background
    const int repeat = 4;
    sf::RectangleShape grassBg;
    grassBg.setSize(gameView.getSize());
    grassBg.setTexture(&grassT);
    grassBg.setTextureRect(
      sf::IntRect(0,0,
        gameView.getSize().x * repeat,
        gameView.getSize().y * repeat
      )
    );
    grassBg.setPosition(
      gameView.getCenter() - gameView.getSize() / 2.f
    );

    // path & waves
    const int ts = Map::tileSize;
    vector<sf::Vector2f> path = {
        {3*ts,0}, {3*ts,1*ts}, {3*ts,2*ts}, {3*ts,3*ts},
        {4*ts,4*ts}, {5*ts,5*ts}, {5*ts,6*ts}, {5*ts,7*ts},
        {5*ts,8*ts}
    };
    Waves waves(regZ, fastZ, strongZ, path);

    // two towers
    vector<Tower> towers = {
      Tower({1*ts+ts/2,3*ts+ts/2}, towerT, 300.f, 1.5f),
      Tower({7*ts+ts/2,6*ts+ts/2}, towerT, 300.f, 1.5f)
    };

    // per-tower click cooldown
    const float cd = 0.5f;
    vector<bool> clicked(towers.size(),false);
    vector<float> cooldown(towers.size(),0.f);

    sf::Clock clk;
    bool gameOver = false;

    while (window.isOpen()) {
        float dt = clk.restart().asSeconds();
        sf::Event evt;
        while (window.pollEvent(evt)) {
            if (evt.type == sf::Event::Closed ||
               (evt.type==sf::Event::KeyPressed && evt.key.code==sf::Keyboard::Escape))
            {
                window.close();
                return;
            }
            else if (evt.type==sf::Event::Resized) {
                gameView = calculateView(window);
                window.setView(gameView);
                grassBg.setSize(gameView.getSize());
                grassBg.setTextureRect(
                  sf::IntRect(0,0,
                    gameView.getSize().x*repeat,
                    gameView.getSize().y*repeat
                  )
                );
                grassBg.setPosition(
                  gameView.getCenter() - gameView.getSize()/2.f
                );
            }
            else if (evt.type==sf::Event::MouseButtonPressed &&
                     evt.mouseButton.button==sf::Mouse::Left)
            {
                auto mp = window.mapPixelToCoords(
                  {evt.mouseButton.x, evt.mouseButton.y}
                );
                for (size_t i=0; i<towers.size(); ++i) {
                    sf::FloatRect b(
                      towers[i].getPosition().x - towerT.getSize().x/2.f,
                      towers[i].getPosition().y - towerT.getSize().y/2.f,
                      towerT.getSize().x,
                      towerT.getSize().y
                    );
                    if (b.contains(mp) && cooldown[i]<=0.f) {
                        clicked[i] = true;
                        cooldown[i] = cd;
                    }
                }
            }
        }

        // update cooldowns
        for (auto& c : cooldown)
            if (c > 0.f) c -= dt;

        // update waves & towers
        waves.update(dt);
        auto zombies = waves.getZombies();

        // check Game Over: any zombie at end of path?
        sf::Vector2f endPt = path.back();
        for (auto z : zombies) {
            if (!z->isDead()) {
                float dx = z->getPosition().x - endPt.x;
                float dy = z->getPosition().y - endPt.y;
                if (hypot(dx,dy) < 1.f) {
                    gameOver = true;
                }
            }
        }
        if (gameOver) {
            string choice = showGameOver(window, gameView);
            if (choice == "restart") {
                return runGame(window);  // recursion to restart
            } else {
                window.close();
                return;
            }
        }

        // tower shooting
        for (size_t i=0; i<towers.size(); ++i) {
            bool can = false;
            if (clicked[i]) {
                for (auto z : zombies) {
                    if (z->isDead()) continue;
                    float dx = towers[i].getPosition().x - z->getPosition().x;
                    float dy = towers[i].getPosition().y - z->getPosition().y;
                    if (hypot(dx,dy) < towers[i].getRange()) {
                        can = true;
                        break;
                    }
                }
            }
            towers[i].update(dt, zombies, can);
            clicked[i] = false;
        }

        // draw everything
        window.clear();
        window.setView(gameView);
        window.draw(grassBg);
        Map::drawMap(window, grassT, towerT, pathT, chestT);
        waves.drawZombies(window);
        for (auto& t : towers) t.draw(window);
        window.display();
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 900), "Tower Defense", sf::Style::Default);
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        string res = showMainMenu(window);
        if (res == "start") {
            runGame(window);
        } else {
            window.close();
        }
    }
    return 0;
}

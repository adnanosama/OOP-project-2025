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
#include <fstream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <algorithm>

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

//——— Centering text helper function ———
void centerText(sf::Text& text, const sf::RectangleShape& button) {
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f,
                   textRect.top + textRect.height/2.0f);
    text.setPosition(button.getPosition().x + button.getSize().x/2.0f,
                     button.getPosition().y + button.getSize().y/2.0f);
}

//——— Main Menu with title and boxes ———
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

    // Title
    sf::Text title("TOWER DEFENSE SIMULATOR", font, 60);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);

    // Button parameters
    const float buttonWidth = 400.f;
    const float buttonHeight = 80.f;
    const float buttonSpacing = 30.f;
    const float initialY = 300.f;

    // Create buttons
    vector<sf::RectangleShape> buttons(3);
    for(auto& btn : buttons) {
        btn.setSize({buttonWidth, buttonHeight});
        btn.setFillColor(sf::Color(70, 70, 70, 200));
        btn.setOutlineThickness(2.f);
        btn.setOutlineColor(sf::Color::White);
    }

    // Button texts
    vector<sf::Text> buttonTexts = {
        sf::Text("START GAME", font, 40),
        sf::Text("SCOREBOARD", font, 40),
        sf::Text("EXIT GAME", font, 40)
    };

    for(auto& text : buttonTexts) {
        text.setFillColor(sf::Color::White);
    }

    while (window.isOpen()) {
        auto winSize = window.getSize();
        float winWidth = static_cast<float>(winSize.x);
        float winHeight = static_cast<float>(winSize.y);

        // Position elements
        title.setPosition((winWidth - title.getLocalBounds().width)/2, 150);
        
        float yPos = initialY;
        for(size_t i=0; i<buttons.size(); i++) {
            buttons[i].setPosition((winWidth - buttonWidth)/2, yPos);
            centerText(buttonTexts[i], buttons[i]);
            yPos += buttonHeight + buttonSpacing;
        }

        // Update background scale
        bg.setScale(winWidth/bgTex.getSize().x, winHeight/bgTex.getSize().y);

        // Handle mouse hover
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        for(size_t i=0; i<buttons.size(); i++) {
            if(buttons[i].getGlobalBounds().contains(mousePos)) {
                buttons[i].setFillColor(sf::Color(100, 100, 100, 200));
                buttonTexts[i].setFillColor(sf::Color::Yellow);
            } else {
                buttons[i].setFillColor(sf::Color(70, 70, 70, 200));
                buttonTexts[i].setFillColor(sf::Color::White);
            }
        }

        sf::Event evt;
        while (window.pollEvent(evt)) {
            if (evt.type == sf::Event::Closed) return "exit";
            if (evt.type == sf::Event::Resized) {
                sf::View view = sf::View(sf::FloatRect(0, 0, evt.size.width, evt.size.height));
                window.setView(view);
            }
            if (evt.type == sf::Event::MouseButtonPressed &&
                evt.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f m = window.mapPixelToCoords(
                    {evt.mouseButton.x, evt.mouseButton.y}
                );
                if (buttons[0].getGlobalBounds().contains(m)) return "start";
                if (buttons[1].getGlobalBounds().contains(m)) return "scoreboard";
                if (buttons[2].getGlobalBounds().contains(m)) return "exit";
            }
        }

        window.clear();
        window.draw(bg);
        window.draw(title);
        for(const auto& btn : buttons) window.draw(btn);
        for(const auto& text : buttonTexts) window.draw(text);
        window.display();
    }
    return "exit";
}

//——— Scoreboard Screen ———
vector<pair<string, float>> loadScores();

string showScoreboard(sf::RenderWindow& window) {
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

    // Title
    sf::Text title("SCOREBOARD", font, 60);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);

    // Back button
    sf::RectangleShape backButton({200.f, 60.f});
    backButton.setFillColor(sf::Color(70, 70, 70, 200));
    backButton.setOutlineThickness(2.f);
    backButton.setOutlineColor(sf::Color::White);
    sf::Text backText("BACK", font, 40);
    backText.setFillColor(sf::Color::White);

    // Load scores from file
    auto scoresData = loadScores();

    // Create score texts
    vector<sf::Text> scores;
    int rank = 1;
    for(const auto& data : scoresData) {
        // Format time to 2 decimal places
        stringstream ss;
        ss.precision(2);
        ss << fixed << data.second << "s";
        scores.emplace_back(
            to_string(rank++) + ". " + data.first + " - " + ss.str(),
            font, 45
        );
        scores.back().setFillColor(sf::Color::Yellow);
        if(rank > 10) break; // Show top 10
    }

    while (window.isOpen()) {
        auto winSize = window.getSize();
        float winWidth = static_cast<float>(winSize.x);
        float winHeight = static_cast<float>(winSize.y);

        // Position elements
        title.setPosition((winWidth - title.getLocalBounds().width)/2, 100);
        backButton.setPosition(50, winHeight - 100);
        centerText(backText, backButton);

        float yPos = 200.f;
        for(auto& score : scores) {
            score.setPosition((winWidth - score.getLocalBounds().width)/2, yPos);
            yPos += 80.f;
        }

        // Update background scale
        bg.setScale(winWidth/bgTex.getSize().x, winHeight/bgTex.getSize().y);

        // Handle interactions
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        if(backButton.getGlobalBounds().contains(mousePos)) {
            backButton.setFillColor(sf::Color(100, 100, 100, 200));
            backText.setFillColor(sf::Color::Yellow);
        } else {
            backButton.setFillColor(sf::Color(70, 70, 70, 200));
            backText.setFillColor(sf::Color::White);
        }

        sf::Event evt;
        while (window.pollEvent(evt)) {
            if (evt.type == sf::Event::Closed) return "exit";
            if (evt.type == sf::Event::Resized) {
                sf::View view = sf::View(sf::FloatRect(0, 0, evt.size.width, evt.size.height));
                window.setView(view);
            }
            if (evt.type == sf::Event::MouseButtonPressed &&
                evt.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f m = window.mapPixelToCoords(
                    {evt.mouseButton.x, evt.mouseButton.y}
                );
                if (backButton.getGlobalBounds().contains(m)) return "menu";
            }
        }

        window.clear();
        window.draw(bg);
        window.draw(title);
        for(const auto& score : scores) window.draw(score);
        window.draw(backButton);
        window.draw(backText);
        window.display();
    }
    return "exit";
}

//——— Score handling functions ———
vector<pair<string, float>> loadScores() {
    vector<pair<string, float>> scores;
    ifstream file("scores.txt");
    if (file.is_open()) {
        string name;
        float time;
        while (file >> name >> time) {
            scores.emplace_back(name, time);
        }
        file.close();
    }
    // Sort by best time
    sort(scores.begin(), scores.end(), 
        [](const pair<string, float>& a, const pair<string, float>& b) {
            return a.second < b.second;
        });
    return scores;
}

void saveScore(const string& name, float time) {
    ofstream file("scores.txt", ios::app);
    if (file.is_open()) {
        file << name << " " << time << "\n";
        file.close();
    }
}

//——— Name input screen ———
string getNameInput(sf::RenderWindow& window) {
    // Store and set the default view
    sf::View defaultView = window.getDefaultView();
    window.setView(defaultView);

    sf::Font font;
    if (!font.loadFromFile("fonts/BarlowCondensed-Regular.ttf")) {
        return "Anonymous";
    }

    sf::Texture bgTex;
    if (!bgTex.loadFromFile("resources/background.JPG")) {
        return "Anonymous";
    }
    sf::Sprite bg(bgTex);

    string input;
    sf::Text inputText("", font, 40);
    inputText.setFillColor(sf::Color::White);
    
    sf::Text prompt("Enter your name:", font, 50);
    prompt.setFillColor(sf::Color::White);

    sf::RectangleShape textBox(sf::Vector2f(400, 60));
    textBox.setFillColor(sf::Color::Transparent);
    textBox.setOutlineThickness(2);
    textBox.setOutlineColor(sf::Color::White);

    while (window.isOpen()) {
        auto winSize = window.getSize();
        float winWidth = static_cast<float>(winSize.x);
        float winHeight = static_cast<float>(winSize.y);

        // Update view on resize
        window.setView(sf::View(sf::FloatRect(0, 0, winWidth, winHeight)));

        // Position elements
        prompt.setPosition((winWidth - prompt.getLocalBounds().width)/2, 300);
        textBox.setPosition((winWidth - 400)/2, 400);
        inputText.setPosition(textBox.getPosition().x + 10, textBox.getPosition().y + 5);

        // Scale background
        bg.setScale(
            winWidth/static_cast<float>(bgTex.getSize().x),
            winHeight/static_cast<float>(bgTex.getSize().y)
        );

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                return "";
            if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
            }
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b' && !input.empty()) {
                    input.pop_back();
                }
                else if (event.text.unicode < 128 && event.text.unicode != '\r') {
                    if (input.size() < 15)
                        input += static_cast<char>(event.text.unicode);
                }
                inputText.setString(input);
            }
            if (event.type == sf::Event::KeyPressed && 
                event.key.code == sf::Keyboard::Enter && !input.empty()) {
                return input;
            }
        }

        window.clear();
        window.draw(bg);
        window.draw(prompt);
        window.draw(textBox);
        window.draw(inputText);
        window.display();
    }
    return "";
}

string showGameOver(sf::RenderWindow& window) {
    // Store and set the default view
    sf::View defaultView = window.getDefaultView();
    window.setView(defaultView);

    sf::Font font;
    font.loadFromFile("fonts/BarlowCondensed-Regular.ttf");

    sf::Texture bgTex;
    bgTex.loadFromFile("resources/background.JPG");
    sf::Sprite bg(bgTex);

    // Message box
    sf::RectangleShape box(sf::Vector2f(600, 300));
    box.setFillColor(sf::Color(0, 0, 0, 220));
    box.setOutlineThickness(4.f);
    box.setOutlineColor(sf::Color::White);

    sf::Text title("GAME OVER", font, 70);
    title.setFillColor(sf::Color::Red);
    title.setStyle(sf::Text::Bold);

    sf::Text prompt("Press R to Retry\nEsc to Menu", font, 40);
    prompt.setFillColor(sf::Color::White);

    while (window.isOpen()) {
        auto winSize = window.getSize();
        float winWidth = winSize.x;
        float winHeight = winSize.y;

        // Position elements
        bg.setScale(winWidth/float(bgTex.getSize().x), winHeight/float(bgTex.getSize().y));
        box.setPosition((winWidth - 600)/2, (winHeight - 300)/2);
        title.setPosition((winWidth - title.getLocalBounds().width)/2, box.getPosition().y + 40);
        prompt.setPosition((winWidth - prompt.getLocalBounds().width)/2, box.getPosition().y + 150);

        sf::Event evt;
        while (window.pollEvent(evt)) {
            if (evt.type == sf::Event::Closed) return "exit";
            if (evt.type == sf::Event::KeyPressed) {
                if (evt.key.code == sf::Keyboard::R) return "retry";
                if (evt.key.code == sf::Keyboard::Escape) return "menu";
            }
        }

        window.clear();
        window.draw(bg);
        window.draw(box);
        window.draw(title);
        window.draw(prompt);
        window.display();
    }
    // Make sure to return to default view before returning
    window.setView(defaultView);
    return "exit";
}

//——— The merged Run Logic ———
void runGame(sf::RenderWindow& window, const string& playerName) {
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

    // load sound effects
    sf::SoundBuffer bulletHitBuffer, zombieHitBuffer, zombieDieBuffer;
    sf::Sound bulletHitSound, zombieHitSound, zombieDieSound;

    if (!bulletHitBuffer.loadFromFile("resources/bullet-hit.wav") ||
        !zombieHitBuffer.loadFromFile("resources/zombiehit.wav") ||
        !zombieDieBuffer.loadFromFile("resources/zombiedie.wav")) {
        cerr << "Failed to load sound effects!" << endl;
        return;
    }
    bulletHitSound.setBuffer(bulletHitBuffer);
    zombieHitSound.setBuffer(zombieHitBuffer);
    zombieDieSound.setBuffer(zombieDieBuffer);

    // path & waves
    const int ts = Map::tileSize;
    vector<sf::Vector2f> path = {
        {3*ts,0}, {3*ts,1*ts}, {3*ts,2*ts}, {3*ts,3*ts},
        {4*ts,4*ts}, {5*ts,5*ts}, {5*ts,6*ts}, {5*ts,7*ts},
        {5*ts,8*ts}
    };
    Waves waves(regZ, fastZ, strongZ, path, &zombieHitSound, &zombieDieSound);

    // two towers
    vector<Tower> towers = {
      Tower({1*ts+ts/2,3*ts+ts/2}, towerT, 300.f, 1.5f, &bulletHitSound),
      Tower({7*ts+ts/2,6*ts+ts/2}, towerT, 300.f, 1.5f, &bulletHitSound)
    };

    // per-tower click cooldown
    const float cd = 0.5f;
    vector<bool> clicked(towers.size(),false);
    vector<float> cooldown(towers.size(),0.f);

    sf::Clock clk;
    sf::Clock gameClock;
    float completionTime = 0.f;
    bool gameOver = false;
    bool gameRunning = true;
    bool wonGame = false;

    // Menu button
    sf::Font font;
    font.loadFromFile("fonts/BarlowCondensed-Regular.ttf");
    sf::Text menuButton("Menu", font, 30);
    menuButton.setFillColor(sf::Color::White);
    menuButton.setPosition(20, 20);

    while (window.isOpen() && gameRunning) {
        float dt = clk.restart().asSeconds();
        completionTime += dt;

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
                // Menu button click
                if (menuButton.getGlobalBounds().contains(mp)) {
                    gameRunning = false;
                }
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
            // Store current view
            sf::View defaultView = window.getDefaultView();
            window.setView(defaultView);
            
            string choice = showGameOver(window);
            if (choice == "retry") {
                return runGame(window, playerName);
            } else if (choice == "menu") {
                gameRunning = false;
                window.setView(defaultView);  // Reset view before returning to menu
                continue;
            } else if (choice == "exit") {
                window.close();
                return;
            }
        }

        // WIN CONDITION: all zombies dead and waves finished
        if (waves.isFinished() && !wonGame) {
            completionTime = gameClock.getElapsedTime().asSeconds();
            saveScore(playerName, completionTime);
            wonGame = true;

            // Create win screen elements
            sf::RectangleShape winBox(sf::Vector2f(600, 300));
            winBox.setFillColor(sf::Color(0, 0, 0, 220));
            winBox.setOutlineThickness(4.f);
            winBox.setOutlineColor(sf::Color::White);

            sf::Text winText("VICTORY!", font, 70);
            winText.setFillColor(sf::Color::Green);
            winText.setStyle(sf::Text::Bold);

            sf::Text timeText("Time: " + to_string(int(completionTime)) + "s", font, 40);
            timeText.setFillColor(sf::Color::White);

            // Position elements using window size
            sf::Vector2u winSize = window.getSize();
            winBox.setPosition((winSize.x - 600)/2, (winSize.y - 300)/2);
            winText.setPosition((winSize.x - winText.getLocalBounds().width)/2, winBox.getPosition().y + 40);
            timeText.setPosition((winSize.x - timeText.getLocalBounds().width)/2, winBox.getPosition().y + 150);

            // Draw final frame
            window.clear();
            window.setView(gameView);
            window.draw(grassBg);
            Map::drawMap(window, grassT, towerT, pathT, chestT);
            waves.drawZombies(window);
            for (auto& t : towers) t.draw(window);
            window.setView(sf::View(sf::FloatRect(0, 0, winSize.x, winSize.y)));
            window.draw(winBox);
            window.draw(winText);
            window.draw(timeText);
            window.display();
            
            sf::sleep(sf::seconds(2));
            gameRunning = false;
            continue;
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

        // Draw menu button and timer
        window.draw(menuButton);
        sf::Text timerText("Time: " + to_string(int(completionTime)) + "s", font, 30);
        timerText.setFillColor(sf::Color::White);
        timerText.setPosition(20, 60);
        window.draw(timerText);

        window.display();
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 900), "Tower Defense", sf::Style::Default);
    window.setFramerateLimit(60);

    // Store initial window size and view
    const sf::Vector2u initialSize(1000, 900);
    sf::View defaultView(sf::FloatRect(0, 0, initialSize.x, initialSize.y));

    while (window.isOpen()) {
        // Reset window size and view before showing menu
        window.setSize(initialSize);
        window.setView(defaultView);
        
        string res = showMainMenu(window);
        
        if (res == "start") {
            // Reset for name input screen
            window.setSize(initialSize);
            window.setView(defaultView);
            
            string name = getNameInput(window);
            if (name.empty()) name = "Anonymous";
            
            // Run game and reset after
            runGame(window, name);
            
            // Reset after game ends
            window.setSize(initialSize);
            window.setView(defaultView);
        }
        else if (res == "scoreboard") {
            // Reset for scoreboard
            window.setSize(initialSize);
            window.setView(defaultView);
            
            while (window.isOpen()) {
                string sbRes = showScoreboard(window);
                if (sbRes == "menu") {
                    window.setSize(initialSize);
                    window.setView(defaultView);
                    break;
                }
                if (sbRes == "exit") {
                    window.close();
                    return 0;
                }
            }
        }
        else if (res == "exit") {
            window.close();
        }
    }
    return 0;
}
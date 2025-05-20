#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include "zombie.h"

class Waves {
    std::vector<std::unique_ptr<Zombie>> zombies;
    std::vector<sf::Vector2f> path;

    sf::Texture& regularTexture;
    sf::Texture& fastTexture;
    sf::Texture& strongTexture;

    sf::Text gameMessage;
    sf::RectangleShape messageBackground;
    sf::Text breachWarning;
    sf::Text waveMessage;
    sf::Font font;

    float popupTimer = 0.0f;
    bool showPopup = false;

    float spawnTimer = 0.0f;
    int zombiesSpawned = 0;
    int currentWave = 0;
    int zombiesend = 0;
    bool gameOver = false;

    sf::Sound* zombieHitSound;
    sf::Sound* zombieDieSound;

    struct Wave {
        int regularCount;
        int fastCount;
        int strongCount;
    };

    std::vector<Wave> waves;
    std::vector<std::string> spawnQueue;

    public:
    std::vector<Zombie*> getZombies() const;
    Waves(sf::Texture& regularTexture, sf::Texture& fastTexture, sf::Texture& strongTexture, const std::vector<sf::Vector2f>& path,
          sf::Sound* zombieHitSound, sf::Sound* zombieDieSound);
    void update(float deltaTime);
    void drawZombies(sf::RenderWindow& window);
    bool allZombiesDead() const;
    void prepareNextWave();
    bool isFinished() const;
    void updateBreachWarning();
    void updateWaveMessage();
};
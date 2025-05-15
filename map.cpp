#include <SFML/Graphics.hpp>
#include "map.h"

const int Map::mapData[Map::height][Map::width] = {
    // REMOVED ALL '3's (TOWERS) FROM MAP DATA
    {0, 0, 2, 2, 2, 0, 0, 0, 0, 0},
    {0, 0, 2, 2, 2, 0, 0, 0, 0, 0},
    {0, 0, 2, 2, 2, 0, 0, 0, 0, 0},
    {0, 0, 2, 2, 2, 0, 0, 0, 0, 0}, // Changed from 3 to 0
    {0, 0, 0, 2, 2, 2, 0, 0, 0, 0},
    {0, 0, 0, 0, 2, 2, 2, 0, 0, 0},
    {0, 0, 0, 0, 2, 2, 2, 0, 0, 0},
    {0, 0, 0, 0, 2, 2, 2, 0, 0, 0}, // Changed from 3 to 0
    {0, 0, 0, 0, 2, 1, 2, 0, 0, 0},
};

void Map::drawMap(sf::RenderWindow& window, sf::Texture& grassTexture, sf::Texture& towerTexture, 
                sf::Texture& pathTexture, sf::Texture& chestTexture) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // Always draw grass first
            sf::Sprite grassSprite;
            grassSprite.setTexture(grassTexture);
            grassSprite.setPosition(j * tileSize, i * tileSize);
            window.draw(grassSprite);

            // Draw other elements on top
            switch (Map::mapData[i][j]) {
                case 1: { // Chest
                    sf::Sprite chestSprite;
                    chestSprite.setTexture(chestTexture);
                    sf::Vector2u texSize = chestTexture.getSize();
                    float scale = std::min(
                        tileSize / static_cast<float>(texSize.x),
                        tileSize / static_cast<float>(texSize.y)
                    );
                    chestSprite.setScale(scale, scale);
                    chestSprite.setOrigin(texSize.x/2.0f, texSize.y/2.0f);
                    chestSprite.setPosition(j * tileSize + tileSize/2, i * tileSize + tileSize/2);
                    window.draw(chestSprite);
                    break;
                }
                case 2: { // Path
                    sf::Sprite pathSprite;
                    pathSprite.setTexture(pathTexture);
                    pathSprite.setPosition(j * tileSize, i * tileSize);
                    window.draw(pathSprite);
                    break;
                }
                // REMOVED CASE 3 (TOWERS DRAWN ELSEWHERE)
            }
        }
    }
}
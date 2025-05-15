#include <SFML/Graphics.hpp>

class Map {
public:
    static const int width = 10;  // Should match actual map dimensions
    static const int height = 9;
    static const int tileSize = 100;
    
    // Add this declaration
    static const int mapData[height][width];

    static void drawMap(sf::RenderWindow& window, sf::Texture& grassTexture, 
                       sf::Texture& towerTexture, sf::Texture& pathTexture, 
                       sf::Texture& chestTexture);
};
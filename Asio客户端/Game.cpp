#include"Game.h"
#include<algorithm>
void Game::run() {
    sf::Clock clock;
    while (window.isOpen())
    {
        const float dt = std::min(clock.restart().asSeconds()//上一次调用过去多久，转化为s
            , 0.1f);
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            sc->handleEvent(*event);
        }
        sc->update(dt);
        window.clear(sf::Color(30, 30, 40));
        window.draw(*sc);
        window.display();
    }
}
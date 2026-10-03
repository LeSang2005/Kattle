#pragma once
#include <SFML/Graphics.hpp>
#include"Scene.h"
#include"WorldScene.h"
#include<memory.h>
class Game {
public:
	Game() :sc(std::make_shared<WorldScence>()), window(sf::VideoMode({ WORLDSCENCEWIDTH, WORLDSCENCEHEIGHT }), "game") {}
	void run();
private:
	std::shared_ptr<Scene>sc;
	sf::RenderWindow window;
};

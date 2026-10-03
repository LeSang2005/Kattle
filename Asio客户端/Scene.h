#pragma once
#include <SFML/Graphics.hpp>
class Scene:public sf::Drawable{
public:
	virtual ~Scene() = default;//³éÏó
	virtual void handleEvent(const sf::Event& event)=0;
	virtual void update(float dt)=0;
protected:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override = 0;
};
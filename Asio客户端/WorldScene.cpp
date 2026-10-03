#include"WorldScene.h"
#include"player.h"
WorldScence::WorldScence(unsigned width, unsigned height,std::string name)
	:width_(width), height_(height)
{
	init();
}
WorldScence::~WorldScence() {

}

void WorldScence::init() {
	//场景设计
	Player_ = std::make_shared<Player>();

	//加入vector
	Scencs.push_back(Player_);
	//函数初始化
	/*EventMap.insert({GameEvent::KeyA,std::bind(&WorldScence::handle_key_A,this,std::placeholders::_1)});
	EventMap.insert({ GameEvent::KeyS,std::bind(&WorldScence::handle_key_S,this,std::placeholders::_1) });
	EventMap.insert({ GameEvent::KeyD,std::bind(&WorldScence::handle_key_D,this,std::placeholders::_1) });
	EventMap.insert({ GameEvent::KeyW,std::bind(&WorldScence::handle_Key_W,this,std::placeholders::_1) });*/
	
}

void WorldScence::handleEvent(const sf::Event& event) {
	if (EventMap.count(WhatEvent(event))) {
		EventMap[WhatEvent(event)](event);
	}
	for (auto& object : Scencs) {
		object->handleEvent(event);
	}
}

void WorldScence::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	//渲染
	for (int i = 0; i < Scencs.size(); i++) {
		target.draw(*Scencs[i],states);
	}
}

void WorldScence::update(float dt) {
	for (int i = 0; i < Scencs.size(); i++) {
		Scencs[i]->update(dt);
	}
}
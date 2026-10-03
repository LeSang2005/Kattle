#pragma once
#include"Scene.h"
#include"const.h"
#include<string>
#include<functional>
#include<vector>
#include<unordered_map>
class WorldScence:public Scene {
public:

	WorldScence(unsigned width = WORLDSCENCEWIDTH, unsigned height = WORLDSCENCEHEIGHT, std::string name = "worldscence");
	~WorldScence()override;
	void update(float dt)override;
	//void update()override;
	void handleEvent(const sf::Event& event)override;
	void init();
protected:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
private:
	//私有函数
	/*void handle_Key_W(const sf::Event& event){
		for (int i = 0; i < Scencs.size(); ++i) {
			Scencs[i]->handleEvent(event);
			Scencs[i]->update();
		}
	}
	void handle_key_S(const sf::Event& event) {
		for (int i = 0; i < Scencs.size(); ++i) {
			Scencs[i]->handleEvent(event);
			Scencs[i]->update();
		}
	}
	void handle_key_A(const sf::Event& event) {
		for (int i = 0; i < Scencs.size(); ++i) {
			Scencs[i]->handleEvent(event);
			Scencs[i]->update();
		}
	}
	void handle_key_D(const sf::Event& event) {
		for (int i = 0; i < Scencs.size(); ++i) {
			Scencs[i]->handleEvent(event);
			Scencs[i]->update();
		}
	}*/
	//私有变量
	std::string name_;
	unsigned width_;
	unsigned height_;
	std::string name;
	std::vector< std::shared_ptr<Scene>>Scencs;
	std::unordered_map<GameEvent,std::function<void(const sf::Event& event)>>EventMap;
	std::shared_ptr<Scene>Player_;
};
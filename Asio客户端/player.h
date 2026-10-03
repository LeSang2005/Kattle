#pragma once
#include"Scene.h"
#include"const.h"
#include<functional>
#include<unordered_map>
#include"Animation.h"
class Player :public Scene {
public:
	enum class AnimState {
		Idle_,
		run_
	};
	Player();
	virtual void handleEvent(const sf::Event& event);
	virtual void update(float dt)override;
	void init();
protected:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
private:
	//变量
	int level_;
	bool ShowPassEvent;
	std::unordered_map<GameEvent,std::function<void(const sf::Event& event)>>PlayerEventHandleMap_;
	std::unordered_map<AnimState, std::function<void(float dt)>>AnimationsMap_;
	//图画素材
	sf::Texture player_tex_;
	sf::Texture player_tex_Idle_;
	sf::Texture plater_tex_Attack_;
	//
	sf::Sprite player_;
	//动画
	Animation RunAni_;
	Animation IdleAni_;
	Animation AttackAni_;
	//函数
	//动画函数
	void run(float dt);
	void Idle(float dt);
	void attack(float dt);
	//工具函数
	void LoadAnimal(Animation& ani,sf::Texture& text,const int& n);
	void HandleAnimal(float dt);
	void ApplyCurrentAnim(const Animation& a);
	//事件函数
	void Event_Key_K(const sf::Event& event);
	void updateAni( Animation& Ani);
};

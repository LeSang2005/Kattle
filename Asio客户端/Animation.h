#pragma once
#include <SFML/Graphics.hpp>
#include<string>
//添加完后必须调用init
class Animation {
public:
	struct Sprite_str {
		Sprite_str(sf::Sprite spr,size_t level):Spr_(std::move(spr)),level_(level){
		}
		sf::Sprite Spr_;
		size_t level_;
	};
	Animation(double dt) {
		dt_ = dt;
		NowTime_ = 0;
		start = false;
	}
	void addSprite(const sf::Sprite& sprite_,int Level);
	sf::Sprite GetNowSprite();
	void setNowIndex(size_t index);
	void init();
	int GetSpritesIndex(const sf::Sprite& sf) const {
		for (size_t i = 0; i < Sprites.size(); ++i) {
			if (&Sprites[i].Spr_.getTexture() == &sf.getTexture() &&
				Sprites[i].Spr_.getTextureRect() == sf.getTextureRect())
				return i;
		}
		return -1;
	}
	double GetTime() {
		return dt_;
	}
	const sf::IntRect& getCurrentRect() const { return Sprites[now_index_].Spr_.getTextureRect(); }
	void reset() { now_index_ = 0; NowTime_ = 0; start = false; }
	void updateTime(double Time) {
		NowTime_ += Time;
	}
	double& GetNowTime() {
		return NowTime_;
	}
	void updateStart(bool flag) {
		start = flag;
	}
	bool GetStart() {
		return start;
	}
	void advance() { now_index_ = (now_index_ + 1) % Sprites.size(); }
	const sf::Texture& getCurrentTexture() const {
		return Sprites[now_index_].Spr_.getTexture();
	}
	bool IsEnd() const { return now_index_ + 1 >= Sprites.size(); }
private:
	std::vector<Sprite_str>Sprites;
	size_t now_index_=0;
	std::string Ani_Name_;
	double dt_;
	double NowTime_;
	bool start;
};
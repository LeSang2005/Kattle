#include"Animation.h"
void Animation::init(){
	sort(Sprites.begin(), Sprites.end(), [](Sprite_str& pre,Sprite_str& next) {
		return pre.level_ < next.level_;
		});
}

sf::Sprite Animation::GetNowSprite() {
	sf::Sprite sf_tempInthis= Sprites[now_index_].Spr_;
	now_index_ = (now_index_ + 1) % Sprites.size();
	return sf_tempInthis;
}

void Animation::addSprite(const sf::Sprite& sprite_, int Level) {
	Sprite_str Sprite_(sprite_,Level);
	Sprites.push_back(std::move(Sprite_));
}

void Animation::setNowIndex(size_t index) {
	if (index < 0 || index >= Sprites.size()) {
		return;
	}
	now_index_ = index;
}
#include"player.h"
#include"Log.h"
#include<iostream>
Player::Player()
    : player_(player_tex_, sf::IntRect({ 0, 0 }, { Player_Width, Player_Height })),
    //动画设置
    RunAni_(0.08), IdleAni_(0.08), AttackAni_(0.10)
{
    //加载
    if (!player_tex_.loadFromFile(Player_image_Path)) {
        LOG_ERROR("加载失败: " + Player_image_Path);
        return;
    }
    if (!player_tex_Idle_.loadFromFile(Plater_image_Idle)) {
        LOG_ERROR("加载失败: " + Plater_image_Idle);
    }
    if (!plater_tex_Attack_.loadFromFile(Player_image_attack)) {
        LOG_ERROR("加载失败: " + Plater_image_Idle);
    }
    player_.setOrigin({ Player_Width / 2.0f, Player_Height / 2.0f });
    player_tex_.setSmooth(false);
    player_.setPosition({ Player_Begin_Positon_X, Player_Begin_Position_Y });
    init();
}
void Player::init() {
    //加载移动动画
    LoadAnimal(RunAni_,player_tex_,6);
    //加载平移动画
    LoadAnimal(IdleAni_,player_tex_Idle_,8);
    //加载攻击动画
    LoadAnimal(AttackAni_, plater_tex_Attack_,4);
    //-----------------
    //函数放置
    AnimationsMap_.insert({ AnimState::run_,std::bind(&Player::run,this,std::placeholders::_1) });
    AnimationsMap_.insert({AnimState::Idle_,std::bind(&Player::Idle,this,std::placeholders::_1)});
    //事件响应函数加载 
    PlayerEventHandleMap_.insert({GameEvent::KeyK,std::bind(&Player::Event_Key_K,this,std::placeholders::_1)});
}

void Player::handleEvent(const sf::Event& event) {
    if (PlayerEventHandleMap_.count(WhatEvent(event))) {
        PlayerEventHandleMap_[WhatEvent(event)](event);
    }
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states)const{
    target.draw(player_,states);
}



void Player::update(float dt) {
    HandleAnimal(dt);
}

void Player::run(float dt) {
    float dx = 0.0f, dy = 0.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dx -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dx += 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) dy -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) dy += 1.0f;
    if (dx == 0.0f && dy == 0.0f) {
        RunAni_.reset();
        player_.setTextureRect(RunAni_.getCurrentRect());
        return;
    }
    if (RunAni_.GetStart() == false) {
        RunAni_.updateStart(true);

    }
    const float len = std::sqrt(dx * dx + dy * dy);       
    dx /= len;
    dy /= len;
    if (dx != 0.0f)                                        
        player_.setScale({ dx < 0.f ? -1.f : 1.f, 1.f });

    RunAni_.updateTime(dt);
    player_.move({ dx * Player_Speed * dt, dy * Player_Speed * dt });
    updateAni(RunAni_);
}
//动画处理函数
void Player::LoadAnimal(Animation& ani, sf::Texture& text, const int& n) {
    for (int f = 0; f < n; ++f) {
        sf::Sprite s{ text };
        s.setTextureRect(sf::IntRect({ f * 192, 0 }, { 192, 192 }));
        s.setOrigin({ 96.f, 136.f });
        ani.addSprite(std::move(s), f);
    }
    ani.init();
}

void Player::Idle(float dt) {
    if (IdleAni_.GetStart() == false) {
        player_.setTexture(player_tex_);
        IdleAni_.updateStart(true);
    }
    IdleAni_.updateTime(dt);
    updateAni(IdleAni_);
}

void Player::attack(float dt) {
    if (AttackAni_.GetStart() == false) return;

    AttackAni_.updateTime(dt);

    if (AttackAni_.IsEnd() && AttackAni_.GetNowTime() >= AttackAni_.GetTime()) {
        AttackAni_.updateStart(false);
    }
    updateAni(AttackAni_);
}
//-----------------------
void Player::HandleAnimal(float dt) {
    const bool moving =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);

    if (moving) run(dt);
    else if (AttackAni_.GetStart())attack(dt);
    else        Idle(dt);
}

void Player::ApplyCurrentAnim(const Animation& a) {

    player_.setTexture(a.getCurrentTexture());       
    player_.setTextureRect(a.getCurrentRect());      
}

void Player::Event_Key_K(const sf::Event& event) {
    if (AttackAni_.GetStart()) return;
    AttackAni_.reset();
    AttackAni_.updateStart(true);
    
}

void Player::updateAni( Animation& Ani) {
    if (Ani.GetNowTime() >= Ani.GetTime()) {
        Ani.updateTime(-1 * Ani.GetTime());
        player_.setTextureRect(Ani.getCurrentRect());
        Ani.advance();
    }
    ApplyCurrentAnim(Ani);
}
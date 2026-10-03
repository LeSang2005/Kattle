#pragma once
#include <SFML/Graphics.hpp>

inline constexpr unsigned WORLDSCENCEWIDTH = 800;
inline constexpr unsigned WORLDSCENCEHEIGHT = 600;

enum class GameEvent { 
    MouseLeft,
    MouseRight,
    KeyA, KeyW, KeyS, KeyD,KeyK,
    Close,
    None
};

[[nodiscard]] inline GameEvent WhatEvent(const sf::Event& event) {
    if (event.is<sf::Event::Closed>())
        return GameEvent::Close;
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left)  return GameEvent::MouseLeft;
        if (mb->button == sf::Mouse::Button::Right) return GameEvent::MouseRight;
        return GameEvent::None;
    }
    if (const auto* k = event.getIf<sf::Event::KeyPressed>()) {
        switch (k->code) {
        case sf::Keyboard::Key::A: return GameEvent::KeyA;
        case sf::Keyboard::Key::W: return GameEvent::KeyW;
        case sf::Keyboard::Key::S: return GameEvent::KeyS;
        case sf::Keyboard::Key::D: return GameEvent::KeyD;
        case sf::Keyboard::Key::K: return GameEvent::KeyK;
        default: break;
        }
    }

    return GameEvent::None;
}

//player
inline const std::string Player_image_Path = "image/Warrior_Run.png";
inline const std::string Plater_image_Idle = "image/Warrior_Idle.png";
inline const std::string Player_image_attack = "image/Warrior_Attack1.png";

inline const int Player_Width=192;
inline const int Player_Height = 192;
inline const float Player_Begin_Positon_X = 100.0f;
inline const float Player_Begin_Position_Y = 200.0f;
inline const float Player_Speed= 300.0f;


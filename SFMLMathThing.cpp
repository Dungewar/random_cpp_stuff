//
// Created by dungewar on 4/5/25.
//

#include "SFMLMathThing.h"

#include <iostream>
#include <valarray>
#include <SFML/Graphics.hpp>

constexpr int CYCLE_TIME = 2000;

constexpr double CELL_SIZE = 50;

constexpr double PRECISION = 400;

double MAX_SIZE = 3;

/**
 * Converts HSV values to a color
 * @param h Hue value from 0-360
 * @param s Saturation value from 0-1
 * @param v Value value from 0-1
 * @return A new color with the corresponding HSV
 */
sf::Color hsvToRgb(double h, double s, double v) {
    float c = v * s;
    float x = c * (1 - std::fabs(fmod(h / 60.0f, 2) - 1));
    float m = v - c;

    float rPrime, gPrime, bPrime;

    if (h >= 0 && h < 60) {
        rPrime = c; gPrime = x; bPrime = 0;
    } else if (h >= 60 && h < 120) {
        rPrime = x; gPrime = c; bPrime = 0;
    } else if (h >= 120 && h < 180) {
        rPrime = 0; gPrime = c; bPrime = x;
    } else if (h >= 180 && h < 240) {
        rPrime = 0; gPrime = x; bPrime = c;
    } else if (h >= 240 && h < 300) {
        rPrime = x; gPrime = 0; bPrime = c;
    } else {
        rPrime = c; gPrime = 0; bPrime = x;
    }

    auto r = static_cast<std::uint8_t>((rPrime + m) * 255);
    auto g = static_cast<std::uint8_t>((gPrime + m) * 255);
    auto b = static_cast<std::uint8_t>((bPrime + m) * 255);
    return {r, g, b};
}

int main() {
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(CELL_SIZE*27, CELL_SIZE*15)), "SFML Window");
    // VSYNC IMPORTANT - otherwise 9000 updates a second and bad for shit
    window.setVerticalSyncEnabled(true);
    sf::Clock tickTimer = sf::Clock();

    while (window.isOpen()) {
        // Event handling
        while (const auto &event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // Erase everything from previous frame
        window.clear(sf::Color::White);
        // sf::Time time = sf::Time::Zero;
        // sf::RectangleShape rect = sf::RectangleShape(1, 1);

        double periodTime = (tickTimer.getElapsedTime().asMilliseconds()) / static_cast<double>(CYCLE_TIME)
                            * 2 * M_PI; // 2 seconds per cycle, mapped to values 0-2PI

        for (int yCount = 1; yCount <= 4; ++yCount) {
            for (int xCount = 1; xCount <= 8; ++xCount) {
                for (int i = 0; i < PRECISION; i++) {
                    sf::CircleShape trailHead = sf::CircleShape(MAX_SIZE);
                    // sf::CircleShape trailHead = sf::CircleShape(i / PRECISION * MAX_SIZE);
                    trailHead.setFillColor(hsvToRgb(i*360/PRECISION, 1, 1));
                    trailHead.setPosition(sf::Vector2f(
                        std::cos((periodTime + i / PRECISION * 2 * M_PI) * xCount) * CELL_SIZE + CELL_SIZE * xCount * 3,
                        std::sin((periodTime + i / PRECISION * 2 * M_PI) * yCount) * CELL_SIZE + CELL_SIZE * yCount *
                        3));
                    window.draw(trailHead);
                }
            }
        }


        // Display changes to window
        window.display();
    }
    return 0;
}

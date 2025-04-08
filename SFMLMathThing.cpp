//
// Created by dungewar on 4/5/25.
//

#include "SFMLMathThing.h"

#include <iostream>
#include <valarray>
#include <SFML/Graphics.hpp>
#include "font_data.h"

int cycleTime = 10000; // Time (in ms) per cycle

constexpr float CELL_SIZE = 150; // Size (width / height) of each parametric

constexpr float CELL_SPACING = 50; // Spacing in between each cell

constexpr float PRECISION = 600; // Amount of shapes per parametric used to simulate it

constexpr int COLUMNS = 9;

constexpr int ROWS = 5;

constexpr float MAX_SIZE = 5; // Maximum size of the pointer to the parametric equasion

constexpr float INIT_X_OFFSET = 150; // Offset to the start of the parametrics

constexpr float INIT_Y_OFFSET = 150; // Offset to the start of the parametrics

// Important: the state of the program, AKA what it's doing
// 0 is base rainbow state
// 1 is fading black state
// 2 is fading rainbow state
// 3 is regular black state
int state = 0;

auto trailHead = sf::CircleShape(MAX_SIZE, 6); // Things that draw the equasions, can be rect or circle

auto vertLineTrail = sf::RectangleShape({MAX_SIZE, 100000});

auto horizLineTrail = sf::RectangleShape({100000, MAX_SIZE});



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
        rPrime = c;
        gPrime = x;
        bPrime = 0;
    } else if (h >= 60 && h < 120) {
        rPrime = x;
        gPrime = c;
        bPrime = 0;
    } else if (h >= 120 && h < 180) {
        rPrime = 0;
        gPrime = c;
        bPrime = x;
    } else if (h >= 180 && h < 240) {
        rPrime = 0;
        gPrime = x;
        bPrime = c;
    } else if (h >= 240 && h < 300) {
        rPrime = x;
        gPrime = 0;
        bPrime = c;
    } else {
        rPrime = c;
        gPrime = 0;
        bPrime = x;
    }

    auto r = static_cast<std::uint8_t>((rPrime + m) * 255);
    auto g = static_cast<std::uint8_t>((gPrime + m) * 255);
    auto b = static_cast<std::uint8_t>((bPrime + m) * 255);
    return {r, g, b};
}

/**
 * Calculates a certain parametric equation
 * @param time The time, which acts like a base offset to the function, shifting everything over
 * @param yOscilations Amount of vertical oscillations per 2PI
 * @param xOscilations Amount of horizontal oscillations per 2PI
 * @return The position of the trail
 */
sf::Vector2f parametricEquasion(double time, int xOscilations, int yOscilations, double xOffset, double yOffset) {
    return {
        static_cast<float>(std::cos(time * 2 * M_PI * xOscilations) * CELL_SIZE / 2 + xOffset),
        static_cast<float>(std::sin(time * 2 * M_PI * yOscilations) * CELL_SIZE / 2 + yOffset)
    };
}

sf::Color decideOnColor(double x) {
    if (state % 2 == 0)
        return hsvToRgb(x, 1, 1);
    else
        return sf::Color::Black;
}

float stateSizeOfTrailHead(const int i) {
    int t = state % 8;
    if (t >= 2 && t <= 3) {
        // std::cout << MAX_SIZE * (static_cast<float>(i) / PRECISION) << std::endl;
        return MAX_SIZE * (static_cast<float>(i) / PRECISION);
    } else if (t >= 5 && t <= 6) {
        return i == 0 ? MAX_SIZE : 1;
    } else
        return MAX_SIZE;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(CELL_SIZE * 27, CELL_SIZE * 15)), "SFML Window");
    // VSYNC IMPORTANT - otherwise 9000 updates a second and bad for shit
    window.setVerticalSyncEnabled(true);
    sf::Clock tickTimer = sf::Clock();

    sf::Font inriaSansFont(resources_InriaSans_Regular_ttf, resources_InriaSans_Regular_ttf_len);

    // Initialize things that don't change so no repeat
    sf::Text instructions(inriaSansFont);
    instructions.setPosition({0, 0});
    instructions.setString("Hallo. Left and right arrows to switch modes, up and down to change speed, made by Olgierd Matusiewicz.");
    instructions.setOutlineColor(sf::Color::Black);
    instructions.setOutlineThickness(2);

    sf::Text circleLabel(inriaSansFont);
    circleLabel.setFillColor(sf::Color::White);
    circleLabel.setOutlineColor(sf::Color::Black);
    circleLabel.setOutlineThickness(2);

    double periodTime = 0;

    while (window.isOpen()) {
        // Event handling
        while (const auto &event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto *keyPressedEvent = event->getIf<sf::Event::KeyPressed>()) {
                switch (keyPressedEvent->code) {
                    case(sf::Keyboard::Key::Left):
                        if (state > 0) state--;
                        break;
                    case(sf::Keyboard::Key::Right):
                        if (state < 3) state++;
                        break;
                    case(sf::Keyboard::Key::Up):
                        cycleTime += 1000;
                        break;
                    case(sf::Keyboard::Key::Down):
                        if (cycleTime-1000 > 0)
                            cycleTime -= 1000;
                        break;
                    default:
                        break;
                }
            }
        }

        // Erase everything from previous frame
        window.clear(sf::Color::White);
        // sf::Time time = sf::Time::Zero;
        // sf::RectangleShape rect = sf::RectangleShape(1, 1);

        periodTime += static_cast<double>(tickTimer.restart().asMilliseconds()) / cycleTime;
        // tickTimer.start();
        // 2 seconds per cycle, mapped to values 0-2PI

        for (int yCount = 1; yCount <= ROWS; ++yCount) {
            // Draw the left side circles
            for (int i = 0; i < static_cast<int>(PRECISION); i++) {
                trailHead.setFillColor(decideOnColor(i * 360 / PRECISION));
                const sf::Vector2f &pos = parametricEquasion(periodTime + i / PRECISION, yCount, yCount, CELL_SIZE,
                                                             (CELL_SIZE + CELL_SPACING) * (yCount) + INIT_Y_OFFSET);
                trailHead.setPosition(pos);
                window.draw(trailHead);
            }
            circleLabel.setPosition({CELL_SIZE, (CELL_SIZE + CELL_SPACING) * (yCount) + INIT_Y_OFFSET});
            circleLabel.setString(std::to_string(yCount));
            window.draw(circleLabel);

            for (int xCount = 1; xCount <= COLUMNS; ++xCount) {
                // Draw the main middle parametrics
                for (int i = 0; i < PRECISION; i += 1) {
                    trailHead.setFillColor(decideOnColor(i * 360 / PRECISION));
                    trailHead.setRadius(stateSizeOfTrailHead(i));
                    trailHead.setPosition(parametricEquasion(
                        periodTime + i / PRECISION,
                        xCount,
                        yCount,
                        (CELL_SIZE + CELL_SPACING) * (xCount) + INIT_X_OFFSET,
                        (CELL_SIZE + CELL_SPACING) * (yCount) + INIT_Y_OFFSET));
                    window.draw(trailHead);
                }
                // We only want to do this once
                if (yCount == 1) {
                    // Draw the top circles
                    for (int i = 0; i < PRECISION; i++) {
                        trailHead.setFillColor(decideOnColor(i * 360 / PRECISION));
                        trailHead.setPosition(parametricEquasion(periodTime + i / PRECISION, xCount, xCount,
                                                                 (CELL_SIZE + CELL_SPACING) * (xCount) + INIT_X_OFFSET,
                                                                 CELL_SIZE));
                        window.draw(trailHead);
                    }
                    circleLabel.setPosition({(CELL_SIZE + CELL_SPACING) * (xCount) + INIT_X_OFFSET, CELL_SIZE});
                    circleLabel.setString(std::to_string(xCount));
                    window.draw(circleLabel);
                    // Draw the lines to connect top-bottom
                    vertLineTrail.setPosition(parametricEquasion(periodTime, xCount, xCount,
                                                                 (CELL_SIZE + CELL_SPACING) * (xCount) + INIT_X_OFFSET,
                                                                 CELL_SIZE));
                    vertLineTrail.setFillColor(decideOnColor(0));
                    window.draw(vertLineTrail);
                }
            }
            // Draw the lines to connect left-right
            horizLineTrail.setPosition(parametricEquasion(periodTime, yCount, yCount, CELL_SIZE,
                                                          (CELL_SIZE + CELL_SPACING) * (yCount) + INIT_Y_OFFSET));
            horizLineTrail.setFillColor(decideOnColor(0));
            window.draw(horizLineTrail);
        }


        window.draw(instructions);

        // Display changes to window
        window.display();
    }
    return 0;
}

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

using namespace sf;
using namespace std;

struct Planet {
    Vector2f pos;
    float r;
    float grav;
};

struct Ship {
    Vector2f pos;
    Vector2f speed;
    float r = 6.0f;
    float rotation = 0.0f;
    float fuel = 100.0f;
};

struct DeliveryZone {
    Vector2f pos;
    float r;
    bool completed = false;
};

struct Asteroid {
    size_t planetIndex;
    float orbitRadius;
    float orbitSpeed;
    float angle;
    float r;
};

enum class GameState {
    Flying,
    Crashed,
    Complete
};

class Game {
public:
    Game();

    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void reset();

    void BuildWorld();
    void resetFrameEffects();
    void rotateShip(float dt);
    void Gravitation(float dt);
    void applyThrust(float dt);
    void updatePlayer(float dt);
    void updateAsteroids(float dt);
    bool handlePlanetCrash();
    bool handleAsteroidCrash();
    bool handleDelivery();
    void completeDelivery(DeliveryZone& deliveryZone);

    Vector2f etMouseWorldPos() const;
    Vector2f getAsteroidPosition(const Asteroid& asteroid) const;
    bool isButtonPrest() const;
    bool isOffScreen() const;
    void updateTitle();

    void drawStars();
    void drawDeliveryZones();
    void drawPlanets();
    void drawAsteroids();
    void drawAcceleration();
    void drawTurnVents();
    void drawShip();
    void drawFuelBar();
    void drawCounter();
    void drawCompleteOverlay();
    void drawCursor();

    RenderWindow m_window;
    Clock m_clock;
    Font m_hudFont;
    vector<Vector2f> m_stars;
    vector<Planet> m_planets;
    vector<Asteroid> m_asteroids;
    Ship m_ship;
    vector<DeliveryZone> m_deliveryZones;
    size_t m_completedDeliveries = 0;
    bool m_hasHudFont = false;
    bool m_isAccelerating = false;
    float m_turningRate = 0.0f;
    float m_turnDirection = 0.0f;
    GameState m_state = GameState::Flying;
};

const unsigned WindowWidth = 1280;
const unsigned WindowHeight = 720;
const float WorldWidth = static_cast<float>(WindowWidth);
const float WorldHeight = static_cast<float>(WindowHeight);
const float Pi = 3.14159265358979323846f;
const float TwoPi = Pi * 2.0f;
const float MaxFuel = 100.0f;
const float CargoFuelReward = 45.0f;
const float PlanetGravityDensity = 10.56f;
const float FuelBurnRate = 16.0f;
const float TurnFuelBurnRate = 1.45f;
const float TurnSpeedRadians = 3.8f;
const float shipAcceleration = 138.0f;
const float MaxShipSpeed = 285.0f;
const float speedDicreaseRate = 0.994f;

VideoMode makeVideoMode(unsigned width, unsigned height)
{
    return VideoMode({width, height});
}

float lengthSquared(Vector2f value)
{
    return value.x * value.x + value.y * value.y;
}

float length(Vector2f value)
{
    return sqrt(lengthSquared(value));
}

Vector2f normal(Vector2f value)
{
    const float magnitude = length(value);
    if (magnitude <= 0.0001f) {
        return {};
    }

    return value / magnitude;
}

Vector2f Turn90Right(Vector2f value)
{
    return {-value.y, value.x};
}

float RadNormForm(float radians)
{
    while (radians > Pi) {
        radians -= TwoPi;
    }

    while (radians < -Pi) {
        radians += TwoPi;
    }

    return radians;
}

Vector2f directionFromAngle(float radians)
{
    return {cos(radians), sin(radians)};
}

float gravForRadius(float r)
{
    return PlanetGravityDensity * r * r * r;
}

Planet makePlanet(Vector2f pos, float r)
{
    return {pos, r, gravForRadius(r)};
}

Text makeHudText(const Font& font, const string& text, unsigned size)
{
    return Text(font, text, size);
}

Vector2f textBoundsPosition(const Text& text)
{
    const auto bounds = text.getLocalBounds();
    return bounds.position;
}

Vector2f textBoundsSize(const Text& text)
{
    const auto bounds = text.getLocalBounds();
    return bounds.size;
}

void centerText(Text& text, Vector2f center)
{
    const Vector2f boundsPosition = textBoundsPosition(text);
    const Vector2f boundsSize = textBoundsSize(text);
    text.setOrigin(boundsPosition + boundsSize * 0.5f);
    text.setPosition(center);
}

Game::Game()
    : m_window(makeVideoMode(WindowWidth, WindowHeight), "Space Dilivery Adventure")
{
    m_window.setFramerateLimit(120);
    m_window.setMouseCursorVisible(true);
    m_hasHudFont = m_hudFont.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
    BuildWorld();
    reset();
}

void Game::BuildWorld()
{
    m_stars = {
        {56.0f, 92.0f},    {184.0f, 640.0f},  {268.0f, 144.0f},
        {390.0f, 514.0f},  {512.0f, 88.0f},   {608.0f, 360.0f},
        {734.0f, 624.0f},  {820.0f, 182.0f},  {948.0f, 542.0f},
        {1044.0f, 84.0f},  {1136.0f, 316.0f}, {1210.0f, 664.0f},
        {72.0f, 390.0f},   {330.0f, 302.0f},  {704.0f, 74.0f},
        {1004.0f, 262.0f}, {1182.0f, 144.0f}, {1160.0f, 494.0f},
        {126.0f, 706.0f},  {474.0f, 56.0f},   {612.0f, 668.0f},
        {860.0f, 42.0f},   {918.0f, 704.0f},  {1098.0f, 586.0f},
    };

    m_planets = {
        makePlanet({220.0f, 210.0f}, 60.0f),
        makePlanet({480.0f, 520.0f}, 47.0f),
        makePlanet({710.0f, 260.0f}, 25.0f),
        makePlanet({930.0f, 470.0f}, 41.0f),
        makePlanet({1110.0f, 185.0f}, 34.0f),
        makePlanet({1080.0f, 590.0f}, 27.0f),
        makePlanet({610.0f, 620.0f}, 44.0f),
    };

    m_asteroids = {
        {0, 75.0f, 0.95f, 0.15f, 7.0f},
        {0, 75.0f, 0.95f, 2.45f, 13.0f},
        {0, 110.0f, -0.48f, 1.20f, 16.0f},
        {0, 110.0f, -0.48f, 4.60f, 9.0f},
        {2, 80.0f, -0.82f, 0.90f, 4.0f},
        {2, 80.0f, -0.82f, 3.85f, 7.0f},
        {3, 85.0f, 0.74f, 0.20f, 5.0f},
        {3, 85.0f, 0.74f, 2.10f, 9.0f},
        {3, 120.0f, -0.44f, 1.45f, 4.0f},
        {3, 118.0f, -0.44f, 3.95f, 5.0f},
        {3, 118.0f, -0.44f, 5.60f, 5.0f},
        {4, 88.0f, -0.62f, 0.40f, 5.0f},
        {4, 88.0f, -0.62f, 2.70f, 5.0f},
        {4, 90.0f, -0.62f, 4.95f, 5.0f},
    };

    m_deliveryZones = {
        {{286.0f, 650.0f}, 24.0f},
        {{596.0f, 112.0f}, 24.0f},
        {{680.0f, 380.0f}, 24.0f},
        {{1006.0f, 635.0f}, 24.0f},
        {{1190.0f, 382.0f}, 24.0f},
    };
}

void Game::run()
{
    while (m_window.isOpen()) {
        processEvents();

        const float dt = min(m_clock.restart().asSeconds(), 1.0f / 30.0f);
        update(dt);
        render();
    }
}

void Game::processEvents()
{
    while (const auto event = m_window.pollEvent()) {
        if (event->is<Event::Closed>()) {
            m_window.close();
        }

        if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
            if (keyPressed->code == Keyboard::Key::Escape) {
                m_window.close();
            }

            if (keyPressed->code == Keyboard::Key::R) {
                reset();
            }
        }
    }
}

void Game::update(float dt)
{
    resetFrameEffects();
    updateAsteroids(dt);

    if (m_state != GameState::Flying) {
        return;
    }

    rotateShip(dt);
    applyThrust(dt);
    Gravitation(dt);
    updatePlayer(dt);

    if (handlePlanetCrash()) {
        return;
    }

    if (handleAsteroidCrash()) {
        return;
    }

    if (handleDelivery()) {
        return;
    }

    if (isOffScreen()) {
        m_state = GameState::Crashed;
        updateTitle();
    }
}

void Game::render()
{
    m_window.clear(Color(5, 10, 15));

    drawStars();
    drawDeliveryZones();
    drawPlanets();
    drawAsteroids();
    drawAcceleration();
    drawTurnVents();
    drawShip();
    drawCursor();

    drawFuelBar();
    drawCounter();
    drawCompleteOverlay();

    m_window.display();
}

void Game::reset()
{
    m_ship.pos = {74.0f, 360.0f};
    m_ship.speed = {0.0f, 0.0f};
    m_ship.rotation = 0.0f;
    m_ship.fuel = MaxFuel;
    m_completedDeliveries = 0;
    for (DeliveryZone& deliveryZone : m_deliveryZones) {
        deliveryZone.completed = false;
    }
    m_isAccelerating = false;
    m_turningRate = 0.0f;
    m_turnDirection = 0.0f;
    m_state = GameState::Flying;
    m_clock.restart();
    updateTitle();
}

void Game::resetFrameEffects()
{
    m_isAccelerating = false;
    m_turningRate = 0.0f;
    m_turnDirection = 0.0f;
}
void Game::rotateShip(float dt)
{
    if (m_ship.fuel <= 0.0f) {
        return;
    }

    const Vector2f toMouse = etMouseWorldPos() - m_ship.pos;
    if (lengthSquared(toMouse) <= 0.0001f) {
        return;
    }

    const float targetRotation = atan2(toMouse.y, toMouse.x);
    const float delta = RadNormForm(targetRotation - m_ship.rotation);
    const float maxTurnThisFrame = TurnSpeedRadians * dt;
    float appliedTurn = clamp(delta, -maxTurnThisFrame, maxTurnThisFrame);
    if (abs(appliedTurn) <= 0.0001f) {
        return;
    }

    float fuelCost = abs(appliedTurn) * TurnFuelBurnRate;
    if (fuelCost > m_ship.fuel) {
        appliedTurn *= m_ship.fuel / fuelCost;
        fuelCost = m_ship.fuel;
    }

    m_ship.rotation = RadNormForm(m_ship.rotation + appliedTurn);
    m_ship.fuel = max(0.0f, m_ship.fuel - fuelCost);
    m_turnDirection = appliedTurn > 0.0f ? 1.0f : -1.0f;
    m_turningRate = clamp(abs(delta) / 1.05f, 0.12f, 1.0f);
}

void Game::Gravitation(float dt)
{
    for (const Planet& planet : m_planets) {
        const Vector2f toPlanet = planet.pos - m_ship.pos;
        const float safeDistanceSquared = max(lengthSquared(toPlanet), 1600.0f);
        const Vector2f direction = normal(toPlanet);
        const float acceleration = planet.grav / safeDistanceSquared;
        m_ship.speed += direction * acceleration * dt;
    }
}

void Game::applyThrust(float dt)
{
    if (!isButtonPrest() || m_ship.fuel <= 0.0f) {
        return;
    }

    m_ship.speed += directionFromAngle(m_ship.rotation) * shipAcceleration * dt;
    m_ship.fuel = max(0.0f, m_ship.fuel - FuelBurnRate * dt);
    m_isAccelerating = true;
}

void Game::updatePlayer(float dt)
{
    const float damping = pow(speedDicreaseRate, dt * 60.0f);
    m_ship.speed = m_ship.speed * damping;

    const float shipSpeed = length(m_ship.speed);
    if (shipSpeed > MaxShipSpeed) {
        m_ship.speed = normal(m_ship.speed) * MaxShipSpeed;
    }

    m_ship.pos += m_ship.speed * dt;
}

void Game::updateAsteroids(float dt)
{
    for (Asteroid& asteroid : m_asteroids) {
        asteroid.angle = RadNormForm(asteroid.angle + asteroid.orbitSpeed * dt);
    }
}

bool Game::handlePlanetCrash()
{
    for (const Planet& planet : m_planets) {
        const float crashDistance = planet.r + m_ship.r;
        if (length(m_ship.pos - planet.pos) <= crashDistance) {
            m_state = GameState::Crashed;
            updateTitle();
            return true;
        }
    }

    return false;
}

bool Game::handleAsteroidCrash()
{
    for (const Asteroid& asteroid : m_asteroids) {
        const float crashDistance = asteroid.r + m_ship.r;
        if (length(m_ship.pos - getAsteroidPosition(asteroid)) <= crashDistance) {
            m_state = GameState::Crashed;
            updateTitle();
            return true;
        }
    }

    return false;
}

bool Game::handleDelivery()
{
    for (DeliveryZone& deliveryZone : m_deliveryZones) {
        if (deliveryZone.completed) {
            continue;
        }

        const float deliveryDistance = deliveryZone.r + m_ship.r;
        if (length(m_ship.pos - deliveryZone.pos) <= deliveryDistance) {
            completeDelivery(deliveryZone);
            return true;
        }
    }

    return false;
}

void Game::completeDelivery(DeliveryZone& deliveryZone)
{
    deliveryZone.completed = true;
    ++m_completedDeliveries;
    m_ship.fuel = min(MaxFuel, m_ship.fuel + CargoFuelReward);

    if (m_completedDeliveries >= m_deliveryZones.size()) {
        m_state = GameState::Complete;
    }

    updateTitle();
}

Vector2f Game::etMouseWorldPos() const
{
    return m_window.mapPixelToCoords(Mouse::getPosition(m_window));
}

Vector2f Game::getAsteroidPosition(const Asteroid& asteroid) const
{
    if (asteroid.planetIndex >= m_planets.size()) {
        return {};
    }

    const Vector2f direction = directionFromAngle(asteroid.angle);
    return m_planets[asteroid.planetIndex].pos + direction * asteroid.orbitRadius;
}

bool Game::isButtonPrest() const
{
    return Mouse::isButtonPressed(Mouse::Button::Left);
}

bool Game::isOffScreen() const
{
    const float DistancetoWorld = 90.0f;
    return m_ship.pos.x < -DistancetoWorld || m_ship.pos.x > WorldWidth + DistancetoWorld || m_ship.pos.y < -DistancetoWorld || m_ship.pos.y > WorldHeight + DistancetoWorld;
}

void Game::updateTitle()
{
    string title = "Space Dilivery Adventure - playing " + to_string(m_completedDeliveries) + "/" + to_string(m_deliveryZones.size()) + " | R reset";

    if (m_state == GameState::Crashed) {
        title = "Space Dilivery Adventure - crashed | cargo " + to_string(m_completedDeliveries) + "/" + to_string(m_deliveryZones.size()) + " | R reset";
    } else if (m_state == GameState::Complete) {
        title = "Space Dilivery Adventure- complete | cargo " + to_string(m_completedDeliveries) + "/" + to_string(m_deliveryZones.size()) + " | R reset";
    }

    m_window.setTitle(title);
}

void Game::drawStars()
{
    CircleShape star(1.5f);
    star.setFillColor(Color(190, 200, 225, 150));

    for (Vector2f pos : m_stars) {
        star.setPosition(pos);
        m_window.draw(star);
    }
}

void Game::drawDeliveryZones()
{
    for (const DeliveryZone& deliveryZone : m_deliveryZones) {
        CircleShape zone(deliveryZone.r);
        zone.setOrigin({deliveryZone.r, deliveryZone.r});
        zone.setPosition(deliveryZone.pos);

        if (deliveryZone.completed) {
            zone.setFillColor(Color(70, 220, 150, 30));
            zone.setOutlineColor(Color(85, 250, 170));
        } else {
            zone.setFillColor(Color(100, 125, 150, 20));
            zone.setOutlineColor(Color(125, 150, 170, 125));
        }

        zone.setOutlineThickness(1.8f);
        m_window.draw(zone);
    }
}

void Game::drawPlanets()
{
    for (size_t planetNumber = 0; planetNumber < m_planets.size(); ++planetNumber) {
        const Planet& planet = m_planets[planetNumber];
        CircleShape Planetdraw(planet.r);
        Planetdraw.setOrigin({planet.r, planet.r});
        Planetdraw.setPosition(planet.pos);

        if (planetNumber == 0) {
            Planetdraw.setFillColor(Color(95, 165, 255));
        } else if (planetNumber == 1) {
            Planetdraw.setFillColor(Color(250, 190, 95));
        } else if (planetNumber == 2) {
            Planetdraw.setFillColor(Color(160, 225, 145));
        } else if (planetNumber == 3) {
            Planetdraw.setFillColor(Color(220, 130, 250));
        } else if (planetNumber == 4) {
            Planetdraw.setFillColor(Color(235, 115, 105));
        } else if (planetNumber == 5) {
            Planetdraw.setFillColor(Color(120, 210, 230));
        } else {
            Planetdraw.setFillColor(Color(230, 170, 110));
        }

        Planetdraw.setOutlineColor(Color(240, 245, 255));
        Planetdraw.setOutlineThickness(2.0f);
        m_window.draw(Planetdraw);
    }
}
void Game::drawAsteroids()
{
    for (const Asteroid& asteroid : m_asteroids) {
        const Vector2f pos = getAsteroidPosition(asteroid);
        const float r = asteroid.r;

        ConvexShape asteroid1(7);
        asteroid1.setPoint(0, pos + Vector2f{r * 0.95f, -r * 0.18f});
        asteroid1.setPoint(1, pos + Vector2f{r * 0.50f, r * 0.74f});
        asteroid1.setPoint(2, pos + Vector2f{-r * 0.20f, r * 0.92f});
        asteroid1.setPoint(3, pos + Vector2f{-r * 0.82f, r * 0.46f});
        asteroid1.setPoint(4, pos + Vector2f{-r * 0.72f, -r * 0.48f});
        asteroid1.setPoint(5, pos + Vector2f{-r * 0.12f, -r * 0.96f});
        asteroid1.setPoint(6, pos + Vector2f{r * 0.66f, -r * 0.58f});
        asteroid1.setFillColor(Color(150, 140, 130));
        asteroid1.setOutlineColor(Color(220, 215, 205, 120));
        asteroid1.setOutlineThickness(1.0f);
        m_window.draw(asteroid1);

        CircleShape asteroid2(r * 0.23f);
        asteroid2.setOrigin({r * 0.23f, r * 0.23f});
        asteroid2.setPosition(pos + Vector2f{-r * 0.22f, r * 0.08f});
        asteroid2.setFillColor(Color(70, 70, 65, 120));
        m_window.draw(asteroid2);
    }
}

void Game::drawAcceleration()
{
    if (!m_isAccelerating) {
        return;
    }

    const Vector2f direction = directionFromAngle(m_ship.rotation);
    const Vector2f side = Turn90Right(direction);
    const Vector2f back = m_ship.pos - direction * 13.5f;

    ConvexShape flame1(3);
    flame1.setPoint(0, back + side * 4.2f);
    flame1.setPoint(1, back - direction * 21.0f);
    flame1.setPoint(2, back - side * 4.2f);
    flame1.setFillColor(Color(255, 140, 60, 210));
    m_window.draw(flame1);

    ConvexShape flame2(3);
    flame2.setPoint(0, back + side * 2.1f);
    flame2.setPoint(1, back - direction * 12.0f);
    flame2.setPoint(2, back - side * 2.1f);
    flame2.setFillColor(Color(255, 230, 140, 230));
    m_window.draw(flame2);
}

void Game::drawTurnVents()
{
    if (m_turningRate <= 0.0f || m_turnDirection == 0.0f) {
        return;
    }

    const Vector2f direction = directionFromAngle(m_ship.rotation);
    const Vector2f rightSide = Turn90Right(direction);
    const Vector2f ventSide = m_turnDirection > 0.0f ? -rightSide : rightSide;
    const float length = 8.0f + 22.0f * m_turningRate;
    const float width = 2.2f + 3.2f * m_turningRate;
    const uint8_t alpha = static_cast<uint8_t>(80.0f + 150.0f * m_turningRate);
    const Vector2f vent_pos = m_ship.pos + ventSide * 9.5f - direction * 1.5f;

    ConvexShape drawTurn(3);
    drawTurn.setPoint(0, vent_pos + direction * width);
    drawTurn.setPoint(1, vent_pos + ventSide * length);
    drawTurn.setPoint(2, vent_pos - direction * width);
    drawTurn.setFillColor(Color(240, 245, 255, alpha));
    m_window.draw(drawTurn);

}

void Game::drawShip()
{
    const Vector2f direction = directionFromAngle(m_ship.rotation);
    const Vector2f side = Turn90Right(direction);

    ConvexShape part_white(8);
    part_white.setPoint(0, m_ship.pos + direction * 18.0f);
    part_white.setPoint(1, m_ship.pos + direction * 5.0f + side * 4.5f);
    part_white.setPoint(2, m_ship.pos - direction * 2.0f + side * 11.0f);
    part_white.setPoint(3, m_ship.pos - direction * 13.0f + side * 5.0f);
    part_white.setPoint(4, m_ship.pos - direction * 15.5f);
    part_white.setPoint(5, m_ship.pos - direction * 13.0f - side * 5.0f);
    part_white.setPoint(6, m_ship.pos - direction * 2.0f - side * 11.0f);
    part_white.setPoint(7, m_ship.pos + direction * 5.0f - side * 4.5f);
    part_white.setFillColor(m_state == GameState::Crashed ? Color(255, 95, 95) : Color(240, 245, 255));
    part_white.setOutlineColor(Color(20, 30, 45));
    part_white.setOutlineThickness(1.0f);
    m_window.draw(part_white);

    ConvexShape part_blue(4);
    part_blue.setPoint(0, m_ship.pos + direction * 9.5f);
    part_blue.setPoint(1, m_ship.pos + direction * 3.0f + side * 2.6f);
    part_blue.setPoint(2, m_ship.pos - direction * 1.5f);
    part_blue.setPoint(3, m_ship.pos + direction * 3.0f - side * 2.6f);
    part_blue.setFillColor(Color(80, 165, 220, 210));
    part_blue.setOutlineColor(Color(210, 235, 255, 170));
    part_blue.setOutlineThickness(0.7f);
    m_window.draw(part_blue);

    CircleShape part_back(2.2f);
    part_back.setOrigin({2.2f, 2.2f});
    part_back.setPosition(m_ship.pos - direction * 13.8f);
    part_back.setFillColor(Color(35, 45, 60));
    m_window.draw(part_back);
}

void Game::drawFuelBar()
{
    const float FrameWidth = 26.0f;
    const float FrameHeight = 230.0f;
    const float FillWidth = FrameWidth - 5.0f * 2.0f;
    const float FillMaxHeight = FrameHeight - 5.0f * 2.0f;
    const Vector2f framePosition{WorldWidth - 44.0f, 34.0f};
    const float fuelRatio = clamp(m_ship.fuel / MaxFuel, 0.0f, 1.0f);
    const float fillHeight = FillMaxHeight * fuelRatio;

    RectangleShape frame({FrameWidth, FrameHeight});
    frame.setPosition(framePosition);
    frame.setFillColor(Color(10, 15, 25, 210));
    frame.setOutlineColor(Color(210, 225, 245, 180));
    frame.setOutlineThickness(2.0f);
    m_window.draw(frame);

    RectangleShape fill({FillWidth, fillHeight});
    fill.setPosition({
        framePosition.x + 5.0f,
        framePosition.y + 5.0f + FillMaxHeight - fillHeight,
    });

    if (fuelRatio < 0.3f) {
        fill.setFillColor(Color(240, 90, 90));
    } else if (fuelRatio < 0.6f) {
        fill.setFillColor(Color(245, 200, 90));
    } else {
        fill.setFillColor(Color(90, 225, 150));
    }

    m_window.draw(fill);

}

void Game::drawCounter()
{
    const float PanelWidth = 230.0f;
    const float PanelHeight = 70.0f;
    const Vector2f panelPosition{24.0f, 24.0f};

    RectangleShape Rectangle({PanelWidth, PanelHeight});
    Rectangle.setPosition(panelPosition);
    Rectangle.setFillColor(Color(10, 15, 25, 205));
    Rectangle.setOutlineColor(Color(210, 225, 245, 130));
    Rectangle.setOutlineThickness(1.5f);
    m_window.draw(Rectangle);

    if (m_hasHudFont) {
        Text StatusText = makeHudText(
            m_hudFont,
            "Point " + to_string(m_completedDeliveries) + "/" + to_string(m_deliveryZones.size()),
            22);
        StatusText.setFillColor(Color(240, 245, 255));
        StatusText.setStyle(Text::Bold);
        centerText(StatusText, panelPosition + Vector2f{PanelWidth * 0.5f, PanelHeight * 0.5f});
        m_window.draw(StatusText);
    }
}

void Game::drawCompleteOverlay()
{
    if (m_state != GameState::Complete) {
        return;
    }

    RectangleShape shade({WorldWidth, WorldHeight});
    shade.setFillColor(Color(5, 10, 15, 165));
    m_window.draw(shade);

    RectangleShape rectangle({460.0f, 190.0f});
    rectangle.setOrigin({230.0f, 95.0f});
    rectangle.setPosition({WorldWidth * 0.5f, WorldHeight * 0.5f});
    rectangle.setFillColor(Color(10, 15, 25, 230));
    rectangle.setOutlineColor(Color(125, 255, 190, 220));
    rectangle.setOutlineThickness(2.5f);
    m_window.draw(rectangle);

    if (m_hasHudFont) {
        Text CompleteText = makeHudText(m_hudFont, "Complete " + to_string(m_completedDeliveries) + "/" + to_string(m_deliveryZones.size()), 46);
        CompleteText.setFillColor(Color(240, 245, 255));
        CompleteText.setStyle(Text::Bold);
        centerText(CompleteText, {WorldWidth * 0.5f, WorldHeight * 0.5f});
        m_window.draw(CompleteText);
    }
}

void Game::drawCursor()
{
    const Vector2f cursor = etMouseWorldPos();
    
    CircleShape circle(12.0f);
    circle.setOrigin({12.0f, 12.0f});
    circle.setPosition(cursor);
    circle.setFillColor(Color::Transparent);
    circle.setOutlineColor(Color(130, 230, 255, 160));
    circle.setOutlineThickness(1.5f);
    m_window.draw(circle);
}
 
int main()
{
    Game game;
    game.run();
    return 0;
}

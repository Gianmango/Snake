#include <SFML/Graphics.hpp>

#include <optional>
#include <random>
#include <string>
#include <vector>

namespace
{
constexpr int cellSize = 50;
constexpr int columns = 14;
constexpr int rows = 16;
constexpr sf::Vector2u windowSize{
    static_cast<unsigned int>(columns * cellSize),
    static_cast<unsigned int>(rows * cellSize)};
constexpr sf::Vector2i initialDirection{1, 0};
constexpr sf::Vector2i noDirection{0, 0};
constexpr sf::Vector2i up{0, -1};
constexpr sf::Vector2i down{0, 1};
constexpr sf::Vector2i left{-1, 0};
constexpr sf::Vector2i right{1, 0};

using Snake = std::vector<sf::Vector2i>;

void updateWindowTitle(sf::RenderWindow& window, bool alive, int score)
{
    std::string title = "Snake | Score: " + std::to_string(score);
    title += alive ? " | Arrows to move" : " | Press Enter to start";
    window.setTitle(title);
}

bool spawnApple(
    sf::CircleShape& apple,
    const Snake& snake,
    std::mt19937& randomEngine)
{
    std::vector<sf::Vector2i> freeCells;
    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < columns; ++x)
        {
            const sf::Vector2i cell{x, y};
            bool occupied = false;
            for (const sf::Vector2i segment : snake)
            {
                if (segment == cell)
                {
                    occupied = true;
                    break;
                }
            }

            if (!occupied)
                freeCells.push_back(cell);
        }
    }

    if (freeCells.empty())
        return false;

    std::uniform_int_distribution<std::size_t> pickCell(0, freeCells.size() - 1);
    const sf::Vector2i cell = freeCells[pickCell(randomEngine)];
    apple.setPosition({
        static_cast<float>(cell.x * cellSize + 3),
        static_cast<float>(cell.y * cellSize + 3)});
    return true;
}

void resetGame(
    Snake& snake,
    sf::Vector2i& direction,
    sf::CircleShape& apple,
    std::mt19937& randomEngine,
    int& score,
    bool& alive)
{
    snake = {{7, 8}, {6, 8}, {5, 8}};
    direction = initialDirection;
    score = 0;
    alive = spawnApple(apple, snake, randomEngine);
}

sf::Vector2i readDirection(sf::Vector2i currentDirection)
{
    sf::Vector2i requestedDirection = noDirection;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        requestedDirection = up;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        requestedDirection = down;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        requestedDirection = left;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        requestedDirection = right;

    if (requestedDirection != noDirection &&
        requestedDirection != sf::Vector2i{-currentDirection.x, -currentDirection.y})
    {
        return requestedDirection;
    }

    return currentDirection;
}

void drawBoard(sf::RenderWindow& window)
{
    sf::RectangleShape tile({static_cast<float>(cellSize), static_cast<float>(cellSize)});

    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < columns; ++x)
        {
            tile.setPosition({
                static_cast<float>(x * cellSize),
                static_cast<float>(y * cellSize)});
            tile.setFillColor((x + y) % 2 == 0
                                  ? sf::Color(50, 100, 50)
                                  : sf::Color(35, 75, 35));
            window.draw(tile);
        }
    }
}

void drawSnake(sf::RenderWindow& window, const Snake& snake)
{
    sf::RectangleShape segment({static_cast<float>(cellSize - 2),
                                static_cast<float>(cellSize - 2)});

    for (std::size_t i = 0; i < snake.size(); ++i)
    {
        const sf::Vector2i cell = snake[i];
        segment.setPosition({
            static_cast<float>(cell.x * cellSize + 1),
            static_cast<float>(cell.y * cellSize + 1)});
        segment.setFillColor(i == 0 ? sf::Color(0, 220, 0) : sf::Color(0, 160, 0));
        window.draw(segment);
    }
}
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(windowSize), "Snake");
    window.setFramerateLimit(60);

    std::mt19937 randomEngine(std::random_device{}());
    Snake snake;
    sf::Vector2i direction = initialDirection;
    sf::CircleShape apple(static_cast<float>(cellSize - 6));
    apple.setFillColor(sf::Color::Red);
    apple.setScale({0.5f, 0.5f});
    int score = 0;
    bool alive = false;
    sf::Clock movementClock;

    updateWindowTitle(window, alive, score);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (!alive && keyPressed->code == sf::Keyboard::Key::Enter)
                {
                    resetGame(snake, direction, apple, randomEngine, score, alive);
                    movementClock.restart();
                    updateWindowTitle(window, alive, score);
                }
            }
        }

        if (alive && movementClock.getElapsedTime() >= sf::milliseconds(130))
        {
            movementClock.restart();
            direction = readDirection(direction);

            const sf::Vector2i nextHead = snake.front() + direction;
            const bool eatingApple =
                nextHead.x == static_cast<int>(apple.getPosition().x) / cellSize &&
                nextHead.y == static_cast<int>(apple.getPosition().y) / cellSize;

            const std::size_t segmentsToCheck = eatingApple ? snake.size() : snake.size() - 1;
            bool collision = nextHead.x < 0 || nextHead.x >= columns ||
                             nextHead.y < 0 || nextHead.y >= rows;

            for (std::size_t i = 0; !collision && i < segmentsToCheck; ++i)
                collision = snake[i] == nextHead;

            if (collision)
            {
                alive = false;
                updateWindowTitle(window, alive, score);
            }
            else
            {
                snake.insert(snake.begin(), nextHead);
                if (eatingApple)
                {
                    score += 10;
                    if (!spawnApple(apple, snake, randomEngine))
                        alive = false;
                    updateWindowTitle(window, alive, score);
                }
                else
                {
                    snake.pop_back();
                }
            }
        }

        window.clear();
        drawBoard(window);
        if (!snake.empty())
            drawSnake(window, snake);
        if (alive)
            window.draw(apple);
        window.display();
    }
}

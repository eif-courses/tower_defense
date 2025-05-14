#include <SFML/Graphics.hpp>
#include <chrono>

class Animation {
public:
    Animation() = default;
    Animation(int x, int y, int w, int h, int frameCount, const sf::Texture &texture) {
        this->texture = texture;
        for (int i = 0; i < frameCount; ++i) {
            frames[i] = sf::IntRect({x + i * w, y}, {w, h});
        }
        this->nFrames = frameCount;
    }

    void ApplyToSprite(sf::Sprite &sprite) const {
        sprite.setTexture(texture);
        sprite.setTextureRect(frames[iFrame]);
    }

    void Update(float dt) {
        time += dt;
        while (time >= holdTime) {
            time -= holdTime;
            Advance();
        }
    }

private:
    int nFrames = 0;
    static constexpr float holdTime = 0.1f;
    sf::Texture texture;
    sf::IntRect frames[9]; // Increased to support 9 frames
    int iFrame = 0;
    float time = 0.0f;

    void Advance() {
        if (++iFrame >= nFrames) {
            iFrame = 0;
        }
    }
};

class Character {
private:
    enum class AnimationIndex {
        WalkingUp,
        WalkingRight,
        WalkingDown,
        WalkingLeft,
        Count
    };

public:
    explicit Character(const sf::Vector2f pos) : position(pos) {
        if (!texture.loadFromFile("character.png")) {
            // error handling...
        }

        // Create the sprite with the loaded texture
        sprite = new sf::Sprite(texture);

        // Define sprite size based on the image
        const int spriteWidth = 64;  // Adjust based on your actual sprite dimensions
        const int spriteHeight = 64; // Adjust based on your actual sprite dimensions

        // Create animations for each direction
        // Parameters: (x, y, width, height, frameCount, texture)
        animations[static_cast<int>(AnimationIndex::WalkingUp)] =
                   Animation(0, 0, spriteWidth, spriteHeight, 9, texture);

        // Second row: LEFT (according to your sprite sheet)
        animations[static_cast<int>(AnimationIndex::WalkingLeft)] =
            Animation(0, spriteHeight, spriteWidth, spriteHeight, 9, texture);

        // Third row: DOWN
        animations[static_cast<int>(AnimationIndex::WalkingDown)] =
            Animation(0, spriteHeight * 2, spriteWidth, spriteHeight, 9, texture);

        // Fourth row: RIGHT (according to your sprite sheet)
        animations[static_cast<int>(AnimationIndex::WalkingRight)] =
            Animation(0, spriteHeight * 3, spriteWidth, spriteHeight, 9, texture);

        // Set initial animation
        animations[static_cast<int>(AnimationIndex::WalkingDown)].ApplyToSprite(*sprite);
        sprite->setPosition(position);
        // Set initial animation
        animations[static_cast<int>(AnimationIndex::WalkingDown)].ApplyToSprite(*sprite);
        sprite->setPosition(position);

        // Optional: scale the sprite if needed
        // sprite->setScale(2.0f, 2.0f); // Double the size
    }

    // Destructor to clean up the sprite
    ~Character() {
        delete sprite;
    }

    // Disable copy constructor and assignment to handle the pointer properly
    Character(const Character&) = delete;
    Character& operator=(const Character&) = delete;

    void Draw(sf::RenderTarget &rt) {
        rt.draw(*sprite);
    }

    void SetDirection(const sf::Vector2f &dir) {
        // Store previous velocity to detect if we've stopped moving
        sf::Vector2f prevVelocity = velocity;

        velocity = dir * speed;

        // Only change animation if we're moving
        if (dir.x != 0.0f || dir.y != 0.0f) {
            // Determine the primary direction based on which component is larger
            if (std::abs(dir.x) > std::abs(dir.y)) {
                // Horizontal movement is dominant
                if (dir.x > 0.0f) {
                    currentAnimationIndex = AnimationIndex::WalkingRight;
                } else {
                    currentAnimationIndex = AnimationIndex::WalkingLeft;
                }
            } else {
                // Vertical movement is dominant
                if (dir.y < 0.0f) {
                    currentAnimationIndex = AnimationIndex::WalkingUp;
                } else {
                    currentAnimationIndex = AnimationIndex::WalkingDown;
                }
            }
        }
    }

    void Update(float dt) {
        position += velocity * dt;

        // Only update animation if moving
        if (velocity.x != 0.0f || velocity.y != 0.0f) {
            animations[static_cast<int>(currentAnimationIndex)].Update(dt);
        }

        animations[static_cast<int>(currentAnimationIndex)].ApplyToSprite(*sprite);
        sprite->setPosition(position);
    }

private:
    static constexpr float speed = 100.0f;
    sf::Texture texture;
    sf::Sprite* sprite = nullptr; // Use a pointer to create the sprite after loading the texture
    sf::Vector2f velocity = sf::Vector2f(0, 0);
    sf::Vector2f position;
    Animation animations[static_cast<int>(AnimationIndex::Count)];
    AnimationIndex currentAnimationIndex = AnimationIndex::WalkingDown;
};

int main() {
    auto window = sf::RenderWindow(sf::VideoMode({800u, 600u}), "Character Animation");
    window.setFramerateLimit(60);

    auto character = Character(sf::Vector2f(400, 300)); // Start in the middle of the screen

    auto tp = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        float dt; {
            const auto new_tp = std::chrono::steady_clock::now();
            dt = std::chrono::duration<float>(new_tp - tp).count();
            tp = new_tp;
        }

        sf::Vector2f dir{0, 0};

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            dir.x -= 1.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            dir.x += 1.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            dir.y -= 1.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            dir.y += 1.0f;
        }

        // Normalize diagonal movement
        if (dir.x != 0.0f && dir.y != 0.0f) {
            dir *= 0.7071f; // 1/sqrt(2)
        }

        character.SetDirection(dir);

        character.Update(dt);
        window.clear(sf::Color(200, 200, 200)); // Light gray background
        character.Draw(window);
        window.display();
    }
}
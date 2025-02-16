#include <SFML/Graphics.hpp>
#include <chrono>


class Animation {
public:
    Animation() = default;
    Animation(int x, int y, int w, int h, const sf::Texture &texture) {
        this->texture = texture;
        for (int i = 0; i < nFrames; ++i) {
            frames[i] = sf::IntRect({x + i * w, y}, {w, h});
        }
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
    static constexpr int nFrames = 4;
    static constexpr float holdTime = 0.1f;
    sf::Texture texture;
    sf::IntRect frames[nFrames];
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
        WalkingDown,
        WalkingLeft,
        WalkingRight,
        Count
    };

public:

    explicit Character(const sf::Vector2f pos) : position(pos), sprite(texture) {
        if (!texture.loadFromFile("hunter.jpg")) {
            // error...
        }
        sprite.setTextureRect(sf::IntRect({0,0},{159,184}));
        sprite.setTexture(texture);

        // TODO need to add all directions and some asset for it
        animations[static_cast<int>(AnimationIndex::WalkingUp)] = Animation(184,0,159,184, texture);
        animations[static_cast<int>(AnimationIndex::WalkingDown)] = Animation(184,0,159,184, texture);
        animations[static_cast<int>(AnimationIndex::WalkingLeft)] = Animation(184,0,159,184, texture);
        animations[static_cast<int>(AnimationIndex::WalkingRight)] = Animation(184,0,159,184, texture);
    }

    void Draw(sf::RenderTarget &rt) {
        rt.draw(sprite);
    }

    void SetDirection(const sf::Vector2f &dir) {
        velocity = dir * speed;
        if (dir.x > 0.0f) {
            currentAnimationIndex = AnimationIndex::WalkingRight;
        }
        else if (dir.x < 0.0f) {
            currentAnimationIndex = AnimationIndex::WalkingLeft;
        }
        else if (dir.y > 0.0f) {
            currentAnimationIndex = AnimationIndex::WalkingUp;
        }
        else if (dir.y < 0.0f) {
            currentAnimationIndex = AnimationIndex::WalkingDown;
        }
    }

    void Update(float dt) {
        position += velocity * dt;
        animations[static_cast<int>(currentAnimationIndex)].Update(dt);
        animations[static_cast<int>(currentAnimationIndex)].ApplyToSprite(sprite);
        sprite.setPosition(position);
    }

private:
    static constexpr float speed = 100.0f;
    sf::Sprite sprite;
    sf::Texture texture;
    sf::Vector2f velocity = sf::Vector2f(0, 0);
    sf::Vector2f position;
    Animation animations[static_cast<int>(AnimationIndex::Count)];
    AnimationIndex currentAnimationIndex = AnimationIndex::WalkingRight;
};

int main() {
    auto window = sf::RenderWindow(sf::VideoMode({1024u, 768u}), "CMake SFML Project");
    //window.setFramerateLimit(60);

    auto charcater = Character(sf::Vector2f(0, 0));

    auto tp = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
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

        if (isKeyPressed(sf::Keyboard::Key::Left)) {
            dir.x -= 1.0f;
        }
        if (isKeyPressed(sf::Keyboard::Key::Right)) {
            dir.x += 1.0f;
        }
        if (isKeyPressed(sf::Keyboard::Key::Up)) {
            dir.y -= 1.0f;
        }
        if (isKeyPressed(sf::Keyboard::Key::Down)) {
            dir.y += 1.0f;
        }
        charcater.SetDirection(dir);

        charcater.Update(dt);
        window.clear();
        charcater.Draw(window);
        window.display();
    }
}

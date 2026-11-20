// Particle fire in front of the stone wall: flames and sparks are drawn with additive blending,
// smoke with normal alpha blending, and a flickering point light illuminates the wall.

#include <algorithm>

#include "examples/common.h"
#include "imgui.h"

namespace {

const size_t kMaxParticles = 6000;

enum class Kind { Flame, Smoke, Spark };

struct Particle {
    Kind kind;
    Vec2 position;
    Vec2 velocity;
    float age;
    float life;
    float size;
    float seed;  // random per-particle value that varies the turbulence
};

struct Settings {
    float flameRate = 420.0f;  // particles per second
    float flameLife = 1.3f;
    float flameSize = 0.5f;
    float baseWidth = 1.2f;
    float riseSpeed = 2.2f;
    float turbulence = 1.5f;
    float wind = 0.0f;
    bool smoke = true;
    float smokeRate = 22.0f;
    bool sparks = true;
    float sparkRate = 14.0f;
    bool light = true;
    float lightIntensity = 1.7f;
    float flicker = 0.25f;
    bool showWall = true;
};

// Flame color by normalized age: white-yellow core, orange, deep red, then fades out.
Color flameColor(float t)
{
    struct Stop {
        float t;
        Color c;
    };
    static const Stop stops[] = {
        {0.00f, {1.00f, 0.85f, 0.50f}},
        {0.35f, {1.00f, 0.55f, 0.12f}},
        {0.70f, {0.85f, 0.25f, 0.04f}},
        {1.00f, {0.30f, 0.05f, 0.01f}},
    };
    for (size_t i = 1; i < sizeof(stops) / sizeof(stops[0]); ++i) {
        if (t <= stops[i].t) {
            const float k = (t - stops[i - 1].t) / (stops[i].t - stops[i - 1].t);
            const Color& a = stops[i - 1].c;
            const Color& b = stops[i].c;
            return {lerp(a.r, b.r, k), lerp(a.g, b.g, k), lerp(a.b, b.b, k)};
        }
    }
    return stops[3].c;
}

class Fire : public Example {
public:
    bool init() override
    {
        glow_ = makeGlowTexture();
        particles_.reserve(kMaxParticles);
        return wall_.init();
    }

    void update(const FrameInput& input) override
    {
        const World2D world(input);
        if (!placed_ || (input.mouseDown && input.mouseInView)) {
            base_ = placed_ ? world.mouse(input) : Vec2{world.width * 0.5f, world.height * 0.18f};
            placed_ = true;
        }

        const float dt = input.dt;
        emit(Kind::Flame, s_.flameRate, dt, flameAccumulator_);
        if (s_.smoke)
            emit(Kind::Smoke, s_.smokeRate, dt, smokeAccumulator_);
        if (s_.sparks)
            emit(Kind::Spark, s_.sparkRate, dt, sparkAccumulator_);

        for (Particle& p : particles_) {
            p.age += dt;
            // Turbulence: a sideways push that changes with height, time and the particle's seed.
            const float swirl = std::sin(p.seed * 17.0f + input.time * 5.0f + p.position.y * 2.3f) * s_.turbulence;
            switch (p.kind) {
            case Kind::Flame:
                p.velocity.y += 1.2f * dt;                                // buoyancy
                p.velocity.x += (base_.x - p.position.x) * 2.5f * dt;     // pull into a tongue shape
                p.velocity.x += (swirl * 2.5f + s_.wind) * dt;
                break;
            case Kind::Smoke:
                p.velocity.y += 0.15f * dt;
                p.velocity.x += (swirl * 0.4f + s_.wind * 1.5f) * dt;
                break;
            case Kind::Spark:
                p.velocity.y -= 0.6f * dt;                                // sparks are heavier than flames
                p.velocity.x += (swirl * 2.5f + s_.wind) * dt;
                break;
            }
            p.velocity.x *= 1.0f - 0.8f * dt;  // air drag
            p.position.x += p.velocity.x * dt;
            p.position.y += p.velocity.y * dt;
        }
        particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
                                        [](const Particle& p) { return p.age >= p.life; }),
                         particles_.end());

        // Flicker: a few incommensurate sine waves plus a little noise.
        const float t = input.time;
        const float wave = 0.5f * std::sin(t * 11.0f) + 0.3f * std::sin(t * 23.7f + 1.3f) + 0.2f * std::sin(t * 4.1f);
        flicker_ = 1.0f + s_.flicker * (0.7f * wave + 0.3f * random_.range(-1.0f, 1.0f));
        lightOffset_ = {0.08f * std::sin(t * 7.3f), 0.06f * std::sin(t * 9.1f)};
    }

    void render(const FrameInput& input) override
    {
        const World2D world(input);
        world.applyProjection();

        if (s_.showWall) {
            std::vector<Light> lights;
            if (s_.light) {
                Light light;
                light.position = {base_.x + lightOffset_.x, base_.y + 1.5f + lightOffset_.y, 2.2f};
                light.color = scale({1.0f, 0.5f, 0.18f}, s_.lightIntensity * flicker_);
                light.radius = 9.0f;
                lights.push_back(light);
            }
            Material material;
            material.ambient = {0.03f, 0.03f, 0.04f};
            material.specular = 0.3f;
            material.shininess = 16.0f;
            wall_.draw(world, lights, material);
        } else {
            glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glow_.bind(0);

        // Smoke first: it is darker than the background and uses normal alpha blending.
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        for (const Particle& p : particles_) {
            if (p.kind != Kind::Smoke)
                continue;
            const float t = p.age / p.life;
            const float alpha = 0.32f * std::min(t * 5.0f, 1.0f) * (1.0f - t);
            emitSprite(p.position.x, p.position.y, p.size * (0.6f + 1.8f * t), 0.12f, 0.11f, 0.10f, alpha);
        }
        glEnd();

        // Flames and sparks add light, so overlapping particles brighten toward white.
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBegin(GL_QUADS);
        for (const Particle& p : particles_) {
            const float t = p.age / p.life;
            if (p.kind == Kind::Flame) {
                const Color c = flameColor(t);
                // Fade in quickly so the base isn't a solid white blob, then fade out with age.
                const float alpha = std::min(t * 6.0f, 1.0f) * (1.0f - t * t) * 0.3f;
                emitSprite(p.position.x, p.position.y, p.size * (1.0f - 0.75f * t), c.r, c.g, c.b, alpha);
            } else if (p.kind == Kind::Spark) {
                const float alpha = 1.0f - t;
                emitSprite(p.position.x, p.position.y, p.size, 1.0f, 0.65f + 0.3f * (1.0f - t), 0.25f, alpha);
            }
        }
        glEnd();

        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
    }

    void drawUi() override
    {
        ImGui::TextDisabled("Click or drag on the scene to move the fire.");
        ImGui::Text("Particles: %d", static_cast<int>(particles_.size()));
        ImGui::SeparatorText("Flames");
        ImGui::SliderFloat("Rate", &s_.flameRate, 0.0f, 800.0f, "%.0f / s");
        ImGui::SliderFloat("Life", &s_.flameLife, 0.3f, 2.5f, "%.2f s");
        ImGui::SliderFloat("Size", &s_.flameSize, 0.3f, 3.0f);
        ImGui::SliderFloat("Base width", &s_.baseWidth, 0.1f, 4.0f);
        ImGui::SliderFloat("Rise speed", &s_.riseSpeed, 0.2f, 4.0f);
        ImGui::SliderFloat("Turbulence", &s_.turbulence, 0.0f, 4.0f);
        ImGui::SliderFloat("Wind", &s_.wind, -3.0f, 3.0f);
        ImGui::SeparatorText("Smoke and sparks");
        ImGui::Checkbox("Smoke", &s_.smoke);
        ImGui::SameLine();
        ImGui::SliderFloat("##smokeRate", &s_.smokeRate, 0.0f, 80.0f, "%.0f / s");
        ImGui::Checkbox("Sparks", &s_.sparks);
        ImGui::SameLine();
        ImGui::SliderFloat("##sparkRate", &s_.sparkRate, 0.0f, 80.0f, "%.0f / s");
        ImGui::SeparatorText("Light");
        ImGui::Checkbox("Fire lights the wall", &s_.light);
        ImGui::SliderFloat("Intensity", &s_.lightIntensity, 0.0f, 6.0f);
        ImGui::SliderFloat("Flicker", &s_.flicker, 0.0f, 0.8f);
        ImGui::Checkbox("Show wall", &s_.showWall);
        if (ImGui::Button("Reset"))
            s_ = Settings();
    }

private:
    // Emits rate * dt particles, carrying the fractional remainder to the next frame.
    void emit(Kind kind, float rate, float dt, float& accumulator)
    {
        accumulator += rate * dt;
        while (accumulator >= 1.0f) {
            accumulator -= 1.0f;
            if (particles_.size() >= kMaxParticles)
                continue;

            Particle p;
            p.kind = kind;
            p.age = 0;
            p.seed = random_.unit();
            switch (kind) {
            case Kind::Flame:
                p.position = {base_.x + random_.range(-0.5f, 0.5f) * s_.baseWidth, base_.y + random_.range(-0.1f, 0.1f)};
                p.velocity = {random_.range(-0.3f, 0.3f), s_.riseSpeed * random_.range(0.6f, 1.2f)};
                p.life = s_.flameLife * random_.range(0.6f, 1.2f);
                p.size = s_.flameSize * random_.range(0.7f, 1.3f);
                break;
            case Kind::Smoke:
                p.position = {base_.x + random_.range(-0.3f, 0.3f), base_.y + s_.riseSpeed * s_.flameLife * 0.9f};
                p.velocity = {random_.range(-0.2f, 0.2f), random_.range(0.5f, 0.9f)};
                p.life = random_.range(2.5f, 4.0f);
                p.size = random_.range(1.0f, 1.6f);
                break;
            case Kind::Spark:
                p.position = {base_.x + random_.range(-0.4f, 0.4f) * s_.baseWidth, base_.y + random_.range(0.0f, 0.4f)};
                p.velocity = {random_.range(-1.2f, 1.2f), random_.range(2.0f, 4.0f)};
                p.life = random_.range(0.8f, 2.0f);
                p.size = random_.range(0.07f, 0.14f);
                break;
            }
            particles_.push_back(p);
        }
    }

    LitWall wall_;
    Texture glow_;
    Random random_;
    std::vector<Particle> particles_;
    Vec2 base_;
    bool placed_ = false;
    float flameAccumulator_ = 0;
    float smokeAccumulator_ = 0;
    float sparkAccumulator_ = 0;
    float flicker_ = 1.0f;
    Vec2 lightOffset_;
    Settings s_;
};

} // namespace

std::unique_ptr<Example> createFire()
{
    return std::unique_ptr<Example>(new Fire());
}

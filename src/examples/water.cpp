// Interactive water surface: a height-field wave simulation on the CPU, uploaded every frame as a
// slope texture. The fragment shader uses the slopes to refract the floor and add highlights.

#include <algorithm>

#include "assets.h"
#include "examples/common.h"
#include "imgui.h"

namespace {

const int kGridHeight = 200;          // simulation cells vertically; width follows the aspect ratio
const float kStepRate = 60.0f;        // simulation steps per second, independent of the frame rate
const float kMaxEncodedSlope = 2.0f;  // slope range stored in the 8-bit texture

struct Settings {
    bool rain = true;
    float rainRate = 5.0f;  // drops per second
    float dropRadius = 5.0f;
    float dropStrength = 1.0f;
    float damping = 0.985f;
    float waveSlope = 3.0f;
    float refraction = 0.06f;
    float waterColor[3] = {0.05f, 0.25f, 0.3f};
    float depthTint = 0.3f;
    float specular = 1.2f;
};

class Water : public Example {
public:
    bool init() override
    {
        if (!shader_.load(assetPath("shaders/water.vert"), assetPath("shaders/water.frag")))
            return false;
        floor_ = Texture::fromBmp(assetPath("textures/stone_wall.bmp"));
        return bool(floor_);
    }

    void update(const FrameInput& input) override
    {
        const int width = std::max(16, int(kGridHeight * input.aspect() + 0.5f));
        if (width != width_)
            resize(width);

        if (input.mouseInView && input.mouseDown) {
            const float x = input.mouseX * (width_ - 1);
            const float y = input.mouseY * (kGridHeight - 1);
            // A click makes one big drop; dragging leaves a trail of small ones.
            drop(x, y, s_.dropRadius, s_.dropStrength * (input.mouseClicked ? 1.5f : 0.25f));
        }

        if (s_.rain) {
            rainAccumulator_ += s_.rainRate * input.dt;
            while (rainAccumulator_ >= 1.0f) {
                rainAccumulator_ -= 1.0f;
                drop(random_.range(0.0f, float(width_ - 1)), random_.range(0.0f, float(kGridHeight - 1)),
                     s_.dropRadius * random_.range(0.5f, 1.0f), s_.dropStrength * random_.range(0.4f, 1.0f));
            }
        }

        stepAccumulator_ += input.dt * kStepRate;
        for (int i = 0; i < 4 && stepAccumulator_ >= 1.0f; ++i) {
            step();
            stepAccumulator_ -= 1.0f;
        }
        stepAccumulator_ = std::min(stepAccumulator_, 1.0f);  // drop steps we couldn't keep up with
        uploadSlopes();
    }

    void render(const FrameInput& input) override
    {
        const World2D world(input);
        world.applyProjection();

        const float light[3] = {0.45f, 0.55f, 0.70f};
        const float len = std::sqrt(light[0] * light[0] + light[1] * light[1] + light[2] * light[2]);

        shader_.use();
        glUniform1i(shader_.uniform("uFloor"), 0);
        glUniform1i(shader_.uniform("uSlopes"), 1);
        glUniform2f(shader_.uniform("uWorldSize"), world.width, world.height);
        glUniform1f(shader_.uniform("uTileSize"), 5.0f);
        glUniform1f(shader_.uniform("uSlopeScale"), kMaxEncodedSlope * s_.waveSlope);
        glUniform1f(shader_.uniform("uRefraction"), s_.refraction);
        glUniform3f(shader_.uniform("uWaterColor"), s_.waterColor[0], s_.waterColor[1], s_.waterColor[2]);
        glUniform1f(shader_.uniform("uDepthTint"), s_.depthTint);
        glUniform1f(shader_.uniform("uSpecular"), s_.specular);
        glUniform3f(shader_.uniform("uLightDirection"), light[0] / len, light[1] / len, light[2] / len);
        slopes_.bind(1);
        floor_.bind(0);

        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(world.width, 0);
        glVertex2f(world.width, world.height);
        glVertex2f(0, world.height);
        glEnd();
        Shader::unuse();
    }

    void drawUi() override
    {
        ImGui::TextDisabled("Click for a drop, drag to stir the water.");
        ImGui::SeparatorText("Drops");
        ImGui::Checkbox("Rain", &s_.rain);
        ImGui::SameLine();
        ImGui::SliderFloat("##rainRate", &s_.rainRate, 0.5f, 40.0f, "%.1f drops / s");
        ImGui::SliderFloat("Radius", &s_.dropRadius, 1.5f, 15.0f, "%.1f cells");
        ImGui::SliderFloat("Strength", &s_.dropStrength, 0.1f, 3.0f);
        ImGui::SeparatorText("Simulation");
        ImGui::SliderFloat("Damping", &s_.damping, 0.95f, 0.999f, "%.3f");
        if (ImGui::Button("Calm the water")) {
            std::fill(current_.begin(), current_.end(), 0.0f);
            std::fill(previous_.begin(), previous_.end(), 0.0f);
        }
        ImGui::SeparatorText("Look");
        ImGui::SliderFloat("Wave slope", &s_.waveSlope, 0.5f, 8.0f);
        ImGui::SliderFloat("Refraction", &s_.refraction, 0.0f, 0.2f);
        ImGui::ColorEdit3("Water color", s_.waterColor);
        ImGui::SliderFloat("Depth tint", &s_.depthTint, 0.0f, 1.0f);
        ImGui::SliderFloat("Specular", &s_.specular, 0.0f, 3.0f);
        if (ImGui::Button("Reset"))
            s_ = Settings();
    }

private:
    void resize(int width)
    {
        width_ = width;
        const size_t cells = static_cast<size_t>(width_) * kGridHeight;
        current_.assign(cells, 0.0f);
        previous_.assign(cells, 0.0f);
        pixels_.assign(cells * 3, 128);
        slopes_ = Texture::create(width_, kGridHeight, GL_RGB, pixels_.data(), GL_CLAMP_TO_EDGE);
    }

    // Classic two-buffer wave equation: each cell moves toward the average of its neighbours,
    // overshoots by its previous height, and loses a little energy.
    void step()
    {
        const int w = width_;
        for (int y = 1; y < kGridHeight - 1; ++y) {
            for (int x = 1; x < w - 1; ++x) {
                const int i = y * w + x;
                const float next = (current_[i - 1] + current_[i + 1] + current_[i - w] + current_[i + w]) * 0.5f -
                                   previous_[i];
                previous_[i] = next * s_.damping;
            }
        }
        std::swap(current_, previous_);
    }

    // Pushes the surface down with a smooth cosine-shaped dent.
    void drop(float cx, float cy, float radius, float strength)
    {
        const int x0 = std::max(1, int(cx - radius)), x1 = std::min(width_ - 2, int(cx + radius) + 1);
        const int y0 = std::max(1, int(cy - radius)), y1 = std::min(kGridHeight - 2, int(cy + radius) + 1);
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                const float d = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
                if (d < radius)
                    current_[y * width_ + x] -= strength * 0.5f * (std::cos(d / radius * kPi) + 1.0f);
            }
        }
    }

    // Encodes the surface slope (central differences) into the R and G channels.
    void uploadSlopes()
    {
        const int w = width_;
        for (int y = 0; y < kGridHeight; ++y) {
            for (int x = 0; x < w; ++x) {
                const int i = y * w + x;
                float sx = 0, sy = 0;
                if (x > 0 && x < w - 1)
                    sx = (current_[i - 1] - current_[i + 1]) * 0.5f;
                if (y > 0 && y < kGridHeight - 1)
                    sy = (current_[i - w] - current_[i + w]) * 0.5f;
                pixels_[i * 3 + 0] = static_cast<unsigned char>(clamp01(0.5f + 0.5f * sx / kMaxEncodedSlope) * 255.0f);
                pixels_[i * 3 + 1] = static_cast<unsigned char>(clamp01(0.5f + 0.5f * sy / kMaxEncodedSlope) * 255.0f);
            }
        }
        slopes_.update(pixels_.data());
    }

    Shader shader_;
    Texture floor_;
    Texture slopes_;
    Random random_;
    int width_ = 0;
    std::vector<float> current_;
    std::vector<float> previous_;
    std::vector<unsigned char> pixels_;
    float stepAccumulator_ = 0;
    float rainAccumulator_ = 0;
    Settings s_;
};

} // namespace

std::unique_ptr<Example> createWater()
{
    return std::unique_ptr<Example>(new Water());
}

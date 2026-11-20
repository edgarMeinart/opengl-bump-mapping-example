// Several colored point lights moving over a normal-mapped wall, evaluated in one shader pass.

#include <algorithm>

#include "examples/common.h"
#include "imgui.h"

namespace {

const int kMaxMovingLights = kMaxLights - 1;  // one slot is kept for the mouse light

struct Settings {
    int count = 4;
    float speed = 0.6f;
    float height = 1.2f;
    float radius = 6.0f;
    float intensity = 1.4f;
    float saturation = 0.75f;
    bool mouseLight = true;
    float mouseColor[3] = {1.0f, 0.92f, 0.8f};
    float mouseIntensity = 1.2f;
    bool showMarkers = true;
    float ambient = 0.04f;
    float specular = 0.5f;
    float shininess = 32.0f;
};

class DynamicLights : public Example {
public:
    bool init() override
    {
        glow_ = makeGlowTexture();
        return wall_.init();
    }

    void update(const FrameInput& input) override
    {
        const World2D world(input);
        lights_.clear();

        // Each light follows its own Lissajous curve; frequencies differ so the paths never sync up.
        for (int i = 0; i < s_.count; ++i) {
            const float phase = float(i) / float(s_.count) * 2.0f * kPi;
            const float t = input.time * s_.speed;
            Light light;
            light.position.x = world.width * (0.5f + 0.42f * std::sin(t * (0.7f + 0.13f * i) + phase));
            light.position.y = world.height * (0.5f + 0.40f * std::sin(t * (1.1f + 0.07f * i) + phase * 2.0f));
            light.position.z = s_.height;
            light.color = scale(hsvToRgb(float(i) / float(s_.count), s_.saturation, 1.0f), s_.intensity);
            light.radius = s_.radius;
            lights_.push_back(light);
        }

        if (s_.mouseLight && input.mouseInView) {
            const Vec2 mouse = world.mouse(input);
            Light light;
            light.position = {mouse.x, mouse.y, s_.height};
            light.color = scale({s_.mouseColor[0], s_.mouseColor[1], s_.mouseColor[2]}, s_.mouseIntensity);
            light.radius = s_.radius;
            lights_.push_back(light);
        }
    }

    void render(const FrameInput& input) override
    {
        const World2D world(input);
        world.applyProjection();

        Material material;
        material.ambient = {s_.ambient, s_.ambient, s_.ambient};
        material.specular = s_.specular;
        material.shininess = s_.shininess;
        wall_.draw(world, lights_, material);

        if (s_.showMarkers) {
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glow_.bind(0);
            glBegin(GL_QUADS);
            for (const Light& light : lights_) {
                const float k = 1.0f / std::max(std::max(light.color.r, light.color.g), std::max(light.color.b, 0.001f));
                emitSprite(light.position.x, light.position.y, 0.7f, light.color.r * k, light.color.g * k,
                           light.color.b * k, 0.9f);
            }
            glEnd();
            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
        }
    }

    void drawUi() override
    {
        ImGui::TextDisabled("Move the mouse over the wall to carry a light.");
        ImGui::SeparatorText("Moving lights");
        ImGui::SliderInt("Count", &s_.count, 0, kMaxMovingLights);
        ImGui::SliderFloat("Speed", &s_.speed, 0.0f, 3.0f);
        ImGui::SliderFloat("Intensity", &s_.intensity, 0.0f, 4.0f);
        ImGui::SliderFloat("Saturation", &s_.saturation, 0.0f, 1.0f);
        ImGui::SeparatorText("Mouse light");
        ImGui::Checkbox("Enabled", &s_.mouseLight);
        ImGui::ColorEdit3("Color", s_.mouseColor);
        ImGui::SliderFloat("Mouse intensity", &s_.mouseIntensity, 0.0f, 4.0f);
        ImGui::SeparatorText("All lights");
        ImGui::SliderFloat("Height", &s_.height, 0.1f, 5.0f);
        ImGui::SliderFloat("Radius", &s_.radius, 1.0f, 20.0f);
        ImGui::Checkbox("Show markers", &s_.showMarkers);
        ImGui::SeparatorText("Wall");
        ImGui::SliderFloat("Ambient", &s_.ambient, 0.0f, 0.5f);
        ImGui::SliderFloat("Specular", &s_.specular, 0.0f, 2.0f);
        ImGui::SliderFloat("Shininess", &s_.shininess, 1.0f, 128.0f, "%.0f", ImGuiSliderFlags_Logarithmic);
        if (ImGui::Button("Reset"))
            s_ = Settings();
    }

private:
    LitWall wall_;
    Texture glow_;
    std::vector<Light> lights_;
    Settings s_;
};

} // namespace

std::unique_ptr<Example> createDynamicLights()
{
    return std::unique_ptr<Example>(new DynamicLights());
}

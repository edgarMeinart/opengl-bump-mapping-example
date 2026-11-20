// A flashlight held in front of a dark wall, aimed at the mouse cursor.

#include "examples/common.h"
#include "imgui.h"

namespace {

struct Settings {
    float coneAngle = 18.0f;  // degrees from the beam axis to the edge
    float softness = 0.35f;   // fraction of the cone used for the soft edge
    float height = 4.0f;      // distance of the flashlight from the wall
    float range = 25.0f;
    float intensity = 2.2f;
    float color[3] = {1.0f, 0.93f, 0.78f};
    bool autoSweep = false;
    bool roomLight = false;
    float ambient = 0.02f;
};

class Spotlight : public Example {
public:
    bool init() override
    {
        glow_ = makeGlowTexture();
        return wall_.init();
    }

    void update(const FrameInput& input) override
    {
        const World2D world(input);

        // Target point on the wall: the mouse, or a slow figure-eight sweep.
        if (s_.autoSweep || !input.mouseInView) {
            if (s_.autoSweep || !hasTarget_) {
                const float t = input.time * 0.5f;
                target_ = {world.width * (0.5f + 0.38f * std::sin(t)), world.height * (0.5f + 0.3f * std::sin(2.0f * t))};
            }
        } else {
            target_ = world.mouse(input);
            hasTarget_ = true;
        }

        // The flashlight is held at the bottom centre of the view, in front of the wall.
        origin_ = {world.width * 0.5f, world.height * 0.08f, s_.height};
        Vec3 dir = {target_.x - origin_.x, target_.y - origin_.y, -origin_.z};
        const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        dir = {dir.x / len, dir.y / len, dir.z / len};

        const float outer = s_.coneAngle * kPi / 180.0f;
        const float inner = outer * (1.0f - s_.softness);

        lights_.clear();
        Light spot;
        spot.position = origin_;
        spot.color = scale({s_.color[0], s_.color[1], s_.color[2]}, s_.intensity);
        spot.radius = s_.range;
        spot.spot = true;
        spot.direction = dir;
        spot.innerCos = std::cos(inner);
        spot.outerCos = std::cos(outer);
        lights_.push_back(spot);

        if (s_.roomLight) {
            Light lamp;
            lamp.position = {world.width * 0.5f, world.height * 1.1f, 3.0f};
            lamp.color = {0.25f, 0.3f, 0.45f};
            lamp.radius = 14.0f;
            lights_.push_back(lamp);
        }
    }

    void render(const FrameInput& input) override
    {
        const World2D world(input);
        world.applyProjection();

        Material material;
        material.ambient = {s_.ambient, s_.ambient, s_.ambient * 1.2f};
        material.specular = 0.6f;
        material.shininess = 40.0f;
        wall_.draw(world, lights_, material);

        // Small glow where the flashlight is held.
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glow_.bind(0);
        glBegin(GL_QUADS);
        emitSprite(origin_.x, origin_.y, 0.8f, s_.color[0], s_.color[1], s_.color[2], 0.8f);
        glEnd();
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
    }

    void drawUi() override
    {
        ImGui::TextDisabled("Point the mouse at the wall to aim the flashlight.");
        ImGui::SeparatorText("Beam");
        ImGui::SliderFloat("Cone angle", &s_.coneAngle, 3.0f, 60.0f, "%.0f deg");
        ImGui::SliderFloat("Edge softness", &s_.softness, 0.0f, 1.0f);
        ImGui::SliderFloat("Distance to wall", &s_.height, 0.5f, 10.0f);
        ImGui::SliderFloat("Range", &s_.range, 5.0f, 50.0f);
        ImGui::SliderFloat("Intensity", &s_.intensity, 0.0f, 5.0f);
        ImGui::ColorEdit3("Color", s_.color);
        ImGui::Checkbox("Auto sweep", &s_.autoSweep);
        ImGui::SeparatorText("Room");
        ImGui::Checkbox("Dim ceiling light", &s_.roomLight);
        ImGui::SliderFloat("Ambient", &s_.ambient, 0.0f, 0.3f);
        if (ImGui::Button("Reset"))
            s_ = Settings();
    }

private:
    LitWall wall_;
    Texture glow_;
    std::vector<Light> lights_;
    Vec3 origin_;
    Vec2 target_;
    bool hasTarget_ = false;
    Settings s_;
};

} // namespace

std::unique_ptr<Example> createSpotlight()
{
    return std::unique_ptr<Example>(new Spotlight());
}

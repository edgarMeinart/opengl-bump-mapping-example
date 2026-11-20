// The original school project: one quad, three point lights, tangent-space bump mapping.
// Each light is a separate pass; passes after the first are added with GL_ONE, GL_ONE blending.

#include <cstdio>

#include "assets.h"
#include "examples/common.h"
#include "imgui.h"

namespace {

// The scene is a 10x10 area with the quad centred on (5, 5); lights are given in that space.
const float kSceneSize = 10.0f;
const float kQuadHalfSize = 5.0f;

struct BumpLight {
    bool enabled;
    float position[3];
};

// Defaults are the values of the original program.
struct Settings {
    BumpLight lights[3] = {
        {true, {8.0f, 4.0f, 0.5f}},
        {true, {4.0f, 4.0f, 0.5f}},
        {true, {0.0f, 0.0f, 0.5f}},
    };
    bool animate = false;
    bool showLights = false;
    float ambient = 0.3f;
    float diffuse = 0.8f;
    float specular = 0.4f;
    float specularPower = 1.0f;
};

class BumpMapping : public Example {
public:
    bool init() override
    {
        if (!shader_.load(assetPath("shaders/bump_mapping.vert"), assetPath("shaders/bump_mapping.frag")))
            return false;
        baseMap_ = Texture::fromBmp(assetPath("textures/stone_wall.bmp"));
        normalMap_ = Texture::fromBmp(assetPath("textures/stone_wall_normal.bmp"));
        glow_ = makeGlowTexture();
        return baseMap_ && normalMap_;
    }

    void update(const FrameInput& input) override
    {
        if (s_.animate) {
            // Swing the first light around the centre so the relief visibly reacts.
            s_.lights[0].position[0] = 5.0f + 3.0f * std::cos(input.time * 0.8f);
            s_.lights[0].position[1] = 5.0f + 3.0f * std::sin(input.time * 0.8f);
        }
    }

    void render(const FrameInput& input) override
    {
        // Keep the original 10x10 scene square and centred, whatever the window shape.
        const float aspect = input.aspect();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if (aspect >= 1.0f)
            glOrtho(5.0f - 5.0f * aspect, 5.0f + 5.0f * aspect, 0, kSceneSize, -20, 20);
        else
            glOrtho(0, kSceneSize, 5.0f - 5.0f / aspect, 5.0f + 5.0f / aspect, -20, 20);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glPushMatrix();
        glTranslatef(kSceneSize / 2, kSceneSize / 2, 0);
        glBlendFunc(GL_ONE, GL_ONE);
        bool firstPass = true;
        for (const BumpLight& light : s_.lights) {
            if (!light.enabled)
                continue;
            if (!firstPass)
                glEnable(GL_BLEND);
            drawLitQuad(light);
            firstPass = false;
        }
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glPopMatrix();

        if (s_.showLights)
            drawLightMarkers();
    }

    void drawUi() override
    {
        ImGui::SeparatorText("Lights (scene is 10 x 10)");
        for (int i = 0; i < 3; ++i) {
            ImGui::PushID(i);
            ImGui::Checkbox("##on", &s_.lights[i].enabled);
            ImGui::SameLine();
            char label[32];
            std::snprintf(label, sizeof(label), "Light %d", i + 1);
            ImGui::DragFloat3(label, s_.lights[i].position, 0.05f, -2.0f, 12.0f, "%.2f");
            ImGui::PopID();
        }
        ImGui::Checkbox("Animate light 1", &s_.animate);
        ImGui::Checkbox("Show light positions", &s_.showLights);

        ImGui::SeparatorText("Material (added once per light pass)");
        ImGui::SliderFloat("Ambient", &s_.ambient, 0.0f, 1.0f);
        ImGui::SliderFloat("Diffuse", &s_.diffuse, 0.0f, 2.0f);
        ImGui::SliderFloat("Specular", &s_.specular, 0.0f, 2.0f);
        ImGui::SliderFloat("Specular power", &s_.specularPower, 1.0f, 64.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
        if (ImGui::Button("Reset to original"))
            s_ = Settings();
    }

private:
    void drawLitQuad(const BumpLight& light)
    {
        const float eyePosition[3] = {0.0f, 0.0f, 100.0f};
        const float ambient[4] = {s_.ambient, s_.ambient, s_.ambient, 1.0f};
        const float diffuse[4] = {s_.diffuse, s_.diffuse, s_.diffuse, 1.0f};
        const float specular[4] = {s_.specular, s_.specular, s_.specular, 1.0f};

        shader_.use();
        glUniform3fv(shader_.uniform("fvLightPosition"), 1, light.position);
        glUniform3fv(shader_.uniform("fvEyePosition"), 1, eyePosition);
        glUniform4fv(shader_.uniform("fvAmbient"), 1, ambient);
        glUniform4fv(shader_.uniform("fvDiffuse"), 1, diffuse);
        glUniform4fv(shader_.uniform("fvSpecular"), 1, specular);
        glUniform1f(shader_.uniform("fSpecularPower"), s_.specularPower);
        glUniform1i(shader_.uniform("baseMap"), 0);
        glUniform1i(shader_.uniform("bumpMap"), 1);
        normalMap_.bind(1);
        baseMap_.bind(0);

        const GLint tangent = shader_.attribute("rm_Tangent");
        const GLint binormal = shader_.attribute("rm_Binormal");
        const float tangentValue[3] = {1.0f, 0.0f, 0.0f};
        const float binormalValue[3] = {0.0f, 1.0f, 0.0f};

        // Corners: position (x, y) and texture coordinate (s, t), as in the original.
        const float corners[4][4] = {
            {-kQuadHalfSize, -kQuadHalfSize, 0.0f, 1.0f},
            {-kQuadHalfSize, kQuadHalfSize, 0.0f, 0.0f},
            {kQuadHalfSize, kQuadHalfSize, 1.0f, 0.0f},
            {kQuadHalfSize, -kQuadHalfSize, 1.0f, 1.0f},
        };

        glNormal3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_QUADS);
        for (const float* c : corners) {
            glVertexAttrib3fv(binormal, binormalValue);
            glVertexAttrib3fv(tangent, tangentValue);
            glTexCoord2f(c[2], c[3]);
            glVertex3f(c[0], c[1], 0.0f);
        }
        glEnd();
        Shader::unuse();
    }

    void drawLightMarkers()
    {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glow_.bind(0);
        glBegin(GL_QUADS);
        for (const BumpLight& light : s_.lights)
            if (light.enabled)
                emitSprite(light.position[0], light.position[1], 0.9f, 1.0f, 0.95f, 0.7f, 1.0f);
        glEnd();
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
    }

    Shader shader_;
    Texture baseMap_;
    Texture normalMap_;
    Texture glow_;
    Settings s_;
};

} // namespace

std::unique_ptr<Example> createBumpMapping()
{
    return std::unique_ptr<Example>(new BumpMapping());
}

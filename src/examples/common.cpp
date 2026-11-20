#include "examples/common.h"

#include <cstdio>

#include "assets.h"

Color hsvToRgb(float h, float s, float v)
{
    h = (h - std::floor(h)) * 6.0f;
    const int sector = static_cast<int>(h);
    const float f = h - sector;
    const float p = v * (1 - s);
    const float q = v * (1 - s * f);
    const float t = v * (1 - s * (1 - f));
    switch (sector) {
    case 0: return {v, t, p};
    case 1: return {q, v, p};
    case 2: return {p, v, t};
    case 3: return {p, q, v};
    case 4: return {t, p, v};
    default: return {v, p, q};
    }
}

void World2D::applyProjection() const
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

Texture makeGlowTexture(int size)
{
    std::vector<unsigned char> pixels(static_cast<size_t>(size) * size * 4);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            const float dx = (x + 0.5f) / size * 2 - 1;
            const float dy = (y + 0.5f) / size * 2 - 1;
            const float falloff = clamp01(1.0f - std::sqrt(dx * dx + dy * dy));
            unsigned char* p = &pixels[(static_cast<size_t>(y) * size + x) * 4];
            p[0] = p[1] = p[2] = 255;
            p[3] = static_cast<unsigned char>(falloff * falloff * 255.0f);
        }
    }
    return Texture::create(size, size, GL_RGBA, pixels.data(), GL_CLAMP_TO_EDGE);
}

void emitSprite(float x, float y, float size, float r, float g, float b, float a)
{
    const float h = size * 0.5f;
    glColor4f(r, g, b, a);
    glTexCoord2f(0, 0); glVertex2f(x - h, y - h);
    glTexCoord2f(1, 0); glVertex2f(x + h, y - h);
    glTexCoord2f(1, 1); glVertex2f(x + h, y + h);
    glTexCoord2f(0, 1); glVertex2f(x - h, y + h);
}

// ---------------------------------------------------------------------------

bool LitWall::init()
{
    if (!shader_.load(assetPath("shaders/lit_wall.vert"), assetPath("shaders/lit_wall.frag")))
        return false;
    baseMap_ = Texture::fromBmp(assetPath("textures/stone_wall.bmp"));
    normalMap_ = Texture::fromBmp(assetPath("textures/stone_wall_normal.bmp"));
    return baseMap_ && normalMap_;
}

void LitWall::draw(const World2D& world, const std::vector<Light>& lights, const Material& material)
{
    // Pack the lights into the uniform arrays the shader expects.
    const int count = static_cast<int>(lights.size() < size_t(kMaxLights) ? lights.size() : size_t(kMaxLights));
    float position[kMaxLights * 3] = {};
    float color[kMaxLights * 3] = {};
    float radius[kMaxLights] = {};
    float direction[kMaxLights * 3] = {};
    float cone[kMaxLights * 2] = {};  // (innerCos, outerCos); outerCos <= -1 means point light
    for (int i = 0; i < count; ++i) {
        const Light& l = lights[i];
        position[i * 3 + 0] = l.position.x;
        position[i * 3 + 1] = l.position.y;
        position[i * 3 + 2] = l.position.z;
        color[i * 3 + 0] = l.color.r;
        color[i * 3 + 1] = l.color.g;
        color[i * 3 + 2] = l.color.b;
        radius[i] = l.radius;
        direction[i * 3 + 0] = l.direction.x;
        direction[i * 3 + 1] = l.direction.y;
        direction[i * 3 + 2] = l.direction.z;
        cone[i * 2 + 0] = l.spot ? l.innerCos : -2.0f;
        cone[i * 2 + 1] = l.spot ? l.outerCos : -2.0f;
    }

    shader_.use();
    glUniform1i(shader_.uniform("uBaseMap"), 0);
    glUniform1i(shader_.uniform("uNormalMap"), 1);
    glUniform1f(shader_.uniform("uTileSize"), material.tileSize);
    glUniform3f(shader_.uniform("uAmbient"), material.ambient.r, material.ambient.g, material.ambient.b);
    glUniform1f(shader_.uniform("uSpecular"), material.specular);
    glUniform1f(shader_.uniform("uShininess"), material.shininess);
    glUniform1i(shader_.uniform("uLightCount"), count);
    glUniform3fv(shader_.uniform("uLightPosition"), kMaxLights, position);
    glUniform3fv(shader_.uniform("uLightColor"), kMaxLights, color);
    glUniform1fv(shader_.uniform("uLightRadius"), kMaxLights, radius);
    glUniform3fv(shader_.uniform("uSpotDirection"), kMaxLights, direction);
    glUniform2fv(shader_.uniform("uSpotCone"), kMaxLights, cone);

    normalMap_.bind(1);
    baseMap_.bind(0);

    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(world.width, 0);
    glVertex2f(world.width, world.height);
    glVertex2f(0, world.height);
    glEnd();

    Shader::unuse();
}

#pragma once

#include <GL/glew.h>

#include <cmath>
#include <random>
#include <vector>

#include "example.h"
#include "gl/shader.h"
#include "gl/texture.h"

// Small helpers shared by the examples.

struct Vec2 {
    float x = 0, y = 0;
};

struct Vec3 {
    float x = 0, y = 0, z = 0;
};

struct Color {
    float r = 1, g = 1, b = 1;
};

const float kPi = 3.14159265358979f;

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
inline float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
inline Color scale(Color c, float k) { return {c.r * k, c.g * k, c.b * k}; }
Color hsvToRgb(float h, float s, float v);

// Uniform random numbers.
class Random {
public:
    explicit Random(unsigned seed = 1234) : engine_(seed) {}
    float range(float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(engine_); }
    float unit() { return range(0.0f, 1.0f); }

private:
    std::mt19937 engine_;
};

// 2D world used by the lighting, fire and water examples: 10 units high, as wide as the aspect ratio allows.
// (0, 0) is the bottom-left corner of the window.
struct World2D {
    float width = 10;
    float height = 10;

    explicit World2D(const FrameInput& input) : width(10.0f * input.aspect()) {}
    // Sets an orthographic projection covering the world.
    void applyProjection() const;
    Vec2 mouse(const FrameInput& input) const { return {input.mouseX * width, input.mouseY * height}; }
};

// White RGBA sprite with a soft radial alpha falloff, for glows and particles.
Texture makeGlowTexture(int size = 64);

// Draws camera-facing quads with the bound texture in immediate mode.
// Call between glBegin(GL_QUADS) and glEnd().
void emitSprite(float x, float y, float size, float r, float g, float b, float a);

// ---------------------------------------------------------------------------
// Normal-mapped stone wall lit by up to kMaxLights point/spot lights in one pass.
// The wall lies in the z = 0 plane of a World2D, so tangent space is world space.

const int kMaxLights = 8;

struct Light {
    Vec3 position;      // world units; z is the height in front of the wall
    Color color;        // already multiplied by intensity
    float radius = 8;   // light reaches zero at this distance
    bool spot = false;
    Vec3 direction;     // spot lights only, normalized
    float innerCos = 0.95f;  // cos of the angle with full intensity
    float outerCos = 0.85f;  // cos of the angle where the spot fades to zero
};

struct Material {
    Color ambient = {0.08f, 0.08f, 0.08f};
    float specular = 0.4f;
    float shininess = 24.0f;
    float tileSize = 5.0f;  // world units per texture repeat
};

class LitWall {
public:
    bool init();
    void draw(const World2D& world, const std::vector<Light>& lights, const Material& material);

private:
    Shader shader_;
    Texture baseMap_;
    Texture normalMap_;
};

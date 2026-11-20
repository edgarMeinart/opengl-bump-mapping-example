#include "app.h"

#include <cstdio>

#include "texture.h"

namespace {

// Scene is a 10x10 orthographic view; the quad is centred on (5, 5).
const float kViewSize = 10.0f;
const float kQuadHalfSize = 5.0f;

// One additive pass per light, in eye-space coordinates.
const float kLightZ = 0.5f;

const float kEyePosition[3] = {0.0f, 0.0f, 100.0f};
const float kAmbient[4] = {0.3f, 0.3f, 0.3f, 1.0f};
const float kDiffuse[4] = {0.8f, 0.8f, 0.8f, 1.0f};
const float kSpecular[4] = {0.4f, 0.4f, 0.4f, 1.0f};
const float kSpecularPower = 1.0f;

// Tangent space of the quad: it lies in the XY plane facing +Z.
const float kTangent[3] = {1.0f, 0.0f, 0.0f};
const float kBinormal[3] = {0.0f, 1.0f, 0.0f};

} // namespace

App::~App()
{
    if (context_) {
        glDeleteTextures(1, &baseMap_);
        glDeleteTextures(1, &normalMap_);
        shader_.destroy();
        SDL_GL_DeleteContext(context_);
    }
    if (window_)
        SDL_DestroyWindow(window_);
    SDL_Quit();
}

bool App::init(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return false;
    }

    // Assets live next to the build directory: <repo>/build/bump -> <repo>/assets/
    char* base = SDL_GetBasePath();
    assetDir_ = std::string(base ? base : "./") + "../assets/";
    SDL_free(base);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    window_ = SDL_CreateWindow("OpenGL bump mapping", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width, height, SDL_WINDOW_OPENGL);
    if (!window_) {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return false;
    }
    context_ = SDL_GL_CreateContext(window_);
    if (!context_) {
        std::fprintf(stderr, "SDL_GL_CreateContext: %s\n", SDL_GetError());
        return false;
    }
    SDL_GL_SetSwapInterval(1);

    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::fprintf(stderr, "glewInit: %s\n", glewGetErrorString(glewStatus));
        return false;
    }
    if (!GLEW_VERSION_2_0) {
        std::fprintf(stderr, "OpenGL 2.0 is required, got %s\n", glGetString(GL_VERSION));
        return false;
    }

    if (!shader_.load(assetPath("shaders/bump.vert"), assetPath("shaders/bump.frag")))
        return false;

    baseMap_ = loadBmpTexture(assetPath("textures/stone_wall.bmp"));
    normalMap_ = loadBmpTexture(assetPath("textures/stone_wall_normal.bmp"));
    if (!baseMap_ || !normalMap_)
        return false;

    int drawableWidth = 0, drawableHeight = 0;
    SDL_GL_GetDrawableSize(window_, &drawableWidth, &drawableHeight);
    glViewport(0, 0, drawableWidth, drawableHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, kViewSize, 0, kViewSize, -20.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    return true;
}

void App::run()
{
    for (;;) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                return;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                return;
        }
        render();
        SDL_GL_SwapWindow(window_);
    }
}

std::string App::assetPath(const char* relative) const
{
    return assetDir_ + relative;
}

void App::render()
{
    static const Light lights[] = {
        {8.0f, 4.0f, kLightZ},
        {4.0f, 4.0f, kLightZ},
        {0.0f, 0.0f, kLightZ},
    };

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    glPushMatrix();
    glTranslatef(kViewSize / 2, kViewSize / 2, 0.0f);

    // First light is drawn opaque, the others are added on top (multipass lighting).
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    for (const Light& light : lights) {
        drawLitQuad(light);
        glEnable(GL_BLEND);
    }
    glDisable(GL_BLEND);

    glPopMatrix();
}

void App::drawLitQuad(const Light& light)
{
    const float lightPosition[3] = {light.x, light.y, light.z};

    shader_.use();
    glUniform3fv(shader_.uniform("fvLightPosition"), 1, lightPosition);
    glUniform3fv(shader_.uniform("fvEyePosition"), 1, kEyePosition);
    glUniform4fv(shader_.uniform("fvAmbient"), 1, kAmbient);
    glUniform4fv(shader_.uniform("fvDiffuse"), 1, kDiffuse);
    glUniform4fv(shader_.uniform("fvSpecular"), 1, kSpecular);
    glUniform1f(shader_.uniform("fSpecularPower"), kSpecularPower);
    glUniform1i(shader_.uniform("baseMap"), 0);
    glUniform1i(shader_.uniform("bumpMap"), 1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, baseMap_);

    const GLint tangent = shader_.attribute("rm_Tangent");
    const GLint binormal = shader_.attribute("rm_Binormal");

    // Corners: position (x, y) and texture coordinate (s, t).
    static const float corners[4][4] = {
        {-kQuadHalfSize, -kQuadHalfSize, 0.0f, 1.0f},
        {-kQuadHalfSize,  kQuadHalfSize, 0.0f, 0.0f},
        { kQuadHalfSize,  kQuadHalfSize, 1.0f, 0.0f},
        { kQuadHalfSize, -kQuadHalfSize, 1.0f, 1.0f},
    };

    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    for (const float* c : corners) {
        glVertexAttrib3fv(binormal, kBinormal);
        glVertexAttrib3fv(tangent, kTangent);
        glTexCoord2f(c[2], c[3]);
        glVertex3f(c[0], c[1], 0.0f);
    }
    glEnd();

    Shader::unuse();
}

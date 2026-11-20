#pragma once

#include <GL/glew.h>
#include <SDL.h>

#include <string>

#include "shader.h"

// Window, GL context and the bump-mapped scene.
class App {
public:
    App() = default;
    ~App();
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    bool init(int width, int height);
    void run();

private:
    struct Light {
        float x, y, z;
    };

    void render();
    void drawLitQuad(const Light& light);
    std::string assetPath(const char* relative) const;

    SDL_Window* window_ = nullptr;
    SDL_GLContext context_ = nullptr;
    std::string assetDir_;

    Shader shader_;
    GLuint baseMap_ = 0;
    GLuint normalMap_ = 0;
};

#pragma once

#include <SDL.h>

#include <memory>
#include <string>

#include "example.h"

struct AppOptions {
    int startExample = -1;        // index into allExamples(); -1 opens the start menu
    std::string screenshotPath;   // if set: save a BMP after screenshotDelay seconds and quit
    float screenshotDelay = 3.0f;
};

// Window, GL context, Dear ImGui, the start menu and the running example.
class App {
public:
    App() = default;
    ~App();
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    bool init(const AppOptions& options);
    void run();

private:
    void handleEvent(const SDL_Event& event, bool& quit);
    FrameInput collectInput(float time, float dt);
    void drawMenu();
    void drawExampleWindow();
    void startExample(int index);
    void stopExample();
    bool saveScreenshot(const std::string& path) const;

    SDL_Window* window_ = nullptr;
    SDL_GLContext context_ = nullptr;
    bool imguiReady_ = false;
    AppOptions options_;

    std::unique_ptr<Example> example_;
    int exampleIndex_ = -1;
    int selected_ = 0;           // highlighted entry in the menu
    std::string error_;          // shown in the menu when an example failed to start
    Uint64 exampleStart_ = 0;
    bool mouseWasDown_ = false;
    bool requestMenu_ = false;   // deferred so the example isn't destroyed mid-frame
};

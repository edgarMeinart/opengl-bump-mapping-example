#pragma once

#include <memory>
#include <vector>

// Per-frame input handed to the running example.
struct FrameInput {
    int width = 0;   // framebuffer size in pixels
    int height = 0;
    float time = 0;  // seconds since the example started
    float dt = 0;    // seconds since the previous frame (clamped)

    // Mouse in normalized view coordinates: (0, 0) bottom-left, (1, 1) top-right.
    // The mouse fields are only set while the cursor is over the scene, not over the UI.
    bool mouseInView = false;
    float mouseX = 0;
    float mouseY = 0;
    bool mouseDown = false;     // left button held
    bool mouseClicked = false;  // left button went down this frame

    float aspect() const { return height > 0 ? float(width) / float(height) : 1.0f; }
};

// One demo scene. The GL context is current for the whole lifetime of the object.
class Example {
public:
    virtual ~Example() = default;

    // Loads shaders and textures. Returns false (after printing why) if the example can't run.
    virtual bool init() = 0;
    virtual void update(const FrameInput& /*input*/) {}
    // Draws the scene. The viewport is already set; matrices are identity; no shader is bound.
    virtual void render(const FrameInput& input) = 0;
    // Adds the example's controls to the settings window (Dear ImGui).
    virtual void drawUi() {}
};

struct ExampleInfo {
    const char* id;           // command-line name, e.g. "fire"
    const char* title;
    const char* description;  // shown in the start menu
    std::unique_ptr<Example> (*create)();
};

// All examples, in menu order.
const std::vector<ExampleInfo>& allExamples();

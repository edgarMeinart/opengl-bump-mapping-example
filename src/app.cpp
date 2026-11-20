#include "app.h"

#include <GL/glew.h>

#include <algorithm>
#include <cstdio>
#include <vector>

#include "assets.h"
#include "imgui.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_sdl2.h"

namespace {

const char* kWindowTitle = "OpenGL examples";

double secondsSince(Uint64 start)
{
    return double(SDL_GetPerformanceCounter() - start) / double(SDL_GetPerformanceFrequency());
}

// Puts the fixed-function state back to defaults so examples start from a known state
// and the OpenGL2 ImGui backend (which can't unbind shaders itself) draws correctly.
void resetGlState()
{
    glUseProgram(0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glColor4f(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

} // namespace

App::~App()
{
    example_.reset();  // examples free GL objects, so they go first
    if (imguiReady_) {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }
    if (context_)
        SDL_GL_DeleteContext(context_);
    if (window_)
        SDL_DestroyWindow(window_);
    SDL_Quit();
}

bool App::init(const AppOptions& options)
{
    options_ = options;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return false;
    }

    // Assets live next to the build directory: <repo>/build/opengl-examples -> <repo>/assets/
    char* base = SDL_GetBasePath();
    setAssetRoot(std::string(base ? base : "./") + "../assets/");
    SDL_free(base);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    window_ = SDL_CreateWindow(kWindowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 800,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
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
    const GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::fprintf(stderr, "glewInit: %s\n", glewGetErrorString(glewStatus));
        return false;
    }
    if (!GLEW_VERSION_2_0) {
        std::fprintf(stderr, "OpenGL 2.0 is required, got %s\n", glGetString(GL_VERSION));
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;  // don't write imgui.ini next to the binary
    ImGui::StyleColorsDark();
    ImGui::GetStyle().FontSizeBase = 15.0f;
    ImGui::GetStyle().WindowRounding = 6.0f;
    ImGui::GetStyle().FrameRounding = 3.0f;
    ImGui_ImplSDL2_InitForOpenGL(window_, context_);
    ImGui_ImplOpenGL2_Init();
    imguiReady_ = true;

    if (options_.startExample >= 0) {
        startExample(options_.startExample);
        if (!example_)
            return false;
    }
    return true;
}

void App::run()
{
    const Uint64 appStart = SDL_GetPerformanceCounter();
    Uint64 last = appStart;
    bool quit = false;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event))
            handleEvent(event, quit);

        const Uint64 now = SDL_GetPerformanceCounter();
        float dt = float(double(now - last) / double(SDL_GetPerformanceFrequency()));
        last = now;
        if (dt > 0.1f)
            dt = 0.1f;

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        int width = 0, height = 0;
        SDL_GL_GetDrawableSize(window_, &width, &height);
        glViewport(0, 0, width, height);
        resetGlState();

        if (example_) {
            const FrameInput input = collectInput(float(secondsSince(exampleStart_)), dt);
            example_->update(input);
            drawExampleWindow();
            example_->render(input);
        } else {
            glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawMenu();
        }

        resetGlState();
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        if (!options_.screenshotPath.empty() && secondsSince(appStart) >= options_.screenshotDelay) {
            if (saveScreenshot(options_.screenshotPath))
                std::printf("Saved %s\n", options_.screenshotPath.c_str());
            quit = true;
        }
        SDL_GL_SwapWindow(window_);

        if (requestMenu_) {
            requestMenu_ = false;
            stopExample();
        }
    }
}

void App::handleEvent(const SDL_Event& event, bool& quit)
{
    ImGui_ImplSDL2_ProcessEvent(&event);

    if (event.type == SDL_QUIT)
        quit = true;
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
        event.window.windowID == SDL_GetWindowID(window_))
        quit = true;
    if (event.type != SDL_KEYDOWN || ImGui::GetIO().WantTextInput)
        return;

    const SDL_Keycode key = event.key.keysym.sym;
    const int count = static_cast<int>(allExamples().size());
    if (key == SDLK_ESCAPE) {
        if (example_)
            requestMenu_ = true;
        else
            quit = true;
    } else if (!example_) {
        if (key == SDLK_UP)
            selected_ = (selected_ + count - 1) % count;
        else if (key == SDLK_DOWN)
            selected_ = (selected_ + 1) % count;
        else if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
            startExample(selected_);
        else if (key >= SDLK_1 && key < SDLK_1 + count)
            startExample(key - SDLK_1);
    }
}

FrameInput App::collectInput(float time, float dt)
{
    FrameInput input;
    SDL_GL_GetDrawableSize(window_, &input.width, &input.height);
    input.time = time;
    input.dt = dt;

    int mouseX = 0, mouseY = 0, windowWidth = 1, windowHeight = 1;
    const Uint32 buttons = SDL_GetMouseState(&mouseX, &mouseY);
    SDL_GetWindowSize(window_, &windowWidth, &windowHeight);

    // Screenshot runs ignore the real cursor so the captured frame doesn't depend on where it is.
    input.mouseInView = options_.screenshotPath.empty() && SDL_GetMouseFocus() == window_ &&
                        !ImGui::GetIO().WantCaptureMouse;
    if (input.mouseInView) {
        input.mouseX = float(mouseX) / float(windowWidth);
        input.mouseY = 1.0f - float(mouseY) / float(windowHeight);
        input.mouseDown = (buttons & SDL_BUTTON_LMASK) != 0;
        input.mouseClicked = input.mouseDown && !mouseWasDown_;
    }
    mouseWasDown_ = input.mouseDown;
    return input;
}

void App::drawMenu()
{
    const std::vector<ExampleInfo>& examples = allExamples();
    const ImGuiIO& io = ImGui::GetIO();
    const ImGuiStyle& style = ImGui::GetStyle();

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(760, 330), ImGuiCond_Always);
    ImGui::Begin(kWindowTitle, nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImGui::TextDisabled("Pick an example. Double-click, Enter or a number key starts it; Esc returns here.");
    ImGui::Spacing();

    const float footer = ImGui::GetFrameHeightWithSpacing() + style.ItemSpacing.y;
    ImGui::BeginChild("list", ImVec2(240, -footer), ImGuiChildFlags_Borders);
    for (int i = 0; i < static_cast<int>(examples.size()); ++i) {
        char label[128];
        std::snprintf(label, sizeof(label), "%d.  %s", i + 1, examples[i].title);
        if (ImGui::Selectable(label, selected_ == i, ImGuiSelectableFlags_AllowDoubleClick)) {
            selected_ = i;
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                startExample(i);
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();

    const ExampleInfo& info = examples[selected_];
    ImGui::BeginChild("details", ImVec2(0, -footer), ImGuiChildFlags_Borders);
    ImGui::PushFont(nullptr, style.FontSizeBase * 1.5f);
    ImGui::TextUnformatted(info.title);
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::TextWrapped("%s", info.description);
    ImGui::Spacing();
    ImGui::TextDisabled("From the terminal: ./build/opengl-examples %s", info.id);
    ImGui::EndChild();

    if (ImGui::Button("Run", ImVec2(120, 0)))
        startExample(selected_);
    if (!error_.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.4f, 1.0f), "%s", error_.c_str());
    }
    ImGui::End();
}

void App::drawExampleWindow()
{
    const ExampleInfo& info = allExamples()[exampleIndex_];

    ImGui::SetNextWindowPos(ImVec2(12, 12), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(0.85f);
    ImGui::Begin(info.title, nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::Button("< Menu (Esc)"))
        requestMenu_ = true;
    ImGui::SameLine();
    ImGui::Text("%.0f FPS", ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::PushItemWidth(200);
    example_->drawUi();
    ImGui::PopItemWidth();
    ImGui::End();
}

void App::startExample(int index)
{
    stopExample();
    const ExampleInfo& info = allExamples()[index];
    std::unique_ptr<Example> example = info.create();
    if (!example->init()) {
        error_ = std::string("Could not start \"") + info.title + "\", see the terminal output.";
        std::fprintf(stderr, "%s\n", error_.c_str());
        return;
    }
    example_ = std::move(example);
    exampleIndex_ = index;
    selected_ = index;
    exampleStart_ = SDL_GetPerformanceCounter();
    error_.clear();
    SDL_SetWindowTitle(window_, (std::string(kWindowTitle) + " - " + info.title).c_str());
}

void App::stopExample()
{
    example_.reset();
    exampleIndex_ = -1;
    SDL_SetWindowTitle(window_, kWindowTitle);
}

bool App::saveScreenshot(const std::string& path) const
{
    int width = 0, height = 0;
    SDL_GL_GetDrawableSize(window_, &width, &height);
    const int rowSize = width * 3;
    std::vector<unsigned char> pixels(static_cast<size_t>(rowSize) * height);
    std::vector<unsigned char> flipped(pixels.size());

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    for (int y = 0; y < height; ++y)  // OpenGL rows are bottom-up, BMP rows here are top-down
        std::copy_n(&pixels[static_cast<size_t>(y) * rowSize], rowSize,
                    &flipped[static_cast<size_t>(height - 1 - y) * rowSize]);

    SDL_Surface* surface =
        SDL_CreateRGBSurfaceWithFormatFrom(flipped.data(), width, height, 24, rowSize, SDL_PIXELFORMAT_RGB24);
    const bool ok = surface && SDL_SaveBMP(surface, path.c_str()) == 0;
    if (!ok)
        std::fprintf(stderr, "Can't save %s: %s\n", path.c_str(), SDL_GetError());
    SDL_FreeSurface(surface);
    return ok;
}

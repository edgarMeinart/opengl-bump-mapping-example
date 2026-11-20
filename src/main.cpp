#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "app.h"

namespace {

void printUsage(const char* program)
{
    std::printf("Usage: %s [options] [example]\n\n", program);
    std::printf("Without an example name the start menu opens.\n\n");
    std::printf("Options:\n");
    std::printf("  --list             list the examples and exit\n");
    std::printf("  --screenshot FILE  save a BMP screenshot after a few seconds, then exit\n");
    std::printf("  --delay SECONDS    screenshot delay (default 3)\n");
    std::printf("  --help             show this help\n");
}

int findExample(const char* id)
{
    const std::vector<ExampleInfo>& examples = allExamples();
    for (size_t i = 0; i < examples.size(); ++i)
        if (std::strcmp(examples[i].id, id) == 0)
            return static_cast<int>(i);
    return -1;
}

} // namespace

int main(int argc, char* argv[])
{
    AppOptions options;
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (std::strcmp(arg, "--help") == 0 || std::strcmp(arg, "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (std::strcmp(arg, "--list") == 0) {
            for (const ExampleInfo& info : allExamples())
                std::printf("%-10s %s\n", info.id, info.title);
            return 0;
        } else if (std::strcmp(arg, "--screenshot") == 0 && i + 1 < argc) {
            options.screenshotPath = argv[++i];
        } else if (std::strcmp(arg, "--delay") == 0 && i + 1 < argc) {
            options.screenshotDelay = static_cast<float>(std::atof(argv[++i]));
        } else if (arg[0] != '-' && options.startExample < 0) {
            options.startExample = findExample(arg);
            if (options.startExample < 0) {
                std::fprintf(stderr, "Unknown example \"%s\"; see --list\n", arg);
                return 1;
            }
        } else {
            printUsage(argv[0]);
            return 1;
        }
    }

    App app;
    if (!app.init(options))
        return 1;
    app.run();
    return 0;
}

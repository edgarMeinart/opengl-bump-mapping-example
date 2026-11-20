#include "app.h"

int main(int /*argc*/, char* /*argv*/[])
{
    App app;
    if (!app.init(700, 700))
        return 1;
    app.run();
    return 0;
}

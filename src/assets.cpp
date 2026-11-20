#include "assets.h"

namespace {
std::string g_assetRoot = "assets/";
}

void setAssetRoot(const std::string& directory)
{
    g_assetRoot = directory;
    if (!g_assetRoot.empty() && g_assetRoot.back() != '/')
        g_assetRoot += '/';
}

std::string assetPath(const std::string& relative)
{
    return g_assetRoot + relative;
}

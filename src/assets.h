#pragma once

#include <string>

// Directory that holds shaders/ and textures/. Set once at startup.
void setAssetRoot(const std::string& directory);

// assetPath("shaders/fire.frag") -> "<asset root>/shaders/fire.frag"
std::string assetPath(const std::string& relative);

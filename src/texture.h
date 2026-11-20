#pragma once

#include <GL/glew.h>

#include <string>

// Loads a BMP file into a new GL_TEXTURE_2D with linear filtering.
// Rows are uploaded bottom row first, so texture coordinate t = 0 is the bottom of the image.
// Returns 0 and prints the error on failure.
GLuint loadBmpTexture(const std::string& path);

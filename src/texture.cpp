#include "texture.h"

#include <SDL.h>

#include <cstdio>
#include <cstring>
#include <vector>

GLuint loadBmpTexture(const std::string& path)
{
    SDL_Surface* bmp = SDL_LoadBMP(path.c_str());
    SDL_Surface* rgb = bmp ? SDL_ConvertSurfaceFormat(bmp, SDL_PIXELFORMAT_RGB24, 0) : nullptr;
    SDL_FreeSurface(bmp);
    if (!rgb) {
        std::fprintf(stderr, "Can't load %s: %s\n", path.c_str(), SDL_GetError());
        return 0;
    }

    // Repack tightly, bottom row first (the layout OpenGL expects for t = 0 at the bottom).
    const int rowSize = rgb->w * 3;
    std::vector<unsigned char> pixels(static_cast<size_t>(rowSize) * rgb->h);
    for (int y = 0; y < rgb->h; ++y) {
        std::memcpy(&pixels[static_cast<size_t>(rgb->h - 1 - y) * rowSize],
                    static_cast<const unsigned char*>(rgb->pixels) + y * rgb->pitch,
                    rowSize);
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rgb->w, rgb->h, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(rgb);
    return texture;
}

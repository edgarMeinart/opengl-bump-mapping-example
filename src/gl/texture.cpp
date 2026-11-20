#include "texture.h"

#include <SDL.h>

#include <cstdio>
#include <cstring>
#include <vector>

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        destroy();
        id_ = other.id_;
        width_ = other.width_;
        height_ = other.height_;
        format_ = other.format_;
        other.id_ = 0;
    }
    return *this;
}

Texture Texture::fromBmp(const std::string& path)
{
    SDL_Surface* bmp = SDL_LoadBMP(path.c_str());
    SDL_Surface* rgb = bmp ? SDL_ConvertSurfaceFormat(bmp, SDL_PIXELFORMAT_RGB24, 0) : nullptr;
    SDL_FreeSurface(bmp);
    if (!rgb) {
        std::fprintf(stderr, "Can't load %s: %s\n", path.c_str(), SDL_GetError());
        return Texture();
    }

    // Repack tightly, bottom row first (the layout OpenGL expects for t = 0 at the bottom).
    const int rowSize = rgb->w * 3;
    std::vector<unsigned char> pixels(static_cast<size_t>(rowSize) * rgb->h);
    for (int y = 0; y < rgb->h; ++y) {
        std::memcpy(&pixels[static_cast<size_t>(rgb->h - 1 - y) * rowSize],
                    static_cast<const unsigned char*>(rgb->pixels) + y * rgb->pitch,
                    rowSize);
    }

    Texture texture = create(rgb->w, rgb->h, GL_RGB, pixels.data());
    SDL_FreeSurface(rgb);
    return texture;
}

Texture Texture::create(int width, int height, GLenum format, const void* pixels, GLenum wrap)
{
    Texture texture;
    texture.width_ = width;
    texture.height_ = height;
    texture.format_ = format;

    glGenTextures(1, &texture.id_);
    glBindTexture(GL_TEXTURE_2D, texture.id_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void Texture::update(const void* pixels)
{
    glBindTexture(GL_TEXTURE_2D, id_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, format_, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::bind(int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id_);
}

void Texture::destroy()
{
    if (id_) {
        glDeleteTextures(1, &id_);
        id_ = 0;
    }
}

#pragma once

#include <GL/glew.h>

#include <string>

// An OpenGL 2D texture. Move-only; must be destroyed while the GL context is alive.
class Texture {
public:
    Texture() = default;
    ~Texture() { destroy(); }
    Texture(Texture&& other) noexcept : id_(other.id_), width_(other.width_), height_(other.height_), format_(other.format_)
    {
        other.id_ = 0;
    }
    Texture& operator=(Texture&& other) noexcept;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Loads a BMP file as RGB, bottom row first (texture coordinate t = 0 is the bottom of the image),
    // with linear filtering and GL_REPEAT wrapping. Returns an empty texture and prints the error on failure.
    static Texture fromBmp(const std::string& path);

    // Creates a texture from tightly packed 8-bit pixels (`format` is GL_RGB or GL_RGBA; `pixels` may be null).
    static Texture create(int width, int height, GLenum format, const void* pixels, GLenum wrap = GL_REPEAT);

    // Replaces the whole image; `pixels` must match the size and format given to create().
    void update(const void* pixels);

    void bind(int unit) const;
    void destroy();

    explicit operator bool() const { return id_ != 0; }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    GLuint id_ = 0;
    int width_ = 0;
    int height_ = 0;
    GLenum format_ = GL_RGB;
};

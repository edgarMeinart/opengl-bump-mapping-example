#pragma once

#include <GL/glew.h>

#include <string>

// A linked GLSL program built from one vertex and one fragment shader file.
// Owns the GL program; must be destroyed while the GL context is alive.
class Shader {
public:
    Shader() = default;
    ~Shader() { destroy(); }
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Compiles and links both stages; prints the info log and returns false on failure.
    bool load(const std::string& vertexPath, const std::string& fragmentPath);
    void destroy();

    void use() const { glUseProgram(program_); }
    static void unuse() { glUseProgram(0); }

    GLint uniform(const char* name) const { return glGetUniformLocation(program_, name); }
    GLint attribute(const char* name) const { return glGetAttribLocation(program_, name); }

private:
    GLuint program_ = 0;
};

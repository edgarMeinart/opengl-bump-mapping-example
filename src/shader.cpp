#include "shader.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

namespace {

bool readFile(const std::string& path, std::string& out)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::fprintf(stderr, "Can't open %s\n", path.c_str());
        return false;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    out = ss.str();
    return true;
}

void printLog(GLuint object, bool isProgram, const std::string& what)
{
    GLint length = 0;
    if (isProgram)
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &length);
    else
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1)
        return;

    std::vector<char> log(length);
    if (isProgram)
        glGetProgramInfoLog(object, length, nullptr, log.data());
    else
        glGetShaderInfoLog(object, length, nullptr, log.data());
    std::fprintf(stderr, "%s:\n%s\n", what.c_str(), log.data());
}

GLuint compile(GLenum type, const std::string& path)
{
    std::string source;
    if (!readFile(path, source))
        return 0;

    GLuint shader = glCreateShader(type);
    const char* text = source.c_str();
    glShaderSource(shader, 1, &text, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        printLog(shader, false, path);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

} // namespace

bool Shader::load(const std::string& vertexPath, const std::string& fragmentPath)
{
    GLuint vs = compile(GL_VERTEX_SHADER, vertexPath);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentPath);
    if (!vs || !fs) {
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) {
        printLog(program_, true, "link");
        destroy();
        return false;
    }
    return true;
}

void Shader::destroy()
{
    glDeleteProgram(program_);
    program_ = 0;
}

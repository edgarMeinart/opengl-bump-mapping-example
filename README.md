# OpenGL bump mapping example

A small per-pixel bump mapping (normal mapping) demo in C++, OpenGL 2.1 and GLSL 1.10, written as a school-time pet project.
It draws one textured quad and lights it with three point lights. The lighting is done in tangent space in the fragment shader, using a normal map.

![Screenshot](docs/screenshot.jpg)

## Stack

- C++ (C++11 compiler)
- OpenGL 2.1, fixed-function pipeline plus GLSL 1.10 shaders
- [SDL2](https://www.libsdl.org/) for the window, the GL context and BMP loading
- [GLEW](https://glew.sourceforge.net/) for the GL 2.0 entry points (shaders, multitexture)

## Build and run

### macOS (Homebrew)

```sh
brew install sdl2 glew pkg-config
make run
```

### Linux

```sh
# Debian / Ubuntu
sudo apt install build-essential pkg-config libsdl2-dev libglew-dev libgl-dev
# Fedora
sudo dnf install gcc-c++ make pkgconf SDL2-devel glew-devel mesa-libGL-devel

make run
```

GLEW from distro packages is usually built for GLX. On a Wayland session, if the window opens but rendering fails, run it through XWayland with `SDL_VIDEODRIVER=x11 ./bump`.

### Windows (MSYS2)

From an MSYS2 MINGW64 shell:

```sh
pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-pkgconf mingw-w64-x86_64-SDL2 mingw-w64-x86_64-glew
make run
```

The original Visual Studio 2008 solution (`Bump.sln` / `LightEngine.vcproj`) is still in the repo for reference. It now links against SDL2 and GLEW, but it is not maintained.

### Make targets

| Command        | What it does                                        |
| -------------- | --------------------------------------------------- |
| `make`         | Builds the `./bump` binary (`-O2`); objects go to `build/` |
| `make DEBUG=1` | Builds with `-O0 -g`                                |
| `make run`     | Builds, then runs the demo                          |
| `make clean`   | Removes `build/` and `./bump`                       |

Run the binary from the repository root. Shaders and textures are loaded with relative paths (`Data/...`).

### Controls

| Key          | Action |
| ------------ | ------ |
| `Esc`        | Quit   |
| Close window | Quit   |

## How it works

1. **Setup** (`CWindowSDL::Init`, `WindowSDL.cpp`): creates a 700×700 window with an OpenGL context, compiles and links `Data/vertex_shader.vs` + `Data/fragment_shader.fs`, and uploads the textures. The scene uses `Data/ground.bmp` as the base (color) map and `Data/Normal Map.bmp` as the normal map.
2. **Geometry**: a single quad drawn in immediate mode (`glBegin(GL_QUADS)`) under an orthographic projection. Each vertex carries a normal `(0,0,1)` plus two generic attributes, `rm_Tangent = (1,0,0)` and `rm_Binormal = (0,1,0)`, which define the tangent space.
3. **Vertex shader**: builds the tangent/binormal/normal basis with `gl_NormalMatrix`. It then projects the light and view direction vectors into tangent space and passes them on as varyings.
4. **Fragment shader**:
   - reads the normal from the normal map (`rgb * 2 - 1`)
   - computes Phong lighting: ambient + diffuse `N·L` + specular `(R·V)^p`
   - applies distance attenuation `1 / (0.1·d² + 0.2·d + 1)`
5. **Multipass lighting** (`CWindowSDL::Render`): the quad is drawn once per light. The first pass is opaque. Passes 2 and 3 are added on top with `glBlendFunc(GL_ONE, GL_ONE)`, so the three lights add up in the framebuffer. Ambient is added once per pass, which is why the scene is fairly bright.

## Project layout

```
MAIN.cpp                  entry point
WindowSDL.{h,cpp}         window, GL setup, main loop, rendering
graphic/Shader.cpp        GLSL shader loading (class cShader, declared in FrameBufferObject.h)
graphic/LoadImages.*      BMP loading into RGB buffers (SDL_LoadBMP)
graphic/FrameBufferObject FBO wrapper (not used by the demo)
graphic/GLObjects.*       2D helper drawing (not used by the demo)
NormalisationCubeMap.*    normalisation cube map generator (not used by the demo)
leMath.*                  2D vector helpers (not used by the demo)
Math/                     vector / matrix / plane / color classes
Data/                     shaders, textures, normal maps
Makefile                  build for macOS, Linux and MSYS2
Bump.sln, LightEngine.vcproj, Debug/   original Visual Studio 2008 project and build output
```

Several files (FBO, cube map, 2D light helpers, `Data/map.txt`) are carried over from the sibling 2D-lighting "LightEngine" project and are compiled but not used here.

## Porting notes

The original build targeted Windows only: Visual Studio 2008, SDL 1.2 and `glaux` for loading BMP files. The changes needed to build on macOS and Linux:

- SDL 1.2 → SDL2: `SDL_CreateWindow` + `SDL_GL_CreateContext`, `SDL_GL_SwapWindow`.
- `glaux`'s `auxDIBImageLoad` replaced by `SDL_LoadBMP`. Rows are kept bottom-up, the same layout `glaux` returned, so textures keep their original orientation.
- Windows-only headers (`windows.h`, `tchar.h`, `_tmain`) dropped, and include paths fixed for case-sensitive file systems.
- `Normalisation Cube Map.*` renamed to `NormalisationCubeMap.*`, because make does not handle spaces in file names.
- GLSL fix: the vertex shader used float literals with an `f` suffix (`1.0f`). GLSL 1.10 does not allow that, and strict drivers (macOS, Mesa) reject the shader.

## Credits

- `Math/` and the normalisation cube map code are by Paul Baker ([paulsprojects.net](http://www.paulsprojects.net/)), New BSD License.
- The shader structure (`rm_Tangent` / `rm_Binormal`, `fvAmbient`, `fSpecularPower`, ...) follows the AMD RenderMonkey bump mapping example.

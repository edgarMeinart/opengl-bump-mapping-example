# OpenGL bump mapping example — SDL2 + GLEW + legacy OpenGL 2.1 / GLSL 1.10
#
#   make            build ./bump (release)
#   make DEBUG=1    build with -O0 -g
#   make run        build and run (from the repo root, so Data/ resolves)
#   make clean      remove build artifacts

TARGET   := bump
BUILDDIR := build

CXX      ?= c++
PKG_CONFIG ?= pkg-config

SRCS := MAIN.cpp \
        WindowSDL.cpp \
        NormalisationCubeMap.cpp \
        leMath.cpp \
        stdafx.cpp \
        graphic/FrameBufferObject.cpp \
        graphic/GLObjects.cpp \
        graphic/LoadImages.cpp \
        graphic/Shader.cpp \
        Math/COLOR.cpp \
        Math/MATRIX4X4.cpp \
        Math/PLANE.cpp \
        Math/VECTOR2D.cpp \
        Math/VECTOR3D.cpp \
        Math/VECTOR4D.cpp

OBJS := $(SRCS:%.cpp=$(BUILDDIR)/%.o)
DEPS := $(OBJS:.o=.d)

PKGS := sdl2 glew
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
  GL_LIBS  := -framework OpenGL
  CPPFLAGS += -DGL_SILENCE_DEPRECATION
else ifneq (,$(findstring MINGW,$(UNAME_S)))
  GL_LIBS  := -lopengl32
else
  PKGS     += gl
endif

PKG_CFLAGS := $(shell $(PKG_CONFIG) --cflags $(PKGS))
PKG_LIBS   := $(shell $(PKG_CONFIG) --libs $(PKGS))

ifeq ($(DEBUG),1)
  OPTFLAGS := -O0 -g
else
  OPTFLAGS := -O2
endif

CPPFLAGS += -I. $(PKG_CFLAGS)
CXXFLAGS += -std=c++11 $(OPTFLAGS) -Wall -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unknown-warning-option
LDLIBS   += $(PKG_LIBS) $(GL_LIBS)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

-include $(DEPS)

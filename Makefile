# OpenGL bump mapping example: SDL2 + GLEW + OpenGL 2.1 / GLSL 1.10
#
#   make            build build/bump (release)
#   make DEBUG=1    build with -O0 -g
#   make run        build and run
#   make clean      remove build/

TARGET   := build/bump
SRCDIR   := src
BUILDDIR := build

CXX        ?= c++
PKG_CONFIG ?= pkg-config

SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(SRCS:$(SRCDIR)/%.cpp=$(BUILDDIR)/obj/%.o)
DEPS := $(OBJS:.o=.d)

PKGS    := sdl2 glew
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

CPPFLAGS += $(PKG_CFLAGS)
CXXFLAGS += -std=c++11 $(OPTFLAGS) -Wall -Wextra
LDLIBS   += $(PKG_LIBS) $(GL_LIBS)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILDDIR)/obj/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILDDIR)

-include $(DEPS)

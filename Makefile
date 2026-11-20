# OpenGL examples: SDL2 + GLEW + OpenGL 2.1 / GLSL 1.10 + Dear ImGui
#
#   make                    build build/opengl-examples (release)
#   make DEBUG=1            build with -O0 -g
#   make run                build and open the start menu
#   make run EXAMPLE=fire   build and start one example directly (see `make list`)
#   make list               list the examples
#   make compile_flags.txt  write compiler flags for clangd / editors
#   make clean              remove build/

TARGET   := build/opengl-examples
BUILDDIR := build

CXX        ?= c++
PKG_CONFIG ?= pkg-config

IMGUI_DIR  := third_party/imgui
APP_SRCS   := $(wildcard src/*.cpp src/*/*.cpp)
IMGUI_SRCS := $(IMGUI_DIR)/imgui.cpp \
              $(IMGUI_DIR)/imgui_draw.cpp \
              $(IMGUI_DIR)/imgui_tables.cpp \
              $(IMGUI_DIR)/imgui_widgets.cpp \
              $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
              $(IMGUI_DIR)/backends/imgui_impl_opengl2.cpp

APP_OBJS   := $(APP_SRCS:%.cpp=$(BUILDDIR)/obj/%.o)
IMGUI_OBJS := $(IMGUI_SRCS:%.cpp=$(BUILDDIR)/obj/%.o)
DEPS       := $(APP_OBJS:.o=.d) $(IMGUI_OBJS:.o=.d)

PKGS    := sdl2 glew
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
  GL_LIBS  := -framework OpenGL
  APP_DEFS := -DGL_SILENCE_DEPRECATION
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

CPPFLAGS += -Isrc -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends $(PKG_CFLAGS)
CXXFLAGS += -std=c++17 $(OPTFLAGS)
WARNINGS := -Wall -Wextra
LDLIBS   += $(PKG_LIBS) $(GL_LIBS)

.PHONY: all run list clean

all: $(TARGET)

$(TARGET): $(APP_OBJS) $(IMGUI_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Our code: full warnings. Third-party code: built as shipped.
$(APP_OBJS): $(BUILDDIR)/obj/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(APP_DEFS) $(CXXFLAGS) $(WARNINGS) -MMD -MP -c -o $@ $<

$(IMGUI_OBJS): $(BUILDDIR)/obj/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c -o $@ $<

run: $(TARGET)
	./$(TARGET) $(EXAMPLE)

list: $(TARGET)
	./$(TARGET) --list

compile_flags.txt: Makefile
	@printf '%s\n' -xc++ $(CPPFLAGS) $(APP_DEFS) $(CXXFLAGS) $(WARNINGS) > $@

clean:
	rm -rf $(BUILDDIR)

# Flags live in this file, so editing it must rebuild everything.
$(APP_OBJS) $(IMGUI_OBJS): Makefile

-include $(DEPS)

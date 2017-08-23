#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
#
# You will need SDL2 (http://www.libsdl.org)
#
#   apt-get install libsdl2-dev  # Linux
#   brew install sdl2            # Mac OS X
#   pacman -S mingw-w64-i686-SDL # MSYS2
#

#CXX = g++

EXE = mapgen
OBJS = main.o imgui_impl_sdl_gl3.o
OBJS += imgui/imgui.o imgui/imgui_demo.o imgui/imgui_draw.o
OBJS += imgui/examples/libs/gl3w/GL/gl3w.o

UNAME_S := $(shell uname -s)


ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS = -lGL -ldl `sdl2-config --libs`

	CXXFLAGS = -lGL -lGLEW -Iimgui -Iimgui/examples/libs/gl3w `sdl2-config --cflags`
	CXXFLAGS += -Wall -Wformat
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
   ECHO_MESSAGE = "Windows"
   LIBS = -l/usr/include -lGL -lglut -lGLEW -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`

   CXXFLAGS = -Iimgui -Iimgui/examples/libs/gl3w `pkg-config --cflags sdl2`
   CXXFLAGS += -Wall -Wformat
   CFLAGS = $(CXXFLAGS)
endif


.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(CXXFLAGS) $(LIBS)

clean:
	rm $(EXE) $(OBJS)

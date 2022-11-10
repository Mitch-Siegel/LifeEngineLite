CC = g++
OBJDIR = builddir

IMGUI_VERSION = imgui-1.88
IMGUI_SRCS = $(basename $(wildcard $(IMGUI_VERSION)/*.cpp))
IMGUI_OBJS = $(IMGUI_SRCS:$(IMGUI_VERSION)/%=%.o)

IMPLOT_VERSION = implot-0.14
IMPLOT_SRCS = $(basename $(wildcard $(IMPLOT_VERSION)/*.cpp))
IMPLOT_OBJS = $(IMPLOT_SRCS:$(IMPLOT_VERSION)/%=%.o)

BACKEND = $(IMGUI_VERSION)/backends/imgui_impl_sdl.cpp $(IMGUI_VERSION)/backends/imgui_impl_sdlrenderer.cpp
BACKEND_SRCS = $(basename $(BACKEND))
BACKEND_OBJS = $(BACKEND_SRCS:$(IMGUI_VERSION)/backends/%=%.o)

GAME_SRCS = $(basename $(wildcard *.cpp))
GAME_OBJS = $(GAME_SRCS:%=%.o)

CFLAGS_NOWARNINGS = -g -O2
CFLAGS_WARNINGS = $(CFLAGS_NOWARNINGS) -Werror -Wall

OBJS = $(IMGUI_OBJS) $(BACKEND_OBJS) $(IMPLOT_OBJS) main.o organismview.o organism.o cell.o board.o rng.o brain.o util.o

programs: lel

#c++ `sdl2-config --cflags` -I .. -I ../.. main.cpp 
# ../../backends/imgui_impl_sdl.cpp ../../backends/imgui_impl_sdlrenderer.cpp 
# ../../imgui*.cpp `sdl2-config --libs` -lGL
lel: $(addprefix $(OBJDIR)/,$(OBJS))
# @echo Final executable:
	$(CC) $(CFLAGS_WARNINGS)  `sdl2-config --cflags` -o $@ $^ `sdl2-config --libs` -lboost_thread 

$(OBJDIR)/%.o: $(IMGUI_VERSION)/backends/%.cpp
# @echo ImGui backend file:
	$(CC) -c $(CFLAGS_NOWARNINGS) -I./imgui-1.88 `sdl2-config --cflags` -o $@ $< 

$(OBJDIR)/%.o: $(IMGUI_VERSION)/%.cpp
# @echo ImGui library file:
	$(CC) -c $(CFLAGS_NOWARNINGS) -o $@ $<

$(OBJDIR)/%.o: $(IMPLOT_VERSION)/%.cpp
# @echo ImPlot library file:
	$(CC) -c $(CFLAGS_NOWARNINGS) -I./imgui-1.88 -o $@ $<

$(OBJDIR)/%.o: %.cpp
# @echo game file:
	$(CC) -c $(CFLAGS_WARNINGS) -o $@ $<

$(OBJDIR)/main.o: main.cpp
	$(CC) -c $(CFLAGS_WARNINGS) -I./imgui-1.88 -I./imgui-1.88/backends -I./implot-0.14 -o $@ $<

$(OBJDIR)/util.o: util.cpp
	$(CC) -c $(CFLAGS_WARNINGS) -I./imgui-1.88 -I./implot-0.14 -o $@ $<

$(OBJDIR)/organismview.o: organismview.cpp
	$(CC) -c $(CFLAGS_WARNINGS) -I./imgui-1.88 -I./implot-0.14 -o $@ $<

clean:
	rm $(OBJDIR)/*.o
	rm ./lel

cleangame:
# IMGUI_SRCS = $(basename $(wildcard $(IMGUI_VERSION)/*.cpp))
	rm $(addprefix $(OBJDIR)/,$(GAME_OBJS))

test:
	$(info OBJS="$(IMGUI_OBJS)")
	$(info SRCS="$(IMGUI_SRCS)")
	$(info BACKEND_OBJS="$(BACKEND_OBJS)")
	$(info BACKEND_SRCS="$(BACKEND_SRCS)")
	$(info OBJS="$(OBJS)")
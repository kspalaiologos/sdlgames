
# -- Compiler flags
CXX=g++
CXXFLAGS=-Os
LDFLAGS=-lSDL2 -lm -s

# -- Targets
.PHONY: all
all: assets sdlreversi

.PHONY: clean
clean:
	@rm -f assets/*.h assets.h assets.cpp *.o sdlreversi imgui/*.o ai/*.o
	@touch assets.h assets.cpp

.PHONY: format
format:
	clang-format -i *.cpp *.hpp ai/*.cpp ai/*.hpp imgui/*.cpp imgui/*.hpp

# -- Generate assets.h
.PHONY: assets
assets:
	./gen_assets.sh

# -- Source code
OBJ=assets.o reversi.o imgui/imgui_widgets.o imgui/imgui_impl_sdlrenderer2.o \
	imgui/imgui_tables.o imgui/imgui_draw.o imgui/imgui.o \
	imgui/imgui_impl_sdl2.o ai/Egaroucid.o

%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

sdlreversi: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)


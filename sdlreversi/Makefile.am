AM_CXXFLAGS = $(SDL_CFLAGS) $(SDL_ttf_CFLAGS)
AM_CFLAGS = $(SDL_CFLAGS) $(SDL_ttf_CFLAGS)
bin_PROGRAMS = sdlreversi

sdlreversi_SOURCES = reversi.cpp assets.cpp ai/Egaroucid.cpp imgui/imgui.cpp imgui/imgui_impl_sdl2.cpp imgui/imgui_tables.cpp imgui/imgui_draw.cpp imgui/imgui_impl_sdlrenderer2.cpp imgui/imgui_widgets.cpp
sdlreversi_LDADD = $(SDL_LIBS) $(SDL_ttf_LIBS)
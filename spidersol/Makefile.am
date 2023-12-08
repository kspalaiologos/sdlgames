AM_CXXFLAGS = $(SDL_CFLAGS) $(SDL_ttf_CFLAGS)
AM_CFLAGS = $(SDL_CFLAGS) $(SDL_ttf_CFLAGS)
bin_PROGRAMS = spidersol

EXTRA_DIST = gen_assets.sh

spidersol_SOURCES = cntrl.c engine.c infobox.c ui.c win.c deal.c game_state.c spidersol.cpp undo.c ../imgui/imgui.cpp ../imgui/imgui_impl_sdl2.cpp ../imgui/imgui_tables.cpp ../imgui/imgui_draw.cpp ../imgui/imgui_impl_sdlrenderer2.cpp ../imgui/imgui_widgets.cpp
spidersol_LDADD = $(SDL_LIBS) $(SDL_ttf_LIBS)
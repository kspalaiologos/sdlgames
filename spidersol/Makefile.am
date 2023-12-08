AM_CXXFLAGS = $(SDL_CFLAGS) $(SDL_ttf_CFLAGS)
AM_CFLAGS = $(SDL_CFLAGS) $(SDL_ttf_CFLAGS)
bin_PROGRAMS = spidersol

spidersol_SOURCES = cntrl.c engine.c infobox.c ui.c win.c deal.c game_state.c spidersol.cpp undo.c ../imgui/imgui.cpp ../imgui/imgui_impl_sdl2.cpp ../imgui/imgui_tables.cpp ../imgui/imgui_draw.cpp ../imgui/imgui_impl_sdlrenderer2.cpp ../imgui/imgui_widgets.cpp
noinst_HEADERS = cntrl.h deal.h engine.h game_state.h infobox.h ui.h undo.h vector.h win.h ../imgui/imconfig.h ../imgui/imgui.h ../imgui/imgui_impl_sdl2.h ../imgui/imgui_impl_sdlrenderer2.h ../imgui/imgui_internal.h ../imgui/imstb_rectpack.h ../imgui/imstb_textedit.h ../imgui/imstb_truetype.h
spidersol_LDADD = $(SDL_LIBS) $(SDL_ttf_LIBS)
if NETBSD
AM_COMMON = -I/usr/pkg/include -L/usr/pkg/lib -Wl,-R/usr/pkg/lib -I/usr/X11R7/include -L/usr/X11R7/lib
else
AM_COMMON =
endif

EXTRA_DIST = gen_assets.sh

AM_CXXFLAGS = $(AM_COMMON) $(SDL_CFLAGS) $(SDL_ttf_CFLAGS)
AM_CFLAGS = $(AM_COMMON) $(SDL_CFLAGS) $(SDL_ttf_CFLAGS)

bin_PROGRAMS = sdlreversi

sdlreversi_SOURCES = reversi.cpp assets.cpp ai/Egaroucid.cpp ../imgui/imgui.cpp ../imgui/imgui_impl_sdl2.cpp ../imgui/imgui_tables.cpp ../imgui/imgui_draw.cpp ../imgui/imgui_impl_sdlrenderer2.cpp ../imgui/imgui_widgets.cpp
if NETBSD
sdlreversi_LDFLAGS = -Wl,-R/usr/pkg/lib -Wl,-R/usr/X11R7/lib
endif
sdlreversi_LDADD = $(SDL_LIBS) $(SDL_ttf_LIBS)

if NETBSD
AM_COMMON = -I/usr/pkg/include -L/usr/pkg/lib -Wl,-R/usr/pkg/lib -I/usr/X11R7/include -L/usr/X11R7/lib
else
AM_COMMON =
endif

EXTRA_DIST = spidersol/gen_assets.sh sdlmine/gen_assets.sh sdlreversi/gen_assets.sh

AM_CXXFLAGS = $(AM_COMMON) $(SDL_CFLAGS) $(SDL_ttf_CFLAGS) $(SDL_mixer_CFLAGS)
AM_CFLAGS = $(AM_COMMON) $(SDL_CFLAGS) $(SDL_ttf_CFLAGS) $(SDL_mixer_CFLAGS)

bin_PROGRAMS = bin/sdlmine bin/sdlreversi bin/spidersol

IMGUI = imgui/imgui.cpp imgui/imgui_impl_sdl2.cpp imgui/imgui_tables.cpp imgui/imgui_draw.cpp imgui/imgui_impl_sdlrenderer2.cpp imgui/imgui_widgets.cpp
bin_sdlmine_SOURCES = sdlmine/logic.c sdlmine/game_render.c sdlmine/minesweeper.cpp sdlmine/assets.c $(IMGUI)
bin_sdlreversi_SOURCES = sdlreversi/reversi.cpp sdlreversi/assets.cpp sdlreversi/ai/Egaroucid.cpp $(IMGUI)
bin_spidersol_SOURCES = spidersol/cntrl.c spidersol/engine.c spidersol/infobox.c spidersol/ui.c spidersol/win.c spidersol/deal.c spidersol/game_state.c spidersol/spidersol.cpp spidersol/undo.c $(IMGUI)
noinst_HEADERS = spidersol/cntrl.h spidersol/deal.h spidersol/engine.h spidersol/game_state.h spidersol/infobox.h spidersol/ui.h spidersol/undo.h spidersol/vector.h spidersol/win.h sdlmine/assets.h sdlmine/game_render.h sdlmine/logic.h sdlmine/main.h imgui/imconfig.h imgui/imgui.h imgui/imgui_impl_sdl2.h imgui/imgui_impl_sdlrenderer2.h imgui/imgui_internal.h imgui/imstb_rectpack.h imgui/imstb_textedit.h imgui/imstb_truetype.h sdlreversi/ai/ai.hpp sdlreversi/ai/last_flip.hpp sdlreversi/ai/bit.hpp sdlreversi/ai/level.hpp sdlreversi/ai/board.hpp sdlreversi/ai/midsearch.hpp sdlreversi/ai/book_const.hpp sdlreversi/ai/mobility.hpp sdlreversi/ai/book.hpp sdlreversi/ai/move_ordering.hpp sdlreversi/ai/bookrw.hpp sdlreversi/ai/probcut.hpp sdlreversi/ai/common.hpp sdlreversi/ai/search.hpp sdlreversi/ai/endsearch.hpp sdlreversi/ai/setting.hpp sdlreversi/ai/evaluate_const.hpp sdlreversi/ai/stability.hpp sdlreversi/ai/evaluate.hpp sdlreversi/ai/transpose_table.hpp sdlreversi/ai/flip.hpp sdlreversi/ai/util.hpp
if NETBSD
bin_sdlmine_LDFLAGS = -Wl,-R/usr/pkg/lib -Wl,-R/usr/X11R7/lib
bin_sdlreversi_LDFLAGS = -Wl,-R/usr/pkg/lib -Wl,-R/usr/X11R7/lib
bin_spidersol_LDFLAGS = -Wl,-R/usr/pkg/lib -Wl,-R/usr/X11R7/lib
endif
bin_sdlmine_LDADD = $(SDL_LIBS) $(SDL_ttf_LIBS)
bin_sdlreversi_LDADD = $(SDL_LIBS) $(SDL_ttf_LIBS)
bin_spidersol_LDADD = $(SDL_LIBS) $(SDL_ttf_LIBS)

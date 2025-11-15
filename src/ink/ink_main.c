#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

////////////////////////////////
//~ Build options

#define BUILD_TITLE                        "Ink"
#define OS_FEATURE_GRAPHICAL               1
#define OS_FEATURE_AUDIO                   0
#define BUILD_VERSION_MAJOR                0
#define BUILD_VERSION_MINOR                0
#define BUILD_VERSION_PATCH                10
#define BUILD_RELEASE_PHASE_STRING_LITERAL "ALPHA"
#define BUILD_ISSUES_LINK_STRING_LITERAL   "https://github.com/pickle-prick/ink/issues"

////////////////////////////////
//~ Includes

//- external
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "external/stb/stb_image_write.h"

// [h]
#include "base/base_inc.h"
#include "os/os_inc.h"
#include "render/render_inc.h"
#include "font_provider/font_provider_inc.h"
#include "font_cache/font_cache.h"
#include "draw/draw.h"
#include "ui/ui_inc.h"
#include "serialize/serialize_inc.h"
#include "ink_core.h"
// fonts
#if !BUILD_DEBUG
#include "font_icons.h"
#include "font_icons_extra.h"
#include "font_segoeui.h"
#include "font_Mplus1Code-Medium.h"
#include "font_Virgil.h"
#include "font_XiaolaiMono-Regular.h"
#include "font_JetBrainsMono-Regular.h"
#endif

// [c]
#include "base/base_inc.c"
#include "os/os_inc.c"
#include "render/render_inc.c"
#include "font_provider/font_provider_inc.c"
#include "font_cache/font_cache.c"
#include "draw/draw.c"
#include "ui/ui_inc.c"
#include "serialize/serialize_inc.c"
#include "ink_core.c"

internal void
entry_point(CmdLine *cmd_line)
{
  ////////////////////////////////
  //~ Init

  // change working directory
  String8 binary_path = os_get_process_info()->binary_path; // only directory
  {
    Temp scratch = scratch_begin(0,0);
    String8List parts = str8_split_path(scratch.arena, binary_path);
    str8_list_push(scratch.arena, &parts, str8_lit(".."));
    str8_path_list_resolve_dots_in_place(&parts, PathStyle_SystemAbsolute);
    String8 working_directory = push_str8_copy(scratch.arena, str8_path_list_join_by_style(scratch.arena, &parts, PathStyle_SystemAbsolute));
    os_set_current_path(working_directory);
    scratch_end(scratch);
  }

  // seeding
  U32 seed = time(NULL);
  srand(seed);

  // prepare window dim
  F32 ratio = 16.f/10.f;
  OS_Handle monitor = os_primary_monitor();
  Vec2F32 monitor_dim = os_dim_from_monitor(monitor);
  Vec2F32 center = scale_2f32(monitor_dim, 0.5);
  Vec2F32 window_dim = {monitor_dim.y*0.6f*ratio, monitor_dim.y*0.6f};
  Vec2F32 half_window_dim = scale_2f32(window_dim, 0.5);
  Rng2F32 window_rect = {.p0 = sub_2f32(center, half_window_dim), .p1 = add_2f32(center, half_window_dim)};
  // TODO(Next): just a hack for linux, we don't have os_dim_from_monitor implemented yet
  if(window_rect.x1 == 0)
  {
    window_rect = r2f32p(0,0, 1600, 1000);
  }
  String8 window_title = str8_lit(BUILD_TITLE);

  // open window
  OS_Handle os_wnd = os_window_open(window_rect, 0, window_title);
  os_window_first_paint(os_wnd);
  os_window_focus(os_wnd);

  // init main audio device
#if OS_FEATURE_AUDIO
  OS_Handle main_audio_device = os_audio_device_open();
  os_set_main_audio_device(main_audio_device);
  os_audio_device_start(main_audio_device);
  os_audio_set_master_volume(0.9);
#endif

  // renderer initialization
  r_init(os_wnd, BUILD_DEBUG);
  R_Handle r_wnd = r_window_equip(os_wnd);

  // init ui state
  UI_State *ui = ui_state_alloc();
  ui_select_state(ui);

  // init ik state
  ik_init(os_wnd, r_wnd);

  ////////////////////////////////
  //~ Main loop

  B32 open = 1;
  while(open)
  {
    ProfTick(0);
    // FIXME: the order matters, it has something to do with frame index sync
    fnt_frame();
    update_tick_idx();
    open = ik_frame();
  }

  ////////////////////////////////
  //~ Cleanup

  os_window_close(os_wnd);
}

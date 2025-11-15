#ifndef INK_CORE_H
#define INK_CORE_H

////////////////////////////////
//~ Mouse Button Kinds

typedef enum IK_MouseButtonKind
{
  IK_MouseButtonKind_Left,
  IK_MouseButtonKind_Middle,
  IK_MouseButtonKind_Right,
  IK_MouseButtonKind_COUNT
} IK_MouseButtonKind;

////////////////////////////////
//~ Tool Types

typedef enum IK_ToolKind
{
  IK_ToolKind_Hand,
  IK_ToolKind_Selection,
  IK_ToolKind_Rectangle,
  IK_ToolKind_Draw,
  IK_ToolKind_Text,
  IK_ToolKind_Arrow,
  // IK_ToolKind_InsertImage,
  // IK_ToolKind_Eraser,
  IK_ToolKind_Man,
  IK_ToolKind_COUNT,
} IK_ToolKind;

////////////////////////////////
//~ Drag/Drop Types

typedef enum IK_DragDropState
{
  IK_DragDropState_Null,
  IK_DragDropState_Dragging,
  IK_DragDropState_Dropping,
  IK_DragDropState_COUNT
}
IK_DragDropState;

////////////////////////////////
//~ Text

typedef U64 IK_TextAlign;
enum
{
  IK_TextAlign_Left    = (1<<0),
  IK_TextAlign_Right   = (1<<1),
  IK_TextAlign_HCenter = (1<<2),
  IK_TextAlign_Top     = (1<<3),
  IK_TextAlign_Bottom  = (1<<4),
  IK_TextAlign_VCenter = (1<<5),
};

////////////////////////////////
//~ Message

typedef struct IK_Message IK_Message;
struct IK_Message
{
  IK_Message *next;
  IK_Message *prev;
  F32 create_t;
  F32 expired_t;
  F32 elapsed_sec;
  F32 expired;
  String8 string;
};

typedef struct IK_MessageList IK_MessageList;
struct IK_MessageList
{
  IK_Message *first;
  IK_Message *last;
  U64 count;
};

////////////////////////////////
//~ Palettes

typedef enum IK_ColorCode
{
  IK_ColorCode_Null,
  IK_ColorCode_Background,
  IK_ColorCode_Text,
  IK_ColorCode_TextWeak,
  IK_ColorCode_Border,
  IK_ColorCode_Overlay,
  IK_ColorCode_Cursor,
  IK_ColorCode_Selection,
  IK_ColorCode_COUNT
} IK_ColorCode;

typedef struct IK_Palette IK_Palette;
struct IK_Palette
{
  union
  {
    Vec4F32 colors[IK_ColorCode_COUNT];
    struct
    {
      Vec4F32 null;
      Vec4F32 background;
      Vec4F32 text;
      Vec4F32 text_weak;
      Vec4F32 border;
      Vec4F32 overlay;
      Vec4F32 cursor;
      Vec4F32 selection;
    };
  };
};

typedef struct IK_WidgetPaletteInfo IK_WidgetPaletteInfo;
struct IK_WidgetPaletteInfo
{
  IK_Palette *tooltip_palette;
  IK_Palette *ctx_menu_palette;
  IK_Palette *scrollbar_palette;
};

////////////////////////////////
//~ Key

typedef struct IK_Key IK_Key;
struct IK_Key
{
  U64 u64[2];
};

////////////////////////////////
//~ Setting Types

typedef struct IK_SettingVal IK_SettingVal;
struct IK_SettingVal
{
  B32 set;
  S32 s32;
};

////////////////////////////////
//~ Font slot

typedef enum IK_FontSlot
{
  IK_FontSlot_Main,
  IK_FontSlot_Code,
  IK_FontSlot_Icons,
  IK_FontSlot_IconsExtra,
  IK_FontSlot_HandWrite1,
  IK_FontSlot_HandWrite2,
  IK_FontSlot_COUNT
} IK_FontSlot;

////////////////////////////////
//~ Cfg

typedef struct IK_Cfg IK_Cfg;
struct IK_Cfg
{
  F32 stroke_size;
  Vec4F32 stroke_color;
  Vec4F32 background_color;
  IK_FontSlot text_font_slot;
};

////////////////////////////////
//~ String Block

typedef struct IK_Frame IK_Frame;
typedef struct IK_StringBlock IK_StringBlock;
struct IK_StringBlock
{
  IK_StringBlock *free_next;
  IK_StringBlock *free_prev;
  U8 *p;
  U64 cap_bytes;
};

////////////////////////////////
//~ Image

typedef struct IK_Image IK_Image;
struct IK_Image
{
  IK_Key key;
  R_Handle handle;
  // TODO(Next): this could consumes alot memory, page it out to a file, we only need it when doing saving
  String8 encoded; // png or whatever
  U64 x;
  U64 y;
  U64 comp;
  U64 rc;

  void *decoded; // pixel data
  B32 loading; // initial set by main thread, later set by worker thread
  B32 loaded; // set by main thread
};

typedef struct IK_ImageCacheNode IK_ImageCacheNode;
struct IK_ImageCacheNode
{
  IK_ImageCacheNode *next;
  IK_ImageCacheNode *prev;
  IK_Image v;
};

typedef struct IK_ImageCacheSlot IK_ImageCacheSlot;
struct IK_ImageCacheSlot
{
  IK_ImageCacheNode *first;
  IK_ImageCacheNode *last;
};

////////////////////////////////
//~ Image Decode Job Queue

typedef struct IK_ImageDecodeQueue IK_ImageDecodeQueue;
struct IK_ImageDecodeQueue
{
  Semaphore semaphore;
  IK_Image *queue[256]; // cicular buffer
  volatile U64 queue_count;
  U64 cursor; // next to process
  U64 mark; // next to push to queue
};

////////////////////////////////
//~ Camera

typedef struct IK_Camera IK_Camera;
struct IK_Camera
{
  Rng2F32 rect;
  Rng2F32 target_rect;
  F32 zn;
  F32 zf;

  F32 anim_rate;
  F32 min_zoom_step;
  F32 max_zoom_step;
  F32 zoom_t;

  // pan stage
  B32 is_panning;
};

////////////////////////////////
//~ Action Slot (topleft, downright, center)

typedef enum IK_ActionSlot
{
  IK_ActionSlot_Null,
  IK_ActionSlot_TopLeft,
  IK_ActionSlot_DownRight,
  IK_ActionSlot_Center,
  IK_ActionSlot_COUNT,
} IK_ActionSlot;

////////////////////////////////
//~ Drag State

typedef struct IK_BoxDrag IK_BoxDrag;
struct IK_BoxDrag
{
  Rng2F32 drag_start_rect;
  Vec2F32 drag_start_mouse;
  Vec2F32 drag_start_mouse_in_world;
  B32 drag_started;
};

////////////////////////////////
//~ Signal types

typedef U32 IK_SignalFlags;
enum
{
  // mouse press -> box was pressed while hovering
  IK_SignalFlag_LeftPressed         = (1<<0),
  IK_SignalFlag_MiddlePressed       = (1<<1),
  IK_SignalFlag_RightPressed        = (1<<2),

  // dragging -> box was previously pressed, user is still holding button
  IK_SignalFlag_LeftDragging        = (1<<3),
  IK_SignalFlag_MiddleDragging      = (1<<4),
  IK_SignalFlag_RightDragging       = (1<<5),

  // double-dragging -> box was previously double-clicked, user is still holding button
  IK_SignalFlag_LeftDoubleDragging  = (1<<6),
  IK_SignalFlag_MiddleDoubleDragging= (1<<7),
  IK_SignalFlag_RightDoubleDragging = (1<<8),

  // triple-dragging -> box was previously triple-clicked, user is still holding button
  IK_SignalFlag_LeftTripleDragging  = (1<<9),
  IK_SignalFlag_MiddleTripleDragging= (1<<10),
  IK_SignalFlag_RightTripleDragging = (1<<11),

  // released -> box was previously pressed & user released, in or out of bounds
  IK_SignalFlag_LeftReleased        = (1<<12),
  IK_SignalFlag_MiddleReleased      = (1<<13),
  IK_SignalFlag_RightReleased       = (1<<14),

  // clicked -> box was previously pressed & user released, in bounds
  IK_SignalFlag_LeftClicked         = (1<<15),
  IK_SignalFlag_MiddleClicked       = (1<<16),
  IK_SignalFlag_RightClicked        = (1<<17),

  // double clicked -> box was previously clicked, pressed again
  IK_SignalFlag_LeftDoubleClicked   = (1<<18),
  IK_SignalFlag_MiddleDoubleClicked = (1<<19),
  IK_SignalFlag_RightDoubleClicked  = (1<<20),

  // triple clicked -> box was previously clicked twice, pressed again
  IK_SignalFlag_LeftTripleClicked   = (1<<21),
  IK_SignalFlag_MiddleTripleClicked = (1<<22),
  IK_SignalFlag_RightTripleClicked  = (1<<23),

  // keyboard pressed -> box had focus, user activated via their keyboard
  IK_SignalFlag_KeyboardPressed     = (1<<24),

  // passive mouse info
  IK_SignalFlag_Hovering            = (1<<25), // hovering specifically this box
  IK_SignalFlag_MouseOver           = (1<<26), // mouse is over, but may be occluded

  // committing state changes via user interaction
  IK_SignalFlag_Commit              = (1<<27),

  // k: selection
  IK_SignalFlag_Select              = (1<<28),

  // k: delete
  IK_SignalFlag_Delete              = (1<<29),

  // high-level combos
  IK_SignalFlag_Pressed       = IK_SignalFlag_LeftPressed|IK_SignalFlag_KeyboardPressed,
  IK_SignalFlag_Released      = IK_SignalFlag_LeftReleased,
  IK_SignalFlag_Clicked       = IK_SignalFlag_LeftClicked|IK_SignalFlag_KeyboardPressed,
  IK_SignalFlag_DoubleClicked = IK_SignalFlag_LeftDoubleClicked,
  IK_SignalFlag_TripleClicked = IK_SignalFlag_LeftTripleClicked,
  IK_SignalFlag_Dragging      = IK_SignalFlag_LeftDragging,
};

typedef struct IK_Box IK_Box;
typedef struct IK_Signal IK_Signal;
struct IK_Signal
{
  IK_Box *box;
  OS_Modifiers event_flags;
  Vec2S16 scroll;
  IK_SignalFlags f;
};

#define ik_pressed(s)        !!((s).f&IK_SignalFlag_Pressed)
#define ik_clicked(s)        !!((s).f&IK_SignalFlag_Clicked)
#define ik_released(s)       !!((s).f&IK_SignalFlag_Released)
#define ik_double_clicked(s) !!((s).f&IK_SignalFlag_DoubleClicked)
#define ik_triple_clicked(s) !!((s).f&IK_SignalFlag_TripleClicked)
#define ik_middle_clicked(s) !!((s).f&IK_SignalFlag_MiddleClicked)
#define ik_right_clicked(s)  !!((s).f&IK_SignalFlag_RightClicked)
#define ik_dragging(s)       !!((s).f&IK_SignalFlag_Dragging)
#define ik_hovering(s)       !!((s).f&IK_SignalFlag_Hovering)
#define ik_mouse_over(s)     !!((s).f&IK_SignalFlag_MouseOver)
#define ik_committed(s)      !!((s).f&IK_SignalFlag_Commit)

////////////////////////////////
//~ Equipment types

typedef struct IK_Point IK_Point;
struct IK_Point
{
  IK_Point *next;
  IK_Point *prev;
  Vec2F32 position;
};

////////////////////////////////
//~ Box types

typedef U64 IK_BoxFlags;
// interaction
# define IK_BoxFlag_MouseClickable        (IK_BoxFlags)(1ull<<0)
# define IK_BoxFlag_ClickToFocus          (IK_BoxFlags)(1ull<<1)
# define IK_BoxFlag_Scroll                (IK_BoxFlags)(1ull<<2)
# define IK_BoxFlag_Disabled              (IK_BoxFlags)(1ull<<3)
// size
# define IK_BoxFlag_FixedRatio            (IK_BoxFlags)(1ull<<4)
# define IK_BoxFlag_FitViewport           (IK_BoxFlags)(1ull<<5)
# define IK_BoxFlag_FitText               (IK_BoxFlags)(1ull<<6)
# define IK_BoxFlag_FitChildren           (IK_BoxFlags)(1ull<<7)
// draw
# define IK_BoxFlag_DrawBackground        (IK_BoxFlags)(1ull<<8)
# define IK_BoxFlag_DrawBorder            (IK_BoxFlags)(1ull<<9)
# define IK_BoxFlag_DrawDropShadow        (IK_BoxFlags)(1ull<<10)
# define IK_BoxFlag_DrawText              (IK_BoxFlags)(1ull<<11)
# define IK_BoxFlag_DrawImage             (IK_BoxFlags)(1ull<<12)
# define IK_BoxFlag_DrawStroke            (IK_BoxFlags)(1ull<<13)
# define IK_BoxFlag_DrawHotEffects        (IK_BoxFlags)(1ull<<14)
# define IK_BoxFlag_DrawActiveEffects     (IK_BoxFlags)(1ull<<15)
// text
# define IK_BoxFlag_HasDisplayString      (IK_BoxFlags)(1ull<<16)
# define IK_BoxFlag_DrawTextWeak          (IK_BoxFlags)(1ull<<17)
// drag
# define IK_BoxFlag_DragToScaleFontSize   (IK_BoxFlags)(1ull<<18)
# define IK_BoxFlag_DragToScalePoint      (IK_BoxFlags)(1ull<<19)
# define IK_BoxFlag_DragToScaleRectSize   (IK_BoxFlags)(1ull<<20)
# define IK_BoxFlag_DragToPosition        (IK_BoxFlags)(1ull<<21)
// misc
# define IK_BoxFlag_Orphan                (IK_BoxFlags)(1ull<<22) // won't push to the frame's box list 
# define IK_BoxFlag_OmitGroupSelection    (IK_BoxFlags)(1ull<<23)
# define IK_BoxFlag_OmitDeletion          (IK_BoxFlags)(1ull<<24)
# define IK_BoxFlag_DoubleClickToCenter   (IK_BoxFlags)(1ull<<25)
# define IK_BoxFlag_PruneZeroText         (IK_BoxFlags)(1ull<<26)
// TODO(Next): move these into text section
# define IK_BoxFlag_WrapText              (IK_BoxFlags)(1ull<<27)
// TODO(Next): move these into size section
# define IK_BoxFlag_ClampBotTextDimX      (IK_BoxFlags)(1ull<<28)
# define IK_BoxFlag_ClampBotTextDimY      (IK_BoxFlags)(1ull<<29)
// TODO(Next): move these into drawing section
# define IK_BoxFlag_DrawArrow             (IK_BoxFlags)(1ull<<30)
// TODO(Next): move these into dragging section
// TODO(Next): find a better name? 
# define IK_BoxFlag_DragPoint             (IK_BoxFlags)(1ull<<31)
# define IK_BoxFlag_DragToScaleStrokeSize (IK_BoxFlags)(1ull<<32)
# define IK_BoxFlag_OmitCtxMenu           (IK_BoxFlags)(1ull<<33)
# define IK_BoxFlag_DrawKeyOverlay        (IK_BoxFlags)(1ull<<34)
# define IK_BoxFlag_DoubleClickToUnFocus  (IK_BoxFlags)(1ull<<35)
# define IK_BoxFlag_Transparent           (IK_BoxFlags)(1ull<<36) // FIXME: make use of this flag, maybe for blank or select box?

// compound flags
#define IK_BoxFlag_Dragable (IK_BoxFlag_DragToScaleFontSize|IK_BoxFlag_DragToScalePoint|IK_BoxFlag_DragToScaleRectSize|IK_BoxFlag_DragToPosition|IK_BoxFlag_DragToScaleStrokeSize|IK_BoxFlag_DragToScaleStrokeSize)

typedef struct IK_Box IK_Box;
//- draw functions
#define IK_BOX_DRAW(name) void Glue(ik_draw_, name)(IK_Box *box)
IK_BOX_DRAW(text);
IK_BOX_DRAW(stroke);
IK_BOX_DRAW(arrow);

//- update update
#define IK_BOX_UPDATE(name) void Glue(ik_update_, name)(IK_Box *box)
IK_BOX_UPDATE(text);
IK_BOX_UPDATE(stroke);
IK_BOX_UPDATE(arrow);

typedef struct IK_Frame IK_Frame;
struct IK_Box
{
  IK_Key key;
  Vec3F32 key_3f32;
  IK_Frame *frame;

  // Lookup table links
  IK_Box *hash_next;
  IK_Box *hash_prev;

  // list links
  IK_Box *next;
  IK_Box *prev;

  // Group links
  IK_Box *group;
  IK_Box *group_first;
  IK_Box *group_last;
  IK_Box *group_next;
  IK_Box *group_prev;
  U64 group_children_count;

  // Selection link
  IK_Box *select_next;
  IK_Box *select_prev;

  // Per-build equipment
  String8 name;
  U8 _name[512];
  IK_BoxFlags flags;
  Vec2F32 position; // top left
  F32 rotation; // around center, turns
  Vec2F32 rect_size;
  Vec2F32 last_rect_size;
  Vec4F32 background_color;
  Vec4F32 text_color;
  Vec4F32 border_color;
  F32 ratio; // width/height
  OS_Cursor hover_cursor;
  // TODO(k): make it effective
  F32 transparency;
  // F32 squish;
  F32 stroke_size;
  Vec4F32 stroke_color;
  // image
  IK_Image *image;
  // text
  String8 string;
  IK_FontSlot font_slot;
  F32 font_size;
  F32 tab_size;
  FNT_RasterFlags text_raster_flags;
  F32 text_padding;
  IK_TextAlign text_align;
  TxtPt cursor;
  TxtPt mark;
  // point
  IK_Point *first_point;
  IK_Point *last_point;
  U64 point_count;

  // Per-frmae artifacts
  String8List display_lines;
  // D_FancyStringList *display_line_fstrs; // TODO: support fancy string
  DR_FRunList *display_line_fruns;
  DR_FRunList empty_fruns;
  IK_Signal sig;
  U64 draw_frame_index;
  Vec2F32 text_bounds;
  // TODO(Next): we may remove this
  void *draw_data;
  B32 deleted; // TODO(Next): not the prettiest thing in the world, find a better way

  // Animation
  F32 hot_t;
  F32 active_t;
  F32 disabled_t;
  F32 focus_hot_t;
  F32 focus_active_t;
};

typedef struct IK_BoxRec IK_BoxRec;
struct IK_BoxRec
{
  IK_Box *next;
  S32 push_count;
  S32 pop_count;
};

typedef struct IK_BoxList IK_BoxList;
struct IK_BoxList
{
  IK_Box *first;
  IK_Box *last;
  U64 count;
};

////////////////////////////////
//- Action

typedef enum IK_ActionKind
{
  IK_ActionKind_Create,
  IK_ActionKind_Delete,
  // IK_ActionKind_Edit,
  IK_ActionKind_COUNT,
} IK_ActionKind;

typedef struct IK_Action IK_Action;
struct IK_Action
{
  IK_ActionKind kind;
  union
  {
    struct
    {
      IK_Box *first;
    } create;

    struct
    {
      IK_Box *first;
    } delete;
  } v;
};

typedef struct IK_ActionRing IK_ActionRing;
struct IK_ActionRing
{
  U64 head; // next write pos
  U64 mark; // last write pos
  U64 tail; // oldest write pos
  IK_Action *slots;
  U64 slot_count;
  U64 write_count; // diff between head and tail
};

////////////////////////////////
//- Frame

typedef struct IK_BoxHashSlot IK_BoxHashSlot;
struct IK_BoxHashSlot
{
  IK_Box *hash_first;
  IK_Box *hash_last;
};

typedef struct IK_Frame IK_Frame;
struct IK_Frame
{
  Arena *arena;
  U64 arena_clear_pos;
  IK_Frame *next;
  IK_BoxList box_list;
  IK_Camera camera;
  // TODO(Next): can we move those two out of frame? it's weird 
  IK_Box *blank;
  IK_Box *select;

  IK_Cfg cfg;
  String8 save_path;
  U8 _save_path[512];
  B32 did_backup;

  // lookup table
  IK_BoxHashSlot box_table[1024];

  // image cache
  IK_ImageCacheSlot image_cache_table[1024];

  // action ring buffer
  IK_ActionRing action_ring;

  // free stack
  IK_Box *first_free_box;
  IK_StringBlock *first_free_string_block;
  IK_StringBlock *last_free_string_block;
  IK_ImageCacheNode *first_free_image_cache_node;
  IK_Point *first_free_point;
};

/////////////////////////////////
//~ Generated code

#include "generated/ink.meta.h"

/////////////////////////////////
//~ Theme Types 

typedef struct IK_Theme IK_Theme;
struct IK_Theme
{
  Vec4F32 colors[IK_ThemeColor_COUNT];
};

typedef enum IK_PaletteCode
{
  IK_PaletteCode_Base,
  IK_PaletteCode_MenuBar,
  IK_PaletteCode_Floating,
  IK_PaletteCode_ImplicitButton,
  IK_PaletteCode_PlainButton,
  IK_PaletteCode_PositivePopButton,
  IK_PaletteCode_NegativePopButton,
  IK_PaletteCode_NeutralPopButton,
  IK_PaletteCode_ScrollBarButton,
  IK_PaletteCode_Tab,
  IK_PaletteCode_TabInactive,
  IK_PaletteCode_DropSiteOverlay,
  IK_PaletteCode_COUNT
} IK_PaletteCode;

/////////////////////////////////
//~ Dynamic drawing (in immediate mode fashion)

typedef struct IK_DrawNode IK_DrawNode;
struct IK_DrawNode
{
  IK_DrawNode *next;
  IK_DrawNode *draw_next;
  IK_DrawNode *draw_prev;

  R_GeoPolygonKind polygon;
  R_GeoTopologyKind topology;

  // vertex
  // src
  R_Geo3D_Vertex *vertices_src;
  U64 vertex_count;
  // dst
  R_Handle vertices;
  U64 vertices_buffer_offset;

  // indice
  // src
  U32 *indices_src;
  U64 indice_count;
  // dst
  R_Handle indices;
  U64 indices_buffer_offset;

  // for font altas
  R_Handle albedo_tex;
};

typedef struct IK_DrawList IK_DrawList;
struct IK_DrawList
{
  // per-build
  IK_DrawNode *first_node;
  IK_DrawNode *last_node;
  U64 node_count;
  U64 vertex_buffer_cmt;
  U64 indice_buffer_cmt;

  // persistent
  R_Handle vertices;
  R_Handle indices;
  U64 vertex_buffer_cap;
  U64 indice_buffer_cap;
};

////////////////////////////////
//~ State 

typedef struct IK_State IK_State;
struct IK_State
{
  Arena                 *arena;
  Arena                 *frame_arenas[2];

  R_Handle              r_wnd;
  OS_Handle             os_wnd;
  UI_EventList          *events;

  // debug
  B32                   show_stats;
  B32                   show_man_page;

  IK_Frame              *active_frame;

  // drawing buckets
  DR_Bucket             *bucket_ui;
  DR_Bucket             *bucket_main;

  // drawlists
  IK_DrawList           *drawlists[2];

  // frame history info
  U64                   frame_index;
  U64                   frame_time_us_history[64];
  F64                   time_in_seconds;
  U64                   time_in_us;

  // frame parameters
  F32                   frame_dt;
  F32                   cpu_time_us;
  F32                   pre_cpu_time_us;

  // window
  Rng2F32               window_rect;
  Vec2F32               window_dim;
  Rng2F32               last_window_rect;
  Vec2F32               last_window_dim;
  B32                   window_res_changed;
  F32                   dpi;
  F32                   last_dpi;
  B32                   window_should_close;

  // messages
  Arena                 *message_arena;
  IK_MessageList        messages;

  // camera
  Mat4x4F32             proj_mat;
  Mat4x4F32             proj_mat_inv;
  Vec2F32               world_to_screen_ratio;

  // mouse
  Vec2F32               mouse;
  Vec2F32               mouse_in_world;
  Vec2F32               last_mouse;
  Vec2F32               mouse_delta; // frame delta
  Vec2F32               mouse_delta_in_world;
  B32                   cursor_hidden;

  // image decode queue & threads
  IK_ImageDecodeQueue   decode_queue;
  OS_Handle             decode_threads[3];

  // palette
  // IK_IconInfo           icon_info;
  IK_WidgetPaletteInfo  widget_palette_info;

  // user interaction state
  IK_Key                hot_box_key;
  IK_Key                active_box_key[IK_MouseButtonKind_COUNT];
  IK_ActionSlot         action_slot;
  IK_Key                focus_hot_box_key[IK_MouseButtonKind_COUNT];
  IK_Key                focus_active_box_key;
  IK_Key                pixel_hot_key; // hot pixel key from renderer

  U64                   press_timestamp_history_us[IK_MouseButtonKind_COUNT][3];
  IK_Key                press_key_history[IK_MouseButtonKind_COUNT][3];
  Vec2F32               press_pos_history[IK_MouseButtonKind_COUNT][3];

  B32                   g_ctx_menu_open;
  Vec2F32               drag_start_mouse;
  Arena                 *drag_state_arena;
  String8               drag_state_data;
  Arena                 *box_scratch_arena; // e.g. edit buffer
  Rng2F32               selection_rect;
  IK_Box                *first_box_selected;
  IK_Box                *last_box_selected;
  Rng2F32               selection_bounds;
  U64                   selected_box_count;
  // clipboard
  IK_Key                last_box_key_copied;

  // toolbar
  IK_ToolKind           tool;

  // drag/drop state
  IK_DragDropState      drag_drop_state;
  U64                   drag_drop_slot;
  void                  *drag_drop_src;

  // theme
  IK_Theme              cfg_theme_target;
  IK_Theme              cfg_theme;
  FNT_Tag               cfg_font_tags[IK_FontSlot_COUNT];

  // post effects
  B32                   crt_enabled;

  // palette
  UI_Palette            cfg_ui_debug_palettes[IK_PaletteCode_COUNT]; // derivative from theme
  IK_Palette            cfg_main_palettes[IK_PaletteCode_COUNT]; // derivative from theme

  // global Settings
  IK_SettingVal         setting_vals[IK_SettingCode_COUNT];

  // views (UI)
  // IK_View               views[IK_ViewKind_COUNT];
  // B32                   views_enabled[IK_ViewKind_COUNT];

  // animation
  struct
  {
    F32 vast_rate;
    F32 fast_rate;
    F32 fish_rate;
    F32 slow_rate;
    F32 slug_rate;
    F32 slaf_rate;
  } animation;

  // free stacks
  IK_Frame *first_free_frame;

  IK_DeclStackNils;
  IK_DeclStacks;
};

/////////////////////////////////
//~ Globals

global IK_State *ik_state;

/////////////////////////////////
//~ Basic Type Functions

internal U64     ik_hash_from_string(U64 seed, String8 string);
internal String8 ik_hash_part_from_key_string(String8 string);
internal String8 ik_display_part_from_key_string(String8 string);

/////////////////////////////////
//~ Key

internal IK_Key ik_key_from_string(IK_Key seed, String8 string);
internal IK_Key ik_key_from_stringf(IK_Key seed, char* fmt, ...);
internal inline B32 ik_key_match(IK_Key a, IK_Key b);
internal inline IK_Key ik_key_make(U64 a, U64 b);
internal inline IK_Key ik_key_zero();
internal IK_Key ik_key_new();
internal Vec3F32 ik_3f32_from_key(IK_Key key);
internal IK_Key ik_key_from_3f32(Vec3F32 key_3f32);

/////////////////////////////////
//~ Handle

// internal IK_Handle ik_handle_zero();
// internal B32       ik_handle_match(IK_Handle a, IK_Handle b);

/////////////////////////////////
//~ State accessor/mutator

//- init
internal void ik_init(OS_Handle os_wnd, R_Handle r_wnd);

//- key
internal IK_Key ik_hot_key(void);
internal IK_Key ik_active_key(IK_MouseButtonKind button_kind);
internal IK_Key ik_drop_hot_key(void);

//- interaction
internal void ik_kill_action(void);
internal void ik_kill_focus(void);

//- frame
internal B32          ik_frame(void);
internal Arena*       ik_frame_arena(void);
internal IK_DrawList* ik_frame_drawlist(void);

//- editor
internal IK_ToolKind ik_tool(void);

//- color
internal Vec4F32 ik_rgba_from_theme_color(IK_ThemeColor color);

//- code -> palette
internal IK_Palette* ik_palette_from_code(IK_PaletteCode code);

//- code -> ui palette
internal UI_Palette* ik_ui_palette_from_code(IK_PaletteCode code);

//- fonts/size
internal FNT_Tag ik_font_from_slot(IK_FontSlot slot);
internal F32     ik_font_size_from_slot(IK_FontSlot slot);

//- box lookup
internal IK_Box* ik_box_from_key(IK_Key key);

//- drag data
internal Vec2F32 ik_drag_start_mouse(void);
internal Vec2F32 ik_drag_delta(void);
internal void    ik_store_drag_data(String8 string);
internal String8 ik_get_drag_data(U64 min_required_size);
#define ik_store_drag_struct(ptr) ik_store_drag_data(str8_struct(ptr))
#define ik_get_drag_struct(type) ((type *)ik_get_drag_data(sizeof(type)).str)

//- selecting
internal inline B32 ik_is_selecting(void);

/////////////////////////////////
//~ Clipboard Functions

internal B32 ik_paste();

/////////////////////////////////
//~ OS event consumption helpers

internal B32 ik_pasted(void);
internal B32 ik_copied(void);
internal String8List ik_file_drop(Vec2F32 *return_mouse);
// TODO(k): for now, we just reuse the ui eat consumption helpers, since we are using the same event list
// internal void ik_eat_event_node(UI_EventList *list, UI_EventNode *node);
// internal B32  ik_key_press(OS_Modifiers mods, OS_Key key);
// internal B32  ik_key_release(OS_Modifiers mods, OS_Key key);

/////////////////////////////////
//~ Dynamic drawing (in immediate mode fashion)

//- basic building API
internal IK_DrawList* ik_drawlist_alloc(Arena *arena, U64 vertex_buffer_cap, U64 indice_buffer_cap);
internal IK_DrawNode* ik_drawlist_push(Arena *arena, IK_DrawList *drawlist, R_Geo3D_Vertex *vertices_src, U64 vertex_count, U32 *indices_src, U64 indice_count);
internal void         ik_drawlist_build(IK_DrawList *drawlist); /* upload buffer from cpu to gpu */
internal void         ik_drawlist_reset(IK_DrawList *drawlist);

//- high level building API
internal IK_DrawNode* ik_drawlist_push_rect(Arena *arena, IK_DrawList *drawlist, Rng2F32 dst, Rng2F32 src);

/////////////////////////////////
//~ Message Functions

internal IK_Message* ik_message_push(String8 string);

/////////////////////////////////
//~ String Block Functions

internal String8 ik_push_str8_copy(String8 src);
internal String8 ik_str8_new(U64 size);
internal void    ik_string_block_release(String8 string);

/////////////////////////////////
//~ Box Type Functions

internal IK_BoxRec ik_box_rec_df(IK_Box *box, IK_Box *root, U64 sib_member_off, U64 child_member_off);
#define ik_box_rec_df_pre(box, root) ik_box_rec_df(box, root, OffsetOf(IK_Box, next), OffsetOf(IK_Box, first))
#define ik_box_rec_df_post(box, root) ik_box_rec_df(box, root, OffsetOf(IK_Box, prev), OffsetOf(IK_Box, last))

/////////////////////////////////
//~ Frame Building API

internal IK_Frame* ik_frame_alloc();
internal void ik_frame_release(IK_Frame *frame);

/////////////////////////////////
//~ Cfg Functions

internal IK_Cfg ik_cfg_default();

/////////////////////////////////
//~ Box Tree Building API

//- box node construction
internal IK_Box* ik_build_box_from_key_(UI_BoxFlags flags, IK_Key key, B32 pre_order);
#define ik_build_box_from_key(f,k) ik_build_box_from_key_((f), (k), 1)
internal IK_Key  ik_active_seed_key(void);
internal IK_Box* ik_build_box_from_string(IK_BoxFlags flags, String8 string);
internal IK_Box* ik_build_box_from_stringf(IK_BoxFlags flags, char *fmt, ...);
internal IK_Box* ik_box_clone(IK_Box *src);

//- box node destruction
internal void ik_box_release(IK_Box *box);

//- box node equipment
internal void    ik_box_equip_name(IK_Box *box, String8 name);
internal String8 ik_box_equip_display_string(IK_Box *box, String8 string);

/////////////////////////////////
//~ High Level Box Building

internal IK_Box* ik_text(String8 string, Vec2F32 pos);
internal IK_Box* ik_image(IK_BoxFlags flags, Vec2F32 pos, Vec2F32 rect_size, IK_Image *image);
internal IK_Box* ik_stroke(void);
internal IK_Box* ik_arrow(void);

/////////////////////////////////
//~ Box Manipulation Functions

internal void ik_box_do_scale(IK_Box *box, Vec2F32 scale, Vec2F32 origin);
internal void ik_box_do_translate(IK_Box *box, Vec2F32 translate);

/////////////////////////////////
//~ Action Type Functions

//- ring buffer operations
internal IK_Action *ik_action_ring_write();
internal IK_Action *ik_action_ring_backward();
internal IK_Action *ik_action_ring_forward();

//- action undo/redo
internal void ik_action_undo(IK_Action *action);
internal void ik_action_redo(IK_Action *action);

//- state undo/redo
internal B32 ik_undo();
internal B32 ik_redo();

/////////////////////////////////
//~ Point Function

internal IK_Point* ik_point_alloc();
internal void      ik_point_release(IK_Point *point);

/////////////////////////////////
//~ Image Function

internal String8   ik_decoded_from_bytes(Arena *arena, String8 bytes, Vec3F32 *return_size);
internal IK_Image* ik_image_from_key(IK_Key key);
internal IK_Image* ik_image_push(IK_Key key);

/////////////////////////////////
//~ Image Decode

internal void ik_image_decode_thread__entry_point(void *ptr);
internal void ik_image_decode_queue_push(IK_Image *image);

/////////////////////////////////
//~ User interaction

internal IK_Signal ik_signal_from_box(IK_Box *box);

/////////////////////////////////
//~ Rendering

//- text

internal void ik_fancy_run_list(Vec2F32 p, DR_FRunList *list, F32 max_x);

/////////////////////////////////
//~ UI Widget

internal void      ik_ui_stats(void);
internal void      ik_ui_man_page(void);
internal void      ik_ui_toolbar(void);
internal void      ik_ui_selection(void);
internal void      ik_ui_inspector(void);
internal void      ik_ui_bottom_bar(void);
internal void      ik_ui_notification(void);
internal void      ik_ui_box_ctx_menu(void);
internal void      ik_ui_g_ctx_menu(void);
internal void      ik_ui_version(void);
internal UI_Signal ik_ui_checkbox(String8 key_string, B32 b);
internal UI_Signal ik_ui_button(String8 string);
internal UI_Signal ik_ui_buttonf(char *fmt, ...);
internal UI_Signal ik_ui_slider_f32(String8 string, F32 *value, F32 px_to_val);
internal UI_Signal ik_ui_range_slider_f32(String8 string, F32 *value, F32 max, F32 min);
internal void      ik_ui_color_palette(String8 string, Vec4F32 *colors, U64 color_count, Vec4F32 *current);

/////////////////////////////////
//~ Text Operation Functions

internal UI_TxtOp ik_multi_line_txt_op_from_event(Arena *arena, UI_Event *event, String8 string, TxtPt cursor, TxtPt mark);
internal Rng1U64  ik_replace_range_from_txtrng(String8 string, TxtRng txt_rng);

/////////////////////////////////
//~ Scene serialization/deserialization

//- frame
internal String8   ik_frame_to_tyml(IK_Frame *frame);
internal IK_Frame* ik_frame_from_tyml(String8 path);

//- image
internal void      ik_image_to_png_file(IK_Image *image, String8 path);

/////////////////////////////////
//~ Helpers

#define ik_rect_from_box(b) ((Rng2F32){b->position.x, b->position.y, b->position.x+b->rect_size.x, b->position.y+b->rect_size.y})

// projection
internal Vec2F32 ik_screen_pos_in_world(Mat4x4F32 proj_view_mat_inv, Vec2F32 pos);
internal Vec2F32 ik_screen_pos_from_world(Vec2F32 pos);
#define ik_mouse_in_world(pvmi) ik_screen_pos_in_world(pvmi, ik_state->mouse)

// encode
internal String8 ik_b64string_from_bytes(Arena *arena, String8 src);
internal String8 ik_bytes_from_b64string(Arena *arena, String8 src);

////////////////////////////////
//~ Macro Loop Wrappers

#define IK_Parent(v) DeferLoop(ik_push_parent(v), ik_pop_parent())
#define IK_Frame(v) DeferLoop(ik_push_frame(v), ik_pop_frame())

#endif

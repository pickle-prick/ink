// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

//- GENERATED CODE

#ifndef INK_META_H
#define INK_META_H

typedef enum IK_IconKind
{
IK_IconKind_Null,
IK_IconKind_FolderOpenOutline,
IK_IconKind_FolderClosedOutline,
IK_IconKind_FolderOpenFilled,
IK_IconKind_FolderClosedFilled,
IK_IconKind_FileOutline,
IK_IconKind_FileFilled,
IK_IconKind_Play,
IK_IconKind_PlayStepForward,
IK_IconKind_Pause,
IK_IconKind_Stop,
IK_IconKind_Info,
IK_IconKind_WarningSmall,
IK_IconKind_WarningBig,
IK_IconKind_Unlocked,
IK_IconKind_Locked,
IK_IconKind_LeftArrow,
IK_IconKind_RightArrow,
IK_IconKind_UpArrow,
IK_IconKind_DownArrow,
IK_IconKind_Gear,
IK_IconKind_Pencil,
IK_IconKind_Trash,
IK_IconKind_Pin,
IK_IconKind_RadioHollow,
IK_IconKind_RadioFilled,
IK_IconKind_CheckHollow,
IK_IconKind_CheckFilled,
IK_IconKind_LeftCaret,
IK_IconKind_RightCaret,
IK_IconKind_UpCaret,
IK_IconKind_DownCaret,
IK_IconKind_UpScroll,
IK_IconKind_DownScroll,
IK_IconKind_LeftScroll,
IK_IconKind_RightScroll,
IK_IconKind_Add,
IK_IconKind_Minus,
IK_IconKind_Thread,
IK_IconKind_Threads,
IK_IconKind_Machine,
IK_IconKind_CircleFilled,
IK_IconKind_X,
IK_IconKind_Refresh,
IK_IconKind_Undo,
IK_IconKind_Redo,
IK_IconKind_Save,
IK_IconKind_Window,
IK_IconKind_Target,
IK_IconKind_Clipboard,
IK_IconKind_Scheduler,
IK_IconKind_Module,
IK_IconKind_XSplit,
IK_IconKind_YSplit,
IK_IconKind_ClosePanel,
IK_IconKind_StepInto,
IK_IconKind_StepOver,
IK_IconKind_StepOut,
IK_IconKind_Find,
IK_IconKind_Palette,
IK_IconKind_Thumbnails,
IK_IconKind_Glasses,
IK_IconKind_Binoculars,
IK_IconKind_List,
IK_IconKind_Grid,
IK_IconKind_QuestionMark,
IK_IconKind_Person,
IK_IconKind_Briefcase,
IK_IconKind_Dot,
IK_IconKind_COUNT,
} IK_IconKind;

typedef enum IK_ThemeColor
{
IK_ThemeColor_Null,
IK_ThemeColor_Text,
IK_ThemeColor_TextPositive,
IK_ThemeColor_TextNegative,
IK_ThemeColor_TextNeutral,
IK_ThemeColor_TextWeak,
IK_ThemeColor_Cursor,
IK_ThemeColor_CursorInactive,
IK_ThemeColor_Focus,
IK_ThemeColor_Hover,
IK_ThemeColor_DropShadow,
IK_ThemeColor_DisabledOverlay,
IK_ThemeColor_DropSiteOverlay,
IK_ThemeColor_InactivePanelOverlay,
IK_ThemeColor_SelectionOverlay,
IK_ThemeColor_HighlightOverlay,
IK_ThemeColor_HighlightOverlayError,
IK_ThemeColor_BaseBackground,
IK_ThemeColor_BaseBackgroundAlt,
IK_ThemeColor_BaseBorder,
IK_ThemeColor_MenuBarBackground,
IK_ThemeColor_MenuBarBackgroundAlt,
IK_ThemeColor_MenuBarBorder,
IK_ThemeColor_FloatingBackground,
IK_ThemeColor_FloatingBackgroundAlt,
IK_ThemeColor_FloatingBorder,
IK_ThemeColor_ImplicitButtonBackground,
IK_ThemeColor_ImplicitButtonBorder,
IK_ThemeColor_PlainButtonBackground,
IK_ThemeColor_PlainButtonBorder,
IK_ThemeColor_PositivePopButtonBackground,
IK_ThemeColor_PositivePopButtonBorder,
IK_ThemeColor_NegativePopButtonBackground,
IK_ThemeColor_NegativePopButtonBorder,
IK_ThemeColor_NeutralPopButtonBackground,
IK_ThemeColor_NeutralPopButtonBorder,
IK_ThemeColor_ScrollBarButtonBackground,
IK_ThemeColor_ScrollBarButtonBorder,
IK_ThemeColor_TabBackground,
IK_ThemeColor_TabBorder,
IK_ThemeColor_TabBackgroundInactive,
IK_ThemeColor_TabBorderInactive,
IK_ThemeColor_CodeDefault,
IK_ThemeColor_CodeSymbol,
IK_ThemeColor_CodeType,
IK_ThemeColor_CodeLocal,
IK_ThemeColor_CodeRegister,
IK_ThemeColor_CodeKeyword,
IK_ThemeColor_CodeDelimiterOperator,
IK_ThemeColor_CodeNumeric,
IK_ThemeColor_CodeNumericAltDigitGroup,
IK_ThemeColor_CodeString,
IK_ThemeColor_CodeMeta,
IK_ThemeColor_CodeComment,
IK_ThemeColor_CodeLineNumbers,
IK_ThemeColor_CodeLineNumbersSelected,
IK_ThemeColor_LineInfoBackground0,
IK_ThemeColor_LineInfoBackground1,
IK_ThemeColor_LineInfoBackground2,
IK_ThemeColor_LineInfoBackground3,
IK_ThemeColor_LineInfoBackground4,
IK_ThemeColor_LineInfoBackground5,
IK_ThemeColor_LineInfoBackground6,
IK_ThemeColor_LineInfoBackground7,
IK_ThemeColor_Thread0,
IK_ThemeColor_Thread1,
IK_ThemeColor_Thread2,
IK_ThemeColor_Thread3,
IK_ThemeColor_Thread4,
IK_ThemeColor_Thread5,
IK_ThemeColor_Thread6,
IK_ThemeColor_Thread7,
IK_ThemeColor_ThreadUnwound,
IK_ThemeColor_ThreadError,
IK_ThemeColor_Breakpoint,
IK_ThemeColor_CacheLineBoundary,
IK_ThemeColor_COUNT,
} IK_ThemeColor;

typedef enum IK_ThemePreset
{
IK_ThemePreset_DefaultDark,
IK_ThemePreset_DefaultLight,
IK_ThemePreset_VSDark,
IK_ThemePreset_VSLight,
IK_ThemePreset_SolarizedDark,
IK_ThemePreset_SolarizedLight,
IK_ThemePreset_HandmadeHero,
IK_ThemePreset_FourCoder,
IK_ThemePreset_FarManager,
IK_ThemePreset_COUNT,
} IK_ThemePreset;

typedef enum IK_SettingCode
{
IK_SettingCode_HoverAnimations,
IK_SettingCode_PressAnimations,
IK_SettingCode_FocusAnimations,
IK_SettingCode_TooltipAnimations,
IK_SettingCode_MenuAnimations,
IK_SettingCode_ScrollingAnimations,
IK_SettingCode_TabWidth,
IK_SettingCode_MainFontSize,
IK_SettingCode_CodeFontSize,
IK_SettingCode_COUNT,
} IK_SettingCode;

typedef struct IK_FrameNode IK_FrameNode; struct IK_FrameNode{IK_FrameNode *next; IK_Frame * v;};
typedef struct IK_FontNode IK_FontNode; struct IK_FontNode{IK_FontNode *next; FNT_Tag v;};
typedef struct IK_FontSlotNode IK_FontSlotNode; struct IK_FontSlotNode{IK_FontSlotNode *next; IK_FontSlot v;};
typedef struct IK_FontSizeNode IK_FontSizeNode; struct IK_FontSizeNode{IK_FontSizeNode *next; F32 v;};
typedef struct IK_TextRasterFlagsNode IK_TextRasterFlagsNode; struct IK_TextRasterFlagsNode{IK_TextRasterFlagsNode *next; FNT_RasterFlags v;};
typedef struct IK_TabSizeNode IK_TabSizeNode; struct IK_TabSizeNode{IK_TabSizeNode *next; F32 v;};
typedef struct IK_BackgroundColorNode IK_BackgroundColorNode; struct IK_BackgroundColorNode{IK_BackgroundColorNode *next; Vec4F32 v;};
typedef struct IK_StrokeSizeNode IK_StrokeSizeNode; struct IK_StrokeSizeNode{IK_StrokeSizeNode *next; F32 v;};
typedef struct IK_StrokeColorNode IK_StrokeColorNode; struct IK_StrokeColorNode{IK_StrokeColorNode *next; Vec4F32 v;};
typedef struct IK_TransparencyNode IK_TransparencyNode; struct IK_TransparencyNode{IK_TransparencyNode *next; F32 v;};
typedef struct IK_PaletteNode IK_PaletteNode; struct IK_PaletteNode{IK_PaletteNode *next; IK_Palette*     v;};
typedef struct IK_FlagsNode IK_FlagsNode; struct IK_FlagsNode{IK_FlagsNode *next; IK_BoxFlags v;};
typedef struct IK_HoverCursorNode IK_HoverCursorNode; struct IK_HoverCursorNode{IK_HoverCursorNode *next; OS_Cursor v;};
typedef struct IK_TextPaddingNode IK_TextPaddingNode; struct IK_TextPaddingNode{IK_TextPaddingNode *next; F32 v;};
typedef struct IK_TextAlignmentNode IK_TextAlignmentNode; struct IK_TextAlignmentNode{IK_TextAlignmentNode *next; IK_TextAlign v;};
#define IK_DeclStackNils \
struct\
{\
IK_FrameNode frame_nil_stack_top;\
IK_FontNode font_nil_stack_top;\
IK_FontSlotNode font_slot_nil_stack_top;\
IK_FontSizeNode font_size_nil_stack_top;\
IK_TextRasterFlagsNode text_raster_flags_nil_stack_top;\
IK_TabSizeNode tab_size_nil_stack_top;\
IK_BackgroundColorNode background_color_nil_stack_top;\
IK_StrokeSizeNode stroke_size_nil_stack_top;\
IK_StrokeColorNode stroke_color_nil_stack_top;\
IK_TransparencyNode transparency_nil_stack_top;\
IK_PaletteNode palette_nil_stack_top;\
IK_FlagsNode flags_nil_stack_top;\
IK_HoverCursorNode hover_cursor_nil_stack_top;\
IK_TextPaddingNode text_padding_nil_stack_top;\
IK_TextAlignmentNode text_alignment_nil_stack_top;\
}
#define IK_InitStackNils(state) \
state->frame_nil_stack_top.v = 0;\
state->font_nil_stack_top.v = fnt_tag_zero();\
state->font_slot_nil_stack_top.v = IK_FontSlot_Main;\
state->font_size_nil_stack_top.v = 24.f;\
state->text_raster_flags_nil_stack_top.v = FNT_RasterFlag_Hinted;\
state->tab_size_nil_stack_top.v = 24.f*4.f;\
state->background_color_nil_stack_top.v = rgba_from_u32(0xEEE6CAFF);\
state->stroke_size_nil_stack_top.v = 8;\
state->stroke_color_nil_stack_top.v = rgba_from_u32(0xEEE6CAFF);\
state->transparency_nil_stack_top.v = 0;\
state->palette_nil_stack_top.v = 0;\
state->flags_nil_stack_top.v = 0;\
state->hover_cursor_nil_stack_top.v = OS_Cursor_Pointer;\
state->text_padding_nil_stack_top.v = 0;\
state->text_alignment_nil_stack_top.v = IK_TextAlign_Left;\

#define IK_DeclStacks \
struct\
{\
struct { IK_FrameNode *top; IK_Frame * bottom_val; IK_FrameNode *free; B32 auto_pop; } frame_stack;\
struct { IK_FontNode *top; FNT_Tag bottom_val; IK_FontNode *free; B32 auto_pop; } font_stack;\
struct { IK_FontSlotNode *top; IK_FontSlot bottom_val; IK_FontSlotNode *free; B32 auto_pop; } font_slot_stack;\
struct { IK_FontSizeNode *top; F32 bottom_val; IK_FontSizeNode *free; B32 auto_pop; } font_size_stack;\
struct { IK_TextRasterFlagsNode *top; FNT_RasterFlags bottom_val; IK_TextRasterFlagsNode *free; B32 auto_pop; } text_raster_flags_stack;\
struct { IK_TabSizeNode *top; F32 bottom_val; IK_TabSizeNode *free; B32 auto_pop; } tab_size_stack;\
struct { IK_BackgroundColorNode *top; Vec4F32 bottom_val; IK_BackgroundColorNode *free; B32 auto_pop; } background_color_stack;\
struct { IK_StrokeSizeNode *top; F32 bottom_val; IK_StrokeSizeNode *free; B32 auto_pop; } stroke_size_stack;\
struct { IK_StrokeColorNode *top; Vec4F32 bottom_val; IK_StrokeColorNode *free; B32 auto_pop; } stroke_color_stack;\
struct { IK_TransparencyNode *top; F32 bottom_val; IK_TransparencyNode *free; B32 auto_pop; } transparency_stack;\
struct { IK_PaletteNode *top; IK_Palette*     bottom_val; IK_PaletteNode *free; B32 auto_pop; } palette_stack;\
struct { IK_FlagsNode *top; IK_BoxFlags bottom_val; IK_FlagsNode *free; B32 auto_pop; } flags_stack;\
struct { IK_HoverCursorNode *top; OS_Cursor bottom_val; IK_HoverCursorNode *free; B32 auto_pop; } hover_cursor_stack;\
struct { IK_TextPaddingNode *top; F32 bottom_val; IK_TextPaddingNode *free; B32 auto_pop; } text_padding_stack;\
struct { IK_TextAlignmentNode *top; IK_TextAlign bottom_val; IK_TextAlignmentNode *free; B32 auto_pop; } text_alignment_stack;\
}
#define IK_InitStacks(state) \
state->frame_stack.top = &state->frame_nil_stack_top; state->frame_stack.bottom_val = 0; state->frame_stack.free = 0; state->frame_stack.auto_pop = 0;\
state->font_stack.top = &state->font_nil_stack_top; state->font_stack.bottom_val = fnt_tag_zero(); state->font_stack.free = 0; state->font_stack.auto_pop = 0;\
state->font_slot_stack.top = &state->font_slot_nil_stack_top; state->font_slot_stack.bottom_val = IK_FontSlot_Main; state->font_slot_stack.free = 0; state->font_slot_stack.auto_pop = 0;\
state->font_size_stack.top = &state->font_size_nil_stack_top; state->font_size_stack.bottom_val = 24.f; state->font_size_stack.free = 0; state->font_size_stack.auto_pop = 0;\
state->text_raster_flags_stack.top = &state->text_raster_flags_nil_stack_top; state->text_raster_flags_stack.bottom_val = FNT_RasterFlag_Hinted; state->text_raster_flags_stack.free = 0; state->text_raster_flags_stack.auto_pop = 0;\
state->tab_size_stack.top = &state->tab_size_nil_stack_top; state->tab_size_stack.bottom_val = 24.f*4.f; state->tab_size_stack.free = 0; state->tab_size_stack.auto_pop = 0;\
state->background_color_stack.top = &state->background_color_nil_stack_top; state->background_color_stack.bottom_val = rgba_from_u32(0xEEE6CAFF); state->background_color_stack.free = 0; state->background_color_stack.auto_pop = 0;\
state->stroke_size_stack.top = &state->stroke_size_nil_stack_top; state->stroke_size_stack.bottom_val = 8; state->stroke_size_stack.free = 0; state->stroke_size_stack.auto_pop = 0;\
state->stroke_color_stack.top = &state->stroke_color_nil_stack_top; state->stroke_color_stack.bottom_val = rgba_from_u32(0xEEE6CAFF); state->stroke_color_stack.free = 0; state->stroke_color_stack.auto_pop = 0;\
state->transparency_stack.top = &state->transparency_nil_stack_top; state->transparency_stack.bottom_val = 0; state->transparency_stack.free = 0; state->transparency_stack.auto_pop = 0;\
state->palette_stack.top = &state->palette_nil_stack_top; state->palette_stack.bottom_val = 0; state->palette_stack.free = 0; state->palette_stack.auto_pop = 0;\
state->flags_stack.top = &state->flags_nil_stack_top; state->flags_stack.bottom_val = 0; state->flags_stack.free = 0; state->flags_stack.auto_pop = 0;\
state->hover_cursor_stack.top = &state->hover_cursor_nil_stack_top; state->hover_cursor_stack.bottom_val = OS_Cursor_Pointer; state->hover_cursor_stack.free = 0; state->hover_cursor_stack.auto_pop = 0;\
state->text_padding_stack.top = &state->text_padding_nil_stack_top; state->text_padding_stack.bottom_val = 0; state->text_padding_stack.free = 0; state->text_padding_stack.auto_pop = 0;\
state->text_alignment_stack.top = &state->text_alignment_nil_stack_top; state->text_alignment_stack.bottom_val = IK_TextAlign_Left; state->text_alignment_stack.free = 0; state->text_alignment_stack.auto_pop = 0;\

#define IK_AutoPopStacks(state) \
if(state->frame_stack.auto_pop) { ik_pop_frame(); state->frame_stack.auto_pop = 0; }\
if(state->font_stack.auto_pop) { ik_pop_font(); state->font_stack.auto_pop = 0; }\
if(state->font_slot_stack.auto_pop) { ik_pop_font_slot(); state->font_slot_stack.auto_pop = 0; }\
if(state->font_size_stack.auto_pop) { ik_pop_font_size(); state->font_size_stack.auto_pop = 0; }\
if(state->text_raster_flags_stack.auto_pop) { ik_pop_text_raster_flags(); state->text_raster_flags_stack.auto_pop = 0; }\
if(state->tab_size_stack.auto_pop) { ik_pop_tab_size(); state->tab_size_stack.auto_pop = 0; }\
if(state->background_color_stack.auto_pop) { ik_pop_background_color(); state->background_color_stack.auto_pop = 0; }\
if(state->stroke_size_stack.auto_pop) { ik_pop_stroke_size(); state->stroke_size_stack.auto_pop = 0; }\
if(state->stroke_color_stack.auto_pop) { ik_pop_stroke_color(); state->stroke_color_stack.auto_pop = 0; }\
if(state->transparency_stack.auto_pop) { ik_pop_transparency(); state->transparency_stack.auto_pop = 0; }\
if(state->palette_stack.auto_pop) { ik_pop_palette(); state->palette_stack.auto_pop = 0; }\
if(state->flags_stack.auto_pop) { ik_pop_flags(); state->flags_stack.auto_pop = 0; }\
if(state->hover_cursor_stack.auto_pop) { ik_pop_hover_cursor(); state->hover_cursor_stack.auto_pop = 0; }\
if(state->text_padding_stack.auto_pop) { ik_pop_text_padding(); state->text_padding_stack.auto_pop = 0; }\
if(state->text_alignment_stack.auto_pop) { ik_pop_text_alignment(); state->text_alignment_stack.auto_pop = 0; }\

internal IK_Frame *                 ik_top_frame(void);
internal FNT_Tag                    ik_top_font(void);
internal IK_FontSlot                ik_top_font_slot(void);
internal F32                        ik_top_font_size(void);
internal FNT_RasterFlags            ik_top_text_raster_flags(void);
internal F32                        ik_top_tab_size(void);
internal Vec4F32                    ik_top_background_color(void);
internal F32                        ik_top_stroke_size(void);
internal Vec4F32                    ik_top_stroke_color(void);
internal F32                        ik_top_transparency(void);
internal IK_Palette*                ik_top_palette(void);
internal IK_BoxFlags                ik_top_flags(void);
internal OS_Cursor                  ik_top_hover_cursor(void);
internal F32                        ik_top_text_padding(void);
internal IK_TextAlign               ik_top_text_alignment(void);
internal IK_Frame *                 ik_bottom_frame(void);
internal FNT_Tag                    ik_bottom_font(void);
internal IK_FontSlot                ik_bottom_font_slot(void);
internal F32                        ik_bottom_font_size(void);
internal FNT_RasterFlags            ik_bottom_text_raster_flags(void);
internal F32                        ik_bottom_tab_size(void);
internal Vec4F32                    ik_bottom_background_color(void);
internal F32                        ik_bottom_stroke_size(void);
internal Vec4F32                    ik_bottom_stroke_color(void);
internal F32                        ik_bottom_transparency(void);
internal IK_Palette*                ik_bottom_palette(void);
internal IK_BoxFlags                ik_bottom_flags(void);
internal OS_Cursor                  ik_bottom_hover_cursor(void);
internal F32                        ik_bottom_text_padding(void);
internal IK_TextAlign               ik_bottom_text_alignment(void);
internal IK_Frame *                 ik_push_frame(IK_Frame * v);
internal FNT_Tag                    ik_push_font(FNT_Tag v);
internal IK_FontSlot                ik_push_font_slot(IK_FontSlot v);
internal F32                        ik_push_font_size(F32 v);
internal FNT_RasterFlags            ik_push_text_raster_flags(FNT_RasterFlags v);
internal F32                        ik_push_tab_size(F32 v);
internal Vec4F32                    ik_push_background_color(Vec4F32 v);
internal F32                        ik_push_stroke_size(F32 v);
internal Vec4F32                    ik_push_stroke_color(Vec4F32 v);
internal F32                        ik_push_transparency(F32 v);
internal IK_Palette*                ik_push_palette(IK_Palette*     v);
internal IK_BoxFlags                ik_push_flags(IK_BoxFlags v);
internal OS_Cursor                  ik_push_hover_cursor(OS_Cursor v);
internal F32                        ik_push_text_padding(F32 v);
internal IK_TextAlign               ik_push_text_alignment(IK_TextAlign v);
internal IK_Frame *                 ik_pop_frame(void);
internal FNT_Tag                    ik_pop_font(void);
internal IK_FontSlot                ik_pop_font_slot(void);
internal F32                        ik_pop_font_size(void);
internal FNT_RasterFlags            ik_pop_text_raster_flags(void);
internal F32                        ik_pop_tab_size(void);
internal Vec4F32                    ik_pop_background_color(void);
internal F32                        ik_pop_stroke_size(void);
internal Vec4F32                    ik_pop_stroke_color(void);
internal F32                        ik_pop_transparency(void);
internal IK_Palette*                ik_pop_palette(void);
internal IK_BoxFlags                ik_pop_flags(void);
internal OS_Cursor                  ik_pop_hover_cursor(void);
internal F32                        ik_pop_text_padding(void);
internal IK_TextAlign               ik_pop_text_alignment(void);
internal IK_Frame *                 ik_set_next_frame(IK_Frame * v);
internal FNT_Tag                    ik_set_next_font(FNT_Tag v);
internal IK_FontSlot                ik_set_next_font_slot(IK_FontSlot v);
internal F32                        ik_set_next_font_size(F32 v);
internal FNT_RasterFlags            ik_set_next_text_raster_flags(FNT_RasterFlags v);
internal F32                        ik_set_next_tab_size(F32 v);
internal Vec4F32                    ik_set_next_background_color(Vec4F32 v);
internal F32                        ik_set_next_stroke_size(F32 v);
internal Vec4F32                    ik_set_next_stroke_color(Vec4F32 v);
internal F32                        ik_set_next_transparency(F32 v);
internal IK_Palette*                ik_set_next_palette(IK_Palette*     v);
internal IK_BoxFlags                ik_set_next_flags(IK_BoxFlags v);
internal OS_Cursor                  ik_set_next_hover_cursor(OS_Cursor v);
internal F32                        ik_set_next_text_padding(F32 v);
internal IK_TextAlign               ik_set_next_text_alignment(IK_TextAlign v);
C_LINKAGE_BEGIN
extern String8 ik_icon_kind_text_table[69];
extern String8 ik_theme_preset_display_string_table[9];
extern String8 ik_theme_preset_code_string_table[9];
extern String8 ik_theme_color_version_remap_old_name_table[22];
extern String8 ik_theme_color_version_remap_new_name_table[22];
extern Vec4F32 ik_theme_preset_colors__default_dark[76];
extern Vec4F32 ik_theme_preset_colors__default_light[76];
extern Vec4F32 ik_theme_preset_colors__vs_dark[76];
extern Vec4F32 ik_theme_preset_colors__vs_light[76];
extern Vec4F32 ik_theme_preset_colors__solarized_dark[76];
extern Vec4F32 ik_theme_preset_colors__solarized_light[76];
extern Vec4F32 ik_theme_preset_colors__handmade_hero[76];
extern Vec4F32 ik_theme_preset_colors__four_coder[76];
extern Vec4F32 ik_theme_preset_colors__far_manager[76];
extern Vec4F32* ik_theme_preset_colors_table[9];
extern String8 ik_theme_color_display_string_table[76];
extern String8 ik_theme_color_cfg_string_table[76];
extern String8 ik_setting_code_display_string_table[9];
extern String8 ik_setting_code_lower_string_table[9];
extern IK_SettingVal ik_setting_code_default_val_table[9];
extern Rng1S32 ik_setting_code_s32_range_table[9];

C_LINKAGE_END

#endif // INK_META_H

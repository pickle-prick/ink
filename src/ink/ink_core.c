#define IK_StackTopImpl(state, name_upper, name_lower) \
return state->name_lower##_stack.top->v;

#define IK_StackBottomImpl(state, name_upper, name_lower) \
return state->name_lower##_stack.bottom_val;

#define IK_StackPushImpl(state, name_upper, name_lower, type, new_value) \
IK_##name_upper##Node *node = state->name_lower##_stack.free;\
if(node != 0) {SLLStackPop(state->name_lower##_stack.free);}\
else {node = push_array(ik_state->arena, IK_##name_upper##Node, 1);}\
type old_value = state->name_lower##_stack.top->v;\
node->v = new_value;\
SLLStackPush(state->name_lower##_stack.top, node);\
if(node->next == &state->name_lower##_nil_stack_top)\
{\
state->name_lower##_stack.bottom_val = (new_value);\
}\
state->name_lower##_stack.auto_pop = 0;\
return old_value;

#define IK_StackPopImpl(state, name_upper, name_lower) \
IK_##name_upper##Node *popped = state->name_lower##_stack.top;\
if(popped != &state->name_lower##_nil_stack_top)\
{\
SLLStackPop(state->name_lower##_stack.top);\
SLLStackPush(state->name_lower##_stack.free, popped);\
state->name_lower##_stack.auto_pop = 0;\
}\
return popped->v;\

#define IK_StackSetNextImpl(state, name_upper, name_lower, type, new_value) \
IK_##name_upper##Node *node = state->name_lower##_stack.free;\
if(node != 0) {SLLStackPop(state->name_lower##_stack.free);}\
else {node = push_array(ik_state->arena, IK_##name_upper##Node, 1);}\
type old_value = state->name_lower##_stack.top->v;\
node->v = new_value;\
SLLStackPush(state->name_lower##_stack.top, node);\
state->name_lower##_stack.auto_pop = 1;\
return old_value;

#include "generated/ink.meta.c"

#define M_1 48

// TODO(Next): maybe we could use metagen
// Color palette
internal Vec4F32 stroke_colors[] = {rgba_from_u32_lit_comp(0xEEE6CAFF), rgba_from_u32_lit_comp(0x666666FF), rgba_from_u32_lit_comp(0xEB5B00FF)};
internal Vec4F32 background_colors[] = {rgba_from_u32_lit_comp(0xEEE6CAFF), rgba_from_u32_lit_comp(0x666666FF), rgba_from_u32_lit_comp(0xEB5B00FF)};

internal void
ik_ui_draw()
{
  Temp scratch = scratch_begin(0,0);
  F32 box_squish_epsilon = 0.001f;
  dr_push_viewport(ik_state->window_dim);

  // unpack some settings
  F32 border_softness = 1.f;
  F32 rounded_corner_amount = 1.0;

  // DEBUG mouse
  if(0)
  {
    R_Rect2DInst *cursor = dr_rect(r2f32p(ui_state->mouse.x-15,ui_state->mouse.y-15, ui_state->mouse.x+15,ui_state->mouse.y+15), v4f32(0,0.3,1,0.3), 15, 0.0, 0.7);
  }

  // Recusivly drawing boxes
  UI_Box *box = ui_root_from_state(ui_state);
  while(!ui_box_is_nil(box))
  {
    UI_BoxRec rec = ui_box_rec_df_post(box, &ui_nil_box);

    // rjf: get corner radii
    F32 box_corner_radii[Corner_COUNT] =
    {
      box->corner_radii[Corner_00]*rounded_corner_amount,
      box->corner_radii[Corner_01]*rounded_corner_amount,
      box->corner_radii[Corner_10]*rounded_corner_amount,
      box->corner_radii[Corner_11]*rounded_corner_amount,
    };

    // push transparency
    if(box->transparency != 0)
    {
      dr_push_transparency(box->transparency);
    }

    // push squish
    if(box->squish > box_squish_epsilon)
    {
      Vec2F32 box_dim = dim_2f32(box->rect);
      Vec2F32 anchor_off = {0};
      if(box->flags & UI_BoxFlag_SquishAnchored)
      {
        anchor_off.x = box_dim.x/2.0;
      }
      else
      {
        anchor_off.y = -box_dim.y/8.0;
      }
      // NOTE(k): this is not from center
      Vec2F32 origin = {box->rect.x0 + box_dim.x/2 - anchor_off.x, box->rect.y0 - anchor_off.y};

      Mat3x3F32 box2origin_xform = make_translate_3x3f32(v2f32(-origin.x, -origin.y));
      Mat3x3F32 scale_xform = make_scale_3x3f32(v2f32(1-box->squish, 1-box->squish));
      Mat3x3F32 origin2box_xform = make_translate_3x3f32(origin);
      Mat3x3F32 xform = mul_3x3f32(origin2box_xform, mul_3x3f32(scale_xform, box2origin_xform));
      // TODO(k): why we may need to tranpose this, should it already column major? check it later
      xform = transpose_3x3f32(xform);
      dr_push_xform2d(xform);
      dr_push_tex2d_sample_kind(R_Tex2DSampleKind_Linear);
    }

    // draw drop_shadw
    if(box->flags & UI_BoxFlag_DrawDropShadow)
    {
      Rng2F32 drop_shadow_rect = shift_2f32(pad_2f32(box->rect, 8), v2f32(4, 4));
      Vec4F32 drop_shadow_color = ik_rgba_from_theme_color(IK_ThemeColor_DropShadow);
      dr_rect(drop_shadow_rect, drop_shadow_color, 0.8f, 0, 8.f);
    }

    // draw background
    if(box->flags & UI_BoxFlag_DrawBackground)
    {
      // main rectangle
      R_Rect2DInst *inst = dr_rect(pad_2f32(box->rect, 1), box->palette->colors[UI_ColorCode_Background], 0, 0, 1.f);
      MemoryCopyArray(inst->corner_radii, box->corner_radii);

      if(box->flags & UI_BoxFlag_DrawHotEffects)
      {
        B32 is_hot = !ui_key_match(box->key, ui_key_zero()) && ui_key_match(box->key, ui_hot_key());
        Vec4F32 hover_color = ik_rgba_from_theme_color(IK_ThemeColor_Hover);

        F32 effective_active_t = box->active_t;
        if(!(box->flags & UI_BoxFlag_DrawActiveEffects))
        {
          effective_active_t = 0;
        }
        F32 t = box->hot_t * (1-effective_active_t);

        // brighten
        {
          Vec4F32 color = hover_color;
          color.w *= 0.15f;
          if(!is_hot)
          {
            color.w *= t;
          }
          R_Rect2DInst *inst = dr_rect(pad_2f32(box->rect, 1.f), v4f32(0, 0, 0, 0), 0, 0, border_softness*1.f);
          inst->colors[Corner_00] = color;
          inst->colors[Corner_10] = color;
          inst->colors[Corner_01] = color;
          inst->colors[Corner_11] = color;
          MemoryCopyArray(inst->corner_radii, box_corner_radii);
        }

        // rjf: soft circle around mouse
        if(box->hot_t > 0.01f) DR_ClipScope(box->rect)
        {
          Vec4F32 color = hover_color;
          color.w *= 0.04f;
          if(!is_hot)
          {
            color.w *= t;
          }
          Vec2F32 center = ui_mouse();
          Vec2F32 box_dim = dim_2f32(box->rect);
          F32 max_dim = Max(box_dim.x, box_dim.y);
          F32 radius = box->font_size*12.f;
          radius = Min(max_dim, radius);
          dr_rect(pad_2f32(r2f32(center, center), radius), color, radius, 0, radius/3.f);
        }

        // rjf: slight emboss fadeoff
        if(0)
        {
          Rng2F32 rect = r2f32p(box->rect.x0, box->rect.y0, box->rect.x1, box->rect.y1);
          R_Rect2DInst *inst = dr_rect(rect, v4f32(0, 0, 0, 0), 0, 0, 1.f);
          inst->colors[Corner_00] = v4f32(0.f, 0.f, 0.f, 0.0f*t);
          inst->colors[Corner_01] = v4f32(0.f, 0.f, 0.f, 0.3f*t);
          inst->colors[Corner_10] = v4f32(0.f, 0.f, 0.f, 0.0f*t);
          inst->colors[Corner_11] = v4f32(0.f, 0.f, 0.f, 0.3f*t);
          MemoryCopyArray(inst->corner_radii, box->corner_radii);
        }

        // active effect extension
        if(box->flags & UI_BoxFlag_DrawActiveEffects)
        {
          Vec4F32 shadow_color = ik_rgba_from_theme_color(IK_ThemeColor_DropShadow);
          shadow_color.w *= 1.f*box->active_t;

          Vec2F32 shadow_size =
          {
            (box->rect.x1 - box->rect.x0)*0.60f*box->active_t,
            (box->rect.y1 - box->rect.y0)*0.60f*box->active_t,
          };
          shadow_size.x = Clamp(0, shadow_size.x, box->font_size*2.f);
          shadow_size.y = Clamp(0, shadow_size.y, box->font_size*2.f);

          // rjf: top -> bottom dark effect
          {
            R_Rect2DInst *inst = dr_rect(r2f32p(box->rect.x0, box->rect.y0, box->rect.x1, box->rect.y0 + shadow_size.y), v4f32(0, 0, 0, 0), 0, 0, 1.f);
            inst->colors[Corner_00] = inst->colors[Corner_10] = shadow_color;
            inst->colors[Corner_01] = inst->colors[Corner_11] = v4f32(0.f, 0.f, 0.f, 0.0f);
            MemoryCopyArray(inst->corner_radii, box_corner_radii);
          }
          
          // rjf: bottom -> top light effect
          {
            R_Rect2DInst *inst = dr_rect(r2f32p(box->rect.x0, box->rect.y1 - shadow_size.y, box->rect.x1, box->rect.y1), v4f32(0, 0, 0, 0), 0, 0, 1.f);
            inst->colors[Corner_00] = inst->colors[Corner_10] = v4f32(0, 0, 0, 0);
            inst->colors[Corner_01] = inst->colors[Corner_11] = v4f32(1.0f, 1.0f, 1.0f, 0.08f*box->active_t);
            MemoryCopyArray(inst->corner_radii, box_corner_radii);
          }
          
          // rjf: left -> right dark effect
          {
            R_Rect2DInst *inst = dr_rect(r2f32p(box->rect.x0, box->rect.y0, box->rect.x0 + shadow_size.x, box->rect.y1), v4f32(0, 0, 0, 0), 0, 0, 1.f);
            inst->colors[Corner_10] = inst->colors[Corner_11] = v4f32(0.f, 0.f, 0.f, 0.f);
            inst->colors[Corner_00] = shadow_color;
            inst->colors[Corner_01] = shadow_color;
            MemoryCopyArray(inst->corner_radii, box_corner_radii);
          }
          
          // rjf: right -> left dark effect
          {
            R_Rect2DInst *inst = dr_rect(r2f32p(box->rect.x1 - shadow_size.x, box->rect.y0, box->rect.x1, box->rect.y1), v4f32(0, 0, 0, 0), 0, 0, 1.f);
            inst->colors[Corner_00] = inst->colors[Corner_01] = v4f32(0.f, 0.f, 0.f, 0.f);
            inst->colors[Corner_10] = shadow_color;
            inst->colors[Corner_11] = shadow_color;
            MemoryCopyArray(inst->corner_radii, box_corner_radii);
          }
        }
      }
    }

    // draw image
    if(box->flags & UI_BoxFlag_DrawImage)
    {
      R_Rect2DInst *inst = dr_img(box->rect, box->src, box->albedo_tex, box->albedo_clr, 0., 0., 0.);
      inst->white_texture_override = box->albedo_white_texture_override ? 1.0 : 0.0;
    }

    // draw string
    if(box->flags & UI_BoxFlag_DrawText)
    {
      Vec2F32 text_position = ui_box_text_position(box);

      // max width
      F32 max_x = 100000.0f;
      FNT_Run ellipses_run = {0};

      if(!(box->flags & UI_BoxFlag_DisableTextTrunc))
      {
        max_x = (box->rect.x1-text_position.x);
        ellipses_run = fnt_run_from_string(box->font, box->font_size, 0, box->tab_size, 0, str8_lit("..."));
      }

      dr_truncated_fancy_run_list(text_position, &box->display_string_runs, max_x, ellipses_run);
    }

    // NOTE(k): draw focus viz
    if(1)
    {
      B32 focused = (box->flags & (UI_BoxFlag_FocusHot|UI_BoxFlag_FocusActive) &&
                     box->flags & UI_BoxFlag_Clickable);
      B32 disabled = 0;
      for(UI_Box *p = box; !ui_box_is_nil(p); p = p->parent)
      {
        if(p->flags & (UI_BoxFlag_FocusHotDisabled|UI_BoxFlag_FocusActiveDisabled))
        {
          disabled = 1;
          break;
        }
      }
      if(focused)
      {
        Vec4F32 color = v4f32(0.3f, 0.8f, 0.3f, 1.f);
        if(disabled)
        {
          color = v4f32(0.8f, 0.3f, 0.3f, 1.f);
        }
        dr_rect(r2f32p(box->rect.x0-6, box->rect.y0-6, box->rect.x0+6, box->rect.y0+6), color, 2, 0, 1);
        dr_rect(box->rect, color, 2, 2, 1);
      }
      if(box->flags & (UI_BoxFlag_FocusHot|UI_BoxFlag_FocusActive))
      {
        if(box->flags & (UI_BoxFlag_FocusHotDisabled|UI_BoxFlag_FocusActiveDisabled))
        {
          dr_rect(r2f32p(box->rect.x0-6, box->rect.y0-6, box->rect.x0+6, box->rect.y0+6), v4f32(1, 0, 0, 0.2f), 2, 0, 1);
        }
        else
        {
          dr_rect(r2f32p(box->rect.x0-6, box->rect.y0-6, box->rect.x0+6, box->rect.y0+6), v4f32(0, 1, 0, 0.2f), 2, 0, 1);
        }
      }
    }

    // push clip
    if(box->flags & UI_BoxFlag_Clip)
    {
      Rng2F32 top_clip = dr_top_clip();
      Rng2F32 new_clip = pad_2f32(box->rect, -1);
      if(top_clip.x1 != 0 || top_clip.y1 != 0)
      {
        new_clip = intersect_2f32(new_clip, top_clip);
      }
      dr_push_clip(new_clip);
    }

    // k: custom draw list
    if(box->flags & UI_BoxFlag_DrawBucket)
    {
      Mat3x3F32 xform = make_translate_3x3f32(box->position_delta);
      DR_XForm2DScope(xform)
      {
        dr_sub_bucket(box->draw_bucket);
      }
    }

    // call custom draw callback
    if(box->custom_draw != 0)
    {
      box->custom_draw(box, box->custom_draw_user_data);
    }

    // k: pop stacks
    {
      S32 pop_idx = 0;
      for(UI_Box *b = box; !ui_box_is_nil(b) && pop_idx <= rec.pop_count; b = b->parent)
      {
        pop_idx += 1;
        if(b == box && rec.push_count != 0) continue;

        // k: pop clip
        if(b->flags & UI_BoxFlag_Clip)
        {
          dr_pop_clip();
        }

        // rjf: draw overlay
        if(b->flags & UI_BoxFlag_DrawOverlay)
        {
          R_Rect2DInst *inst = dr_rect(b->rect, b->palette->colors[UI_ColorCode_Overlay], 0, 0, 1.f);
          MemoryCopyArray(inst->corner_radii, b->corner_radii);
        }

        //- k: draw the border
        if(b->flags & UI_BoxFlag_DrawBorder)
        {
          R_Rect2DInst *inst = dr_rect(pad_2f32(b->rect, 1.f), b->palette->colors[UI_ColorCode_Border], 0, 1.f, border_softness*1.f);
          MemoryCopyArray(inst->corner_radii, b->corner_radii);

          // rjf: hover effect
          if(b->flags & UI_BoxFlag_DrawHotEffects)
          {
            Vec4F32 color = ik_rgba_from_theme_color(IK_ThemeColor_Hover);
            color.w *= b->hot_t;
            R_Rect2DInst *inst = dr_rect(pad_2f32(b->rect, 1), color, 0, 1.f, 1.f);
            inst->colors[Corner_01].w *= 0.2f;
            inst->colors[Corner_11].w *= 0.2f;
            MemoryCopyArray(inst->corner_radii, b->corner_radii);
          }
        }

        // k: debug border rendering
        if(0)
        {
          R_Rect2DInst *inst = dr_rect(pad_2f32(b->rect, 1), v4f32(1, 0, 1, 0.25f), 0, 1.f, 1.f);
          MemoryCopyArray(inst->corner_radii, b->corner_radii);
        }

        // rjf: draw sides
        {
          Rng2F32 r = b->rect;
          F32 half_thickness = 1.f;
          F32 softness = 0.5f;
          if(b->flags & UI_BoxFlag_DrawSideTop)
          {
            dr_rect(r2f32p(r.x0, r.y0-half_thickness, r.x1, r.y0+half_thickness), b->palette->colors[UI_ColorCode_Border], 0, 0, softness);
          }
          if(b->flags & UI_BoxFlag_DrawSideBottom)
          {
            dr_rect(r2f32p(r.x0, r.y1-half_thickness, r.x1, r.y1+half_thickness), b->palette->colors[UI_ColorCode_Border], 0, 0, softness);
          }
          if(b->flags & UI_BoxFlag_DrawSideLeft)
          {
            dr_rect(r2f32p(r.x0-half_thickness, r.y0, r.x0+half_thickness, r.y1), b->palette->colors[UI_ColorCode_Border], 0, 0, softness);
          }
          if(b->flags & UI_BoxFlag_DrawSideRight)
          {
            dr_rect(r2f32p(r.x1-half_thickness, r.y0, r.x1+half_thickness, r.y1), b->palette->colors[UI_ColorCode_Border], 0, 0, softness);
          }
        }

        // rjf: draw focus overlay
        if(b->flags & UI_BoxFlag_Clickable && !(b->flags & UI_BoxFlag_DisableFocusOverlay) && b->focus_hot_t > 0.01f)
        {
          Vec4F32 color = ik_rgba_from_theme_color(IK_ThemeColor_Focus);
          color.w *= 0.2f*b->focus_hot_t;
          R_Rect2DInst *inst = dr_rect(b->rect, color, 0, 0, 0.f);
          MemoryCopyArray(inst->corner_radii, b->corner_radii);
        }

        // rjf: draw focus border
        if(b->flags & UI_BoxFlag_Clickable && !(b->flags & UI_BoxFlag_DisableFocusBorder) && b->focus_active_t > 0.01f)
        {
          Vec4F32 color = ik_rgba_from_theme_color(IK_ThemeColor_Focus);
          color.w *= b->focus_active_t;
          R_Rect2DInst *inst = dr_rect(pad_2f32(b->rect, 0.f), color, 0, 1.f, 1.f);
          MemoryCopyArray(inst->corner_radii, b->corner_radii);
        }

        // rjf: disabled overlay
        if(b->disabled_t >= 0.005f)
        {
          Vec4F32 color = ik_rgba_from_theme_color(IK_ThemeColor_DisabledOverlay);
          color.w *= b->disabled_t;
          R_Rect2DInst *inst = dr_rect(b->rect, color, 0, 0, 1);
          MemoryCopyArray(inst->corner_radii, b->corner_radii);
        }

        // rjf: pop squish
        if(b->squish > box_squish_epsilon)
        {
          dr_pop_xform2d();
          dr_pop_tex2d_sample_kind();
        }

        // k: pop transparency
        if(b->transparency != 0)
        {
          dr_pop_transparency();
        }
      }
    }
    box = rec.next;
  }
  dr_pop_viewport();
  scratch_end(scratch);
}

/////////////////////////////////
//~ Basic Type Functions

internal U64
ik_hash_from_string(U64 seed, String8 string)
{
  U64 result = XXH3_64bits_withSeed(string.str, string.size, seed);
  return result;
}

internal String8
ik_hash_part_from_key_string(String8 string)
{
  String8 result = string;
  // k: look for ### patterns, which can replace the entirety of the part of
  // the string that is hashed.
  U64 hash_replace_signifier_pos = str8_find_needle(string, 0, str8_lit("###"), 0);
  if(hash_replace_signifier_pos < string.size)
  {
    result = str8_skip(string, hash_replace_signifier_pos);
  }
  return result;
}

internal String8
ik_display_part_from_key_string(String8 string)
{
  U64 hash_pos = str8_find_needle(string, 0, str8_lit("##"), 0);
  string.size = hash_pos;
  return string;
}

/////////////////////////////////
//~ Key

internal IK_Key
ik_key_from_string(IK_Key seed_key, String8 string)
{
  IK_Key result = {0};
  if(string.size != 0)
  {
    String8 hash_part = ik_hash_part_from_key_string(string);
    result.u64[0] = ik_hash_from_string(seed_key.u64[0], hash_part);
  }
  return result;
}

internal IK_Key
ik_key_from_stringf(IK_Key seed_key, char *fmt, ...)
{
  Temp scratch = scratch_begin(0,0);
  va_list args;
  va_start(args, fmt);
  String8 string = push_str8fv(scratch.arena, fmt, args);
  va_end(args);

  IK_Key result = {0};
  result = ik_key_from_string(seed_key, string);
  scratch_end(scratch);
  return result;
}

internal inline B32
ik_key_match(IK_Key a, IK_Key b)
{
  return MemoryMatchStruct(&a,&b);
}

internal inline IK_Key
ik_key_make(U64 a, U64 b)
{
  IK_Key result = {a,b};
  return result;
}

internal inline IK_Key
ik_key_zero()
{
  return (IK_Key){0}; 
}

internal IK_Key
ik_key_new()
{
  // TODO: we could use generation index here
  IK_Key ret = ik_key_make(os_now_microseconds(), 0);
  return ret;
}

internal Vec3F32
ik_2f32_from_key(IK_Key key)
{
#if 0
  U32 high = (key.u64[0]>>32);
  U32 low = key.u64[0];
  Vec2F32 ret = {*((F32*)&high), *((F32*)&low)};
  // FIXME: vulkan could drop subnormal number (< 1.0e-39) to 0
  // https://stackoverflow.com/questions/73498522/writing-small-floats-to-fbo-in-vulkan
  return ret;
#endif

  // 64-bit into 3 f32
  // R = top 24 bits, G = middle 24 bits, B = lower 16 bits
  F32 r = (F32)((key.u64[0]>>40) & 0xFFFFFFu);
  F32 g = (F32)((key.u64[0]>>16) & 0xFFFFFFu);
  F32 b = (F32)((key.u64[0]>>0)  & 0xFFFFFFu);
  Vec3F32 ret = {r,g,b};
  return ret;
}

internal IK_Key
ik_key_from_3f32(Vec3F32 key_3f32)
{
  U64 r = (U64)key_3f32.x;
  U64 g = (U64)key_3f32.y;
  U64 b = (U64)key_3f32.z;
  U64 key = (r<<40) | (g<<16) | b;
  IK_Key ret = {key, 0};
  return ret;
}

/////////////////////////////////
//~ State accessor/mutator

//- init

internal void
ik_init(OS_Handle os_wnd, R_Handle r_wnd)
{
  Arena *arena = arena_alloc();
  ik_state = push_array(arena, IK_State, 1);
  {
    ik_state->arena = arena;
    ik_state->os_wnd = os_wnd;
    ik_state->r_wnd = r_wnd;
    ik_state->dpi = os_dpi_from_window(os_wnd);
    ik_state->last_dpi = ik_state->last_dpi;
    ik_state->window_rect = os_client_rect_from_window(os_wnd, 1);
    ik_state->window_dim = dim_2f32(ik_state->window_rect);
    ik_state->drag_state_arena = arena_alloc();
    ik_state->box_scratch_arena = arena_alloc();
    ik_state->tool = IK_ToolKind_Selection;
    ik_state->message_arena = arena_alloc();
    // decode queue & threads
    ik_state->decode_queue.semaphore = semaphore_alloc(0, ArrayCount(ik_state->decode_queue.queue), str8_lit(""));
    for(U64 i = 0; i < ArrayCount(ik_state->decode_threads); i++)
    {
      ik_state->decode_threads[i] = os_thread_launch(ik_image_decode_thread__entry_point, &ik_state->decode_queue, 0);
    }

    // frame arena
    for(U64 i = 0; i < ArrayCount(ik_state->frame_arenas); i++)
    {
      ik_state->frame_arenas[i] = arena_alloc(.reserve_size = MB(128), .commit_size = MB(8));
    }
    // NOTE(k): not needed for now
    // frame drawlists
    // for(U64 i = 0; i < ArrayCount(ik_state->drawlists); i++)
    // {
    //   // TODO(k): some device offers 256MB memory which is both cpu visiable and device local
    //   ik_state->drawlists[i] = ik_drawlist_alloc(arena, MB(16), MB(16));
    // }
  }

  // Settings
  for EachEnumVal(IK_SettingCode, code)
  {
    ik_state->setting_vals[code] = ik_setting_code_default_val_table[code];
  }
  ik_state->setting_vals[IK_SettingCode_MainFontSize].s32 = ik_state->setting_vals[IK_SettingCode_MainFontSize].s32 * (ik_state->last_dpi/96.f);
  ik_state->setting_vals[IK_SettingCode_CodeFontSize].s32 = ik_state->setting_vals[IK_SettingCode_CodeFontSize].s32 * (ik_state->last_dpi/96.f);
  ik_state->setting_vals[IK_SettingCode_MainFontSize].s32 = ClampBot(ik_state->setting_vals[IK_SettingCode_MainFontSize].s32, ik_setting_code_default_val_table[IK_SettingCode_MainFontSize].s32);
  ik_state->setting_vals[IK_SettingCode_CodeFontSize].s32 = ClampBot(ik_state->setting_vals[IK_SettingCode_CodeFontSize].s32, ik_setting_code_default_val_table[IK_SettingCode_CodeFontSize].s32);

  // Fonts
#if BUILD_DEBUG
  ik_state->cfg_font_tags[IK_FontSlot_Main] = fnt_tag_from_path(str8_lit("./data/fonts/segoeui.ttf"));
  ik_state->cfg_font_tags[IK_FontSlot_Code] = fnt_tag_from_path(str8_lit("./data/fonts/segoeui.ttf"));
  ik_state->cfg_font_tags[IK_FontSlot_Icons] = fnt_tag_from_path(str8_lit("./data/fonts/icons.ttf"));
  ik_state->cfg_font_tags[IK_FontSlot_IconsExtra] = fnt_tag_from_path(str8_lit("./data/fonts/icons_extra.ttf"));
  ik_state->cfg_font_tags[IK_FontSlot_HandWrite1] = fnt_tag_from_path(str8_lit("./data/fonts/Virgil.ttf"));
  ik_state->cfg_font_tags[IK_FontSlot_HandWrite2] = fnt_tag_from_path(str8_lit("./data/fonts/XiaolaiMono-Regular.ttf"));
#else
  // String8 font_mono = str8(ttf_Mplus1Code_Medium, ttf_Mplus1Code_Medium_len);
  String8 font_mono = str8(ttf_segoeui, ttf_segoeui_len);
  String8 font_icons = str8(ttf_icons, ttf_icons_len);
  String8 font_icons_extra = str8(ttf_icons_extra, ttf_icons_extra_len);
  String8 font_virgil = str8(ttf_Virgil, ttf_Virgil_len);
  String8 font_xiaolai = str8(ttf_XiaolaiMono_Regular, ttf_XiaolaiMono_Regular_len);
  ik_state->cfg_font_tags[IK_FontSlot_Main] = fnt_tag_from_static_data_string(&font_mono);
  ik_state->cfg_font_tags[IK_FontSlot_Code] = fnt_tag_from_static_data_string(&font_mono);
  ik_state->cfg_font_tags[IK_FontSlot_Icons] = fnt_tag_from_static_data_string(&font_icons);
  ik_state->cfg_font_tags[IK_FontSlot_IconsExtra] = fnt_tag_from_static_data_string(&font_icons_extra);
  ik_state->cfg_font_tags[IK_FontSlot_HandWrite1] = fnt_tag_from_static_data_string(&font_virgil);
  ik_state->cfg_font_tags[IK_FontSlot_HandWrite2] = fnt_tag_from_static_data_string(&font_xiaolai);
#endif

  // Theme 
  MemoryCopy(ik_state->cfg_theme_target.colors, ik_theme_preset_colors__handmade_hero, sizeof(ik_theme_preset_colors__handmade_hero));
  MemoryCopy(ik_state->cfg_theme.colors, ik_theme_preset_colors__handmade_hero, sizeof(ik_theme_preset_colors__handmade_hero));

  //////////////////////////////
  //- k: compute palettes from theme
  {
    IK_Theme *current = &ik_state->cfg_theme;

    // ui palette
    for EachEnumVal(IK_PaletteCode, code)
    {
      ik_state->cfg_ui_debug_palettes[code].null       = v4f32(1, 0, 1, 1);
      ik_state->cfg_ui_debug_palettes[code].cursor     = current->colors[IK_ThemeColor_Cursor];
      ik_state->cfg_ui_debug_palettes[code].selection  = current->colors[IK_ThemeColor_SelectionOverlay];
    }
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_Base].background              = current->colors[IK_ThemeColor_BaseBackground];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_Base].text                    = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_Base].text_weak               = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_Base].border                  = current->colors[IK_ThemeColor_BaseBorder];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_MenuBar].background           = current->colors[IK_ThemeColor_MenuBarBackground];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_MenuBar].text                 = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_MenuBar].text_weak            = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_MenuBar].border               = current->colors[IK_ThemeColor_MenuBarBorder];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_Floating].background          = current->colors[IK_ThemeColor_FloatingBackground];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_Floating].text                = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_Floating].text_weak           = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_Floating].border              = current->colors[IK_ThemeColor_FloatingBorder];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_ImplicitButton].background    = current->colors[IK_ThemeColor_ImplicitButtonBackground];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_ImplicitButton].text          = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_ImplicitButton].text_weak     = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_ImplicitButton].border        = current->colors[IK_ThemeColor_ImplicitButtonBorder];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_PlainButton].background       = current->colors[IK_ThemeColor_PlainButtonBackground];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_PlainButton].text             = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_PlainButton].text_weak        = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_PlainButton].border           = current->colors[IK_ThemeColor_PlainButtonBorder];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_PositivePopButton].background = current->colors[IK_ThemeColor_PositivePopButtonBackground];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_PositivePopButton].text       = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_PositivePopButton].text_weak  = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_PositivePopButton].border     = current->colors[IK_ThemeColor_PositivePopButtonBorder];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_NegativePopButton].background = current->colors[IK_ThemeColor_NegativePopButtonBackground];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_NegativePopButton].text       = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_NegativePopButton].text_weak  = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_NegativePopButton].border     = current->colors[IK_ThemeColor_NegativePopButtonBorder];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_NeutralPopButton].background  = current->colors[IK_ThemeColor_NeutralPopButtonBackground];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_NeutralPopButton].text        = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_NeutralPopButton].text_weak   = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_NeutralPopButton].border      = current->colors[IK_ThemeColor_NeutralPopButtonBorder];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_ScrollBarButton].background   = current->colors[IK_ThemeColor_ScrollBarButtonBackground];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_ScrollBarButton].text         = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_ScrollBarButton].text_weak    = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_ScrollBarButton].border       = current->colors[IK_ThemeColor_ScrollBarButtonBorder];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_Tab].background               = current->colors[IK_ThemeColor_TabBackground];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_Tab].text                     = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_Tab].text_weak                = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_Tab].border                   = current->colors[IK_ThemeColor_TabBorder];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_TabInactive].background       = current->colors[IK_ThemeColor_TabBackgroundInactive];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_TabInactive].text             = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_TabInactive].text_weak        = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_TabInactive].border           = current->colors[IK_ThemeColor_TabBorderInactive];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_DropSiteOverlay].background   = current->colors[IK_ThemeColor_DropSiteOverlay];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_DropSiteOverlay].text         = current->colors[IK_ThemeColor_DropSiteOverlay];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_DropSiteOverlay].text_weak    = current->colors[IK_ThemeColor_DropSiteOverlay];
    ik_state->cfg_ui_debug_palettes[IK_PaletteCode_DropSiteOverlay].border       = current->colors[IK_ThemeColor_DropSiteOverlay];

    // main palette
    for EachEnumVal(IK_PaletteCode, code)
    {
      ik_state->cfg_main_palettes[code].null       = v4f32(1, 0, 1, 1);
      ik_state->cfg_main_palettes[code].cursor     = current->colors[IK_ThemeColor_Cursor];
      ik_state->cfg_main_palettes[code].selection  = current->colors[IK_ThemeColor_SelectionOverlay];
    }
    ik_state->cfg_main_palettes[IK_PaletteCode_Base].background              = current->colors[IK_ThemeColor_BaseBackground];
    ik_state->cfg_main_palettes[IK_PaletteCode_Base].text                    = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_main_palettes[IK_PaletteCode_Base].text_weak               = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_main_palettes[IK_PaletteCode_Base].border                  = current->colors[IK_ThemeColor_BaseBorder];
    ik_state->cfg_main_palettes[IK_PaletteCode_MenuBar].background           = current->colors[IK_ThemeColor_MenuBarBackground];
    ik_state->cfg_main_palettes[IK_PaletteCode_MenuBar].text                 = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_main_palettes[IK_PaletteCode_MenuBar].text_weak            = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_main_palettes[IK_PaletteCode_MenuBar].border               = current->colors[IK_ThemeColor_MenuBarBorder];
    ik_state->cfg_main_palettes[IK_PaletteCode_Floating].background          = current->colors[IK_ThemeColor_FloatingBackground];
    ik_state->cfg_main_palettes[IK_PaletteCode_Floating].text                = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_main_palettes[IK_PaletteCode_Floating].text_weak           = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_main_palettes[IK_PaletteCode_Floating].border              = current->colors[IK_ThemeColor_FloatingBorder];
    ik_state->cfg_main_palettes[IK_PaletteCode_ImplicitButton].background    = current->colors[IK_ThemeColor_ImplicitButtonBackground];
    ik_state->cfg_main_palettes[IK_PaletteCode_ImplicitButton].text          = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_main_palettes[IK_PaletteCode_ImplicitButton].text_weak     = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_main_palettes[IK_PaletteCode_ImplicitButton].border        = current->colors[IK_ThemeColor_ImplicitButtonBorder];
    ik_state->cfg_main_palettes[IK_PaletteCode_PlainButton].background       = current->colors[IK_ThemeColor_PlainButtonBackground];
    ik_state->cfg_main_palettes[IK_PaletteCode_PlainButton].text             = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_main_palettes[IK_PaletteCode_PlainButton].text_weak        = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_main_palettes[IK_PaletteCode_PlainButton].border           = current->colors[IK_ThemeColor_PlainButtonBorder];
    ik_state->cfg_main_palettes[IK_PaletteCode_PositivePopButton].background = current->colors[IK_ThemeColor_PositivePopButtonBackground];
    ik_state->cfg_main_palettes[IK_PaletteCode_PositivePopButton].text       = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_main_palettes[IK_PaletteCode_PositivePopButton].text_weak  = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_main_palettes[IK_PaletteCode_PositivePopButton].border     = current->colors[IK_ThemeColor_PositivePopButtonBorder];
    ik_state->cfg_main_palettes[IK_PaletteCode_NegativePopButton].background = current->colors[IK_ThemeColor_NegativePopButtonBackground];
    ik_state->cfg_main_palettes[IK_PaletteCode_NegativePopButton].text       = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_main_palettes[IK_PaletteCode_NegativePopButton].text_weak  = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_main_palettes[IK_PaletteCode_NegativePopButton].border     = current->colors[IK_ThemeColor_NegativePopButtonBorder];
    ik_state->cfg_main_palettes[IK_PaletteCode_NeutralPopButton].background  = current->colors[IK_ThemeColor_NeutralPopButtonBackground];
    ik_state->cfg_main_palettes[IK_PaletteCode_NeutralPopButton].text        = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_main_palettes[IK_PaletteCode_NeutralPopButton].text_weak   = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_main_palettes[IK_PaletteCode_NeutralPopButton].border      = current->colors[IK_ThemeColor_NeutralPopButtonBorder];
    ik_state->cfg_main_palettes[IK_PaletteCode_ScrollBarButton].background   = current->colors[IK_ThemeColor_ScrollBarButtonBackground];
    ik_state->cfg_main_palettes[IK_PaletteCode_ScrollBarButton].text         = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_main_palettes[IK_PaletteCode_ScrollBarButton].text_weak    = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_main_palettes[IK_PaletteCode_ScrollBarButton].border       = current->colors[IK_ThemeColor_ScrollBarButtonBorder];
    ik_state->cfg_main_palettes[IK_PaletteCode_Tab].background               = current->colors[IK_ThemeColor_TabBackground];
    ik_state->cfg_main_palettes[IK_PaletteCode_Tab].text                     = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_main_palettes[IK_PaletteCode_Tab].text_weak                = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_main_palettes[IK_PaletteCode_Tab].border                   = current->colors[IK_ThemeColor_TabBorder];
    ik_state->cfg_main_palettes[IK_PaletteCode_TabInactive].background       = current->colors[IK_ThemeColor_TabBackgroundInactive];
    ik_state->cfg_main_palettes[IK_PaletteCode_TabInactive].text             = current->colors[IK_ThemeColor_Text];
    ik_state->cfg_main_palettes[IK_PaletteCode_TabInactive].text_weak        = current->colors[IK_ThemeColor_TextWeak];
    ik_state->cfg_main_palettes[IK_PaletteCode_TabInactive].border           = current->colors[IK_ThemeColor_TabBorderInactive];
    ik_state->cfg_main_palettes[IK_PaletteCode_DropSiteOverlay].background   = current->colors[IK_ThemeColor_DropSiteOverlay];
    ik_state->cfg_main_palettes[IK_PaletteCode_DropSiteOverlay].text         = current->colors[IK_ThemeColor_DropSiteOverlay];
    ik_state->cfg_main_palettes[IK_PaletteCode_DropSiteOverlay].text_weak    = current->colors[IK_ThemeColor_DropSiteOverlay];
    ik_state->cfg_main_palettes[IK_PaletteCode_DropSiteOverlay].border       = current->colors[IK_ThemeColor_DropSiteOverlay];
  }

  IK_InitStacks(ik_state)
  IK_InitStackNils(ik_state)
}

//- key

internal IK_Key
ik_hot_key(void)
{
  return ik_state->hot_box_key;
}

internal IK_Key
ik_active_key(IK_MouseButtonKind button_kind)
{
  return ik_state->active_box_key[button_kind];
}

internal IK_Key
ik_drop_hot_key(void)
{
  NotImplemented;
}

//- interaction

internal void
ik_kill_action(void)
{
  ik_state->action_slot = IK_ActionSlot_Null;
  for EachEnumVal(IK_MouseButtonKind, k)
  {
    ik_state->active_box_key[k] = ik_key_zero();
  }
}

internal void
ik_kill_focus(void)
{
  for EachEnumVal(IK_MouseButtonKind, k)
  {
    ik_state->focus_hot_box_key[k] = ik_key_zero();
  }
  ik_state->focus_active_box_key = ik_key_zero();
}

//- frame

internal B32
ik_frame(void)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(0,0);

  /////////////////////////////////
  //~ Do per-frame resets

  // ik_drawlist_reset(ik_frame_drawlist());
  arena_clear(ik_frame_arena());

  /////////////////////////////////
  //~ Remake drawing buckets every frame

  dr_begin_frame();
  ik_state->bucket_ui = dr_bucket_make();
  ik_state->bucket_main = dr_bucket_make();

  /////////////////////////////////
  //~ Get events from os
  
  OS_EventList os_events = os_get_events(ik_frame_arena(), 0);
  {
    ik_state->last_window_rect = ik_state->window_rect;
    ik_state->last_window_dim = dim_2f32(ik_state->last_window_rect);
    ik_state->window_rect = os_client_rect_from_window(ik_state->os_wnd, 0);
    ik_state->window_res_changed = ik_state->window_rect.x0 != ik_state->last_window_rect.x0 || ik_state->window_rect.x1 != ik_state->last_window_rect.x1 || ik_state->window_rect.y0 != ik_state->last_window_rect.y0 || ik_state->window_rect.y1 != ik_state->last_window_rect.y1;
    ik_state->window_dim = dim_2f32(ik_state->window_rect);
    ik_state->last_mouse = ik_state->mouse;
    ik_state->mouse = os_window_is_focused(ik_state->os_wnd) ? os_mouse_from_window(ik_state->os_wnd) : v2f32(-100,-100);
    ik_state->mouse_delta = sub_2f32(ik_state->mouse, ik_state->last_mouse);
    ik_state->last_dpi = ik_state->dpi;
    ik_state->dpi = os_dpi_from_window(ik_state->os_wnd);
  }

  /////////////////////////////////
  //~ Calculate avg length in us of last many frames

  U64 frame_time_history_avg_us = 0;
  ProfScope("calculate avg length in us of last many frames")
  {
    U64 num_frames_in_history = Min(ArrayCount(ik_state->frame_time_us_history), ik_state->frame_index);
    U64 frame_time_history_sum_us = 0;
    if(num_frames_in_history > 0)
    {
      for(U64 i = 0; i < num_frames_in_history; i++)
      {
        frame_time_history_sum_us += ik_state->frame_time_us_history[i];
      }
      frame_time_history_avg_us = frame_time_history_sum_us/num_frames_in_history;
    }
  }

  /////////////////////////////////
  //~ Pick target hz

  // pick among a number of sensible targets to snap to, given how well we've been performing
  F32 target_hz = !os_window_is_focused(ik_state->os_wnd) ? 10.0f : os_get_gfx_info()->default_refresh_rate;
  if(ik_state->frame_index > 32)
  {
    F32 possible_alternate_hz_targets[] = {target_hz, 60.f, 120.f, 144.f, 240.f};
    F32 best_target_hz = target_hz;
    S64 best_target_hz_frame_time_us_diff = max_S64;
    for(U64 idx = 0; idx < ArrayCount(possible_alternate_hz_targets); idx += 1)
    {
      F32 candidate = possible_alternate_hz_targets[idx];
      if(candidate <= target_hz)
      {
        U64 candidate_frame_time_us = 1000000/(U64)candidate;
        S64 frame_time_us_diff = (S64)frame_time_history_avg_us - (S64)candidate_frame_time_us;
        if(abs_s64(frame_time_us_diff) < best_target_hz_frame_time_us_diff &&
           frame_time_history_avg_us < candidate_frame_time_us + candidate_frame_time_us/4)
        {
          best_target_hz = candidate;
          best_target_hz_frame_time_us_diff = frame_time_us_diff;
        }
      }
    }
    target_hz = best_target_hz;
  }

  ik_state->frame_dt = 1.f/target_hz;

  /////////////////////////////////
  //~ Fill animation rates

  ik_state->animation.vast_rate = 1 - pow_f32(2, (-60.f * ik_state->frame_dt));
  ik_state->animation.fast_rate = 1 - pow_f32(2, (-50.f * ik_state->frame_dt));
  ik_state->animation.fish_rate = 1 - pow_f32(2, (-40.f * ik_state->frame_dt));
  ik_state->animation.slow_rate = 1 - pow_f32(2, (-30.f * ik_state->frame_dt));
  ik_state->animation.slug_rate = 1 - pow_f32(2, (-15.f * ik_state->frame_dt));
  ik_state->animation.slaf_rate = 1 - pow_f32(2, (-8.f  * ik_state->frame_dt));

  // begin measuring actual per-frame work
  U64 begin_time_us = os_now_microseconds();

  /////////////////////////////////
  //~ Build UI

  ////////////////////////////////
  //- build event list for UI

  UI_EventList ui_events = {0};
  for(OS_Event *os_evt = os_events.first; os_evt != 0; os_evt = os_evt->next)
  {
    UI_Event ui_evt = {0};

    UI_EventKind kind = UI_EventKind_Null;
    switch(os_evt->kind)
    {
      default:{}break;
      case OS_EventKind_Press:     {kind = UI_EventKind_Press;}break;
      case OS_EventKind_Release:   {kind = UI_EventKind_Release;}break;
      case OS_EventKind_MouseMove: {kind = UI_EventKind_MouseMove;}break;
      case OS_EventKind_Text:      {kind = UI_EventKind_Text;}break;
      case OS_EventKind_Scroll:    {kind = UI_EventKind_Scroll;}break;
      case OS_EventKind_FileDrop:  {kind = UI_EventKind_FileDrop;}break;
    }

    ui_evt.kind         = kind;
    ui_evt.key          = os_evt->key;
    ui_evt.modifiers    = os_evt->modifiers;
    ui_evt.string       = os_evt->character ? str8_from_32(ui_build_arena(), str32(&os_evt->character, 1)) : str8_zero();
    ui_evt.paths        = str8_list_copy(ui_build_arena(), &os_evt->strings);
    ui_evt.pos          = os_evt->pos;
    ui_evt.delta_2f32   = os_evt->delta;
    ui_evt.timestamp_us = os_evt->timestamp_us;

    if(ui_evt.key == OS_Key_Backspace && !(ui_evt.modifiers&OS_Modifier_Ctrl) && ui_evt.kind == UI_EventKind_Press)
    {
      ui_evt.kind       = UI_EventKind_Edit;
      ui_evt.flags      = UI_EventFlag_Delete | UI_EventFlag_KeepMark;
      ui_evt.delta_unit = UI_EventDeltaUnit_Char;
      ui_evt.delta_2s32 = v2s32(-1,0);
    }

    if(ui_evt.key == OS_Key_Backspace && (ui_evt.modifiers&OS_Modifier_Ctrl) && ui_evt.kind == UI_EventKind_Press)
    {
      ui_evt.kind       = UI_EventKind_Edit;
      ui_evt.flags      = UI_EventFlag_Delete | UI_EventFlag_KeepMark;
      ui_evt.delta_unit = UI_EventDeltaUnit_Word;
      ui_evt.delta_2s32 = v2s32(-1,0);
    }

    if(ui_evt.kind == UI_EventKind_Text)
    {
      ui_evt.flags = UI_EventFlag_KeepMark;
    }

    if(ui_evt.key == OS_Key_Return && ui_evt.kind == UI_EventKind_Press)
    {
      ui_evt.slot = UI_EventActionSlot_Accept;
    }

    if(ui_evt.key == OS_Key_A && (ui_evt.modifiers & OS_Modifier_Ctrl) && ui_evt.kind == UI_EventKind_Press)
    {
      ui_evt.kind       = UI_EventKind_Navigate;
      ui_evt.flags      = UI_EventFlag_KeepMark;
      ui_evt.delta_unit = UI_EventDeltaUnit_Whole;
      ui_evt.delta_2s32 = v2s32(1,0);

      UI_Event ui_evt_0 = ui_evt;
      ui_evt_0.flags      = 0;
      ui_evt_0.delta_unit = UI_EventDeltaUnit_Whole;
      ui_evt_0.delta_2s32 = v2s32(-1,0);
      ui_event_list_push(ui_build_arena(), &ui_events, &ui_evt_0);
    }

    if(ui_evt.key == OS_Key_X && (ui_evt.modifiers & OS_Modifier_Ctrl) && ui_evt.kind == UI_EventKind_Press)
    {
      ui_evt.kind  = UI_EventKind_Edit;
      ui_evt.flags = UI_EventFlag_Delete | UI_EventFlag_Copy | UI_EventFlag_KeepMark;
    }

    if(ui_evt.key == OS_Key_C && (ui_evt.modifiers & OS_Modifier_Ctrl) && ui_evt.kind == UI_EventKind_Press)
    {
      ui_evt.kind       = UI_EventKind_Edit;
      ui_evt.flags      = UI_EventFlag_Copy | UI_EventFlag_KeepMark;
      ui_evt.delta_unit = UI_EventDeltaUnit_Char;
      ui_evt.delta_2s32 = v2s32(0,0);
    }

    if(ui_evt.key == OS_Key_V && (ui_evt.modifiers & OS_Modifier_Ctrl) && ui_evt.kind == UI_EventKind_Press)
    {
      ui_evt.kind   = UI_EventKind_Edit;
      ui_evt.flags  = UI_EventFlag_Paste;
      // ui_evt.string = os_get_clipboard_text(ui_build_arena());
    }

    if(ui_evt.key == OS_Key_Left && ui_evt.kind == UI_EventKind_Press)
    {
      ui_evt.kind       = UI_EventKind_Navigate;
      ui_evt.flags      = 0;
      ui_evt.delta_unit = ui_evt.modifiers & OS_Modifier_Ctrl ? UI_EventDeltaUnit_Word : UI_EventDeltaUnit_Char;
      ui_evt.delta_2s32 = v2s32(-1,0);
    }

    if(ui_evt.key == OS_Key_Right && ui_evt.kind == UI_EventKind_Press)
    {
      ui_evt.kind       = UI_EventKind_Navigate;
      ui_evt.flags      = 0;
      ui_evt.delta_unit = ui_evt.modifiers & OS_Modifier_Ctrl ? UI_EventDeltaUnit_Word : UI_EventDeltaUnit_Char;
      ui_evt.delta_2s32 = v2s32(1,0);
    }

    // k
    if(ui_evt.key == OS_Key_Up && ui_evt.kind == UI_EventKind_Press)
    {
      ui_evt.kind       = UI_EventKind_Navigate;
      ui_evt.flags      = 0;
      ui_evt.delta_unit = UI_EventDeltaUnit_Line;
      ui_evt.delta_2s32 = v2s32(0,-1);
    }

    // k
    if(ui_evt.key == OS_Key_Down && ui_evt.kind == UI_EventKind_Press)
    {
      ui_evt.kind       = UI_EventKind_Navigate;
      ui_evt.flags      = 0;
      ui_evt.delta_unit = UI_EventDeltaUnit_Line;
      ui_evt.delta_2s32 = v2s32(0,1);
    }

    UI_EventNode *evt_node = ui_event_list_push(ui_build_arena(), &ui_events, &ui_evt);

    if(os_evt->kind == OS_EventKind_WindowClose) {ik_state->window_should_close = 1;}
  }

  ////////////////////////////////
  //- begin build UI

  {
    // gather font info
    FNT_Tag main_font = ik_font_from_slot(IK_FontSlot_Main);
    F32 main_font_size = ik_font_size_from_slot(IK_FontSlot_Main);
    FNT_Tag icon_font = ik_font_from_slot(IK_FontSlot_Icons);

    // build icon info
    UI_IconInfo icon_info = {0};
    {
      icon_info.icon_font = icon_font;
      icon_info.icon_kind_text_map[UI_IconKind_RightArrow]  = ik_icon_kind_text_table[IK_IconKind_RightScroll];
      icon_info.icon_kind_text_map[UI_IconKind_DownArrow]   = ik_icon_kind_text_table[IK_IconKind_DownScroll];
      icon_info.icon_kind_text_map[UI_IconKind_LeftArrow]   = ik_icon_kind_text_table[IK_IconKind_LeftScroll];
      icon_info.icon_kind_text_map[UI_IconKind_UpArrow]     = ik_icon_kind_text_table[IK_IconKind_UpScroll];
      icon_info.icon_kind_text_map[UI_IconKind_RightCaret]  = ik_icon_kind_text_table[IK_IconKind_RightCaret];
      icon_info.icon_kind_text_map[UI_IconKind_DownCaret]   = ik_icon_kind_text_table[IK_IconKind_DownCaret];
      icon_info.icon_kind_text_map[UI_IconKind_LeftCaret]   = ik_icon_kind_text_table[IK_IconKind_LeftCaret];
      icon_info.icon_kind_text_map[UI_IconKind_UpCaret]     = ik_icon_kind_text_table[IK_IconKind_UpCaret];
      icon_info.icon_kind_text_map[UI_IconKind_CheckHollow] = ik_icon_kind_text_table[IK_IconKind_CheckHollow];
      icon_info.icon_kind_text_map[UI_IconKind_CheckFilled] = ik_icon_kind_text_table[IK_IconKind_CheckFilled];
    }

    UI_WidgetPaletteInfo widget_palette_info = {0};
    {
      widget_palette_info.tooltip_palette   = ik_ui_palette_from_code(IK_PaletteCode_Floating);
      widget_palette_info.ctx_menu_palette  = ik_ui_palette_from_code(IK_PaletteCode_Floating);
      widget_palette_info.scrollbar_palette = ik_ui_palette_from_code(IK_PaletteCode_ScrollBarButton);
    }

    // build animation info
    UI_AnimationInfo animation_info = {0};
    {
      animation_info.hot_animation_rate     = ik_state->animation.fast_rate;
      animation_info.active_animation_rate  = ik_state->animation.fast_rate;
      animation_info.focus_animation_rate   = 1.f;
      animation_info.tooltip_animation_rate = ik_state->animation.fast_rate;
      animation_info.menu_animation_rate    = ik_state->animation.fast_rate;
      animation_info.scroll_animation_rate  = ik_state->animation.fast_rate;
    }

    // begin & push initial stack values
    ui_begin_build(ik_state->os_wnd, &ui_events, &icon_info, &widget_palette_info, &animation_info, ik_state->frame_dt);

    ui_push_font(main_font);
    ui_push_font_size(main_font_size*0.85);
    ui_push_text_raster_flags(FNT_RasterFlag_Smooth);
    ui_push_text_padding(floor_f32(ui_top_font_size()*0.3f));
    ui_push_pref_width(ui_px(floor_f32(ui_top_font_size()*20.f), 1.f));
    ui_push_pref_height(ui_px(floor_f32(ui_top_font_size()*1.65f), 1.f));
    ui_push_palette(ik_ui_palette_from_code(IK_PaletteCode_Base));
  }

  ////////////////////////////////
  //~ Global stacks

  // font
  FNT_Tag main_font = ik_font_from_slot(IK_FontSlot_Main);
  F32 main_font_size = ik_font_size_from_slot(IK_FontSlot_Main);
  FNT_Tag icon_font = ik_font_from_slot(IK_FontSlot_Icons);

  // widget palette
  IK_WidgetPaletteInfo widget_palette_info = {0};
  {
    widget_palette_info.tooltip_palette   = ik_palette_from_code(IK_PaletteCode_Floating);
    widget_palette_info.ctx_menu_palette  = ik_palette_from_code(IK_PaletteCode_Floating);
    widget_palette_info.scrollbar_palette = ik_palette_from_code(IK_PaletteCode_ScrollBarButton);
  }
  ik_state->widget_palette_info = widget_palette_info;


  ik_push_font(main_font);
  ik_push_font_size(main_font_size);
  ik_push_text_padding(main_font_size*0.6f);
  ik_push_palette(ik_palette_from_code(IK_PaletteCode_Base));

  ////////////////////////////////
  //~ Load frame

  IK_Frame *frame = ik_state->active_frame;
  if(frame == 0)
  {
    // TODO(Next): check if default tyml is there and can be loaded, otherwise we create a new frame
    String8 binary_path = os_get_process_info()->binary_path; // only directory
    String8 default_path = push_str8f(ik_frame_arena(), "%S/default.tyml", binary_path);
    if(os_file_path_exists(default_path))
    {
      // TODO(Next): we want to handle parse error here
      frame = ik_frame_from_tyml(default_path);
      ik_state->active_frame = frame;
    }
    else
    {
      frame = ik_frame_alloc();
      ik_state->active_frame = frame;
    }
  }

  ik_push_frame(frame);

  ////////////////////////////////
  //~ Frame stacks

  ik_push_background_color(frame->cfg.background_color);
  ik_push_stroke_size(frame->cfg.stroke_size);
  ik_push_stroke_color(frame->cfg.stroke_color);
  ik_push_font_slot(frame->cfg.text_font_slot);

  ////////////////////////////////
  //~ Unpack camera

  IK_Camera *camera = &frame->camera;
  // NOTE(k): since we are not using view_mat, it's a indentity matrix, so proj_mat == proj_view_mat
  ik_state->proj_mat = make_orthographic_vulkan_4x4f32(camera->rect.x0, camera->rect.x1, camera->rect.y1, camera->rect.y0, camera->zn, camera->zf);
  ik_state->proj_mat_inv = inverse_4x4f32(ik_state->proj_mat);
  ik_state->mouse_in_world = ik_mouse_in_world(ik_state->proj_mat_inv);
  Vec2F32 camera_rect_dim = dim_2f32(camera->rect);
  ik_state->world_to_screen_ratio = (Vec2F32){camera_rect_dim.x/ik_state->window_dim.x, camera_rect_dim.y/ik_state->window_dim.y};
  ik_state->mouse_delta_in_world.x = ik_state->mouse_delta.x*ik_state->world_to_screen_ratio.x;
  ik_state->mouse_delta_in_world.y = ik_state->mouse_delta.y*ik_state->world_to_screen_ratio.y;

  ////////////////////////////////
  //~ Messages Processing (purge+animating)

  for(IK_Message *msg = ik_state->messages.first, *next = 0;
      msg != 0;
      msg = next)
  {
    next = msg->next;
    B32 is_top = msg == ik_state->messages.first;
    if(msg->expired && msg->expired_t == 1.f)
    {
      DLLRemove(ik_state->messages.first, ik_state->messages.last, msg);
      ik_state->messages.count--;
      continue;
    }

    msg->create_t += ik_state->animation.fast_rate * (1.f - msg->create_t);
    msg->expired_t += ik_state->animation.slow_rate * ((F32)msg->expired - msg->expired_t);
    if(is_top) msg->elapsed_sec += ik_state->frame_dt;
    msg->expired = msg->elapsed_sec > 3.0;

    F32 epsilon = 1e-2;
    msg->create_t = abs_f32(1.f-msg->create_t) < epsilon ? 1.f : msg->create_t;
    msg->expired_t = abs_f32((F32)msg->expired-msg->expired_t) < epsilon ? 1.f : msg->expired_t;
  }

  ////////////////////////////////
  //~ Build Debug UI

  ik_ui_man_page();
  ik_ui_toolbar();
  ik_ui_bottom_bar();
  ik_ui_notification();
  ik_ui_selection();
  ik_ui_inspector();
  ik_ui_stats();
  ik_ui_box_ctx_menu();
  ik_ui_g_ctx_menu();
  ik_ui_version();

  ////////////////////////////////
  //~ Copy events
  //  NOTE(k): we are using the same ui_events as ui used, so any event eaten can be detected

  ik_state->events = &ui_events; // NOTE(k): ui_events is stored on the stack

  ////////////////////////////////
  //~ Main scene building

  // unpack ctx
  B32 cursor_override = 0;
  OS_Cursor next_cursor = 0;

  ////////////////////////////////
  //- window resized

  if(ik_state->window_res_changed)
  {
    if(ik_state->window_dim.x != 0 && ik_state->window_dim.y != 0)
    {
      Vec2F32 camera_dim = dim_2f32(camera->target_rect);
      F32 area = camera_dim.x*camera_dim.y;
      F32 ratio = ik_state->window_dim.x/ik_state->window_dim.y;
      F32 y = sqrt_f32(area/ratio);
      F32 x = ratio*y;
      camera->target_rect.y1 = camera->target_rect.y0 + y;
      camera->target_rect.x1 = camera->target_rect.x0 + x;
    }
  }

  ////////////////////////////////
  //- camera control

  {
    typedef struct IK_CameraDrag IK_CameraDrag;
    struct IK_CameraDrag
    {
      Rng2F32 drag_start_rect;
      Vec2F32 drag_start_mouse;
    };
    B32 is_zooming = 0;
    B32 can_pan = ik_tool() == IK_ToolKind_Hand;
    B32 is_panning = camera->is_panning;
    for(UI_EventNode *n = ik_state->events->first, *next = 0; n != 0; n = next)
    {
      B32 taken = 0;
      next = n->next;
      UI_Event *evt = &n->v;

      // camera pan begin
      if(can_pan && !is_panning && evt->kind == UI_EventKind_Press && evt->key == OS_Key_LeftMouseButton)
      {
        IK_CameraDrag drag = {camera->target_rect, ik_state->mouse};
        ik_store_drag_struct(&drag);
        camera->anim_rate = ik_state->animation.fast_rate;
        is_panning = 1;
        taken = 1;
      }

      // camera pan end
      if(is_panning && (evt->kind == UI_EventKind_Release && evt->key == OS_Key_LeftMouseButton))
      {
        is_panning = 0;
        taken = 1;
      }

      // camera zoom in/out
      if(evt->kind == UI_EventKind_Scroll && evt->modifiers == OS_Modifier_Ctrl)
      {
        is_zooming = 1;

        Vec2F32 delta = evt->delta_2f32;
        Vec2S16 delta16 = v2s16((S16)(delta.x/30.f), (S16)(delta.y/30.f));
        if(delta.x > 0 && delta16.x == 0) { delta16.x = +1; }
        if(delta.x < 0 && delta16.x == 0) { delta16.x = -1; }
        if(delta.y > 0 && delta16.y == 0) { delta16.y = +1; }
        if(delta.y < 0 && delta16.y == 0) { delta16.y = -1; }

        // get normalized rect
        Rng2F32 rect = camera->rect;
        Vec2F32 rect_center = center_2f32(camera->rect);
        Vec2F32 shift = {-rect_center.x, -rect_center.y};
        Vec2F32 shift_inv = rect_center;
        rect = shift_2f32(rect, shift);

        F32 zoom_step = mix_1f32(camera->min_zoom_step, camera->max_zoom_step, camera->zoom_t);
        F32 scale_unit = 1.f+zoom_step;
        F32 scale = pow_f32(scale_unit, (F32)delta.y);
        rect.x0 *= scale;
        rect.x1 *= scale;
        rect.y0 *= scale;
        rect.y1 *= scale;
        rect = shift_2f32(rect, shift_inv);

        // anchor mouse pos in world
        Mat4x4F32 proj_mat_after = make_orthographic_vulkan_4x4f32(rect.x0, rect.x1, rect.y1, rect.y0, camera->zn, camera->zf);
        Mat4x4F32 proj_mat_inv_after = inverse_4x4f32(proj_mat_after);
        Vec2F32 mouse_in_world_after = ik_mouse_in_world(proj_mat_inv_after);
        Vec2F32 world_delta = sub_2f32(ik_state->mouse_in_world, mouse_in_world_after);
        rect = shift_2f32(rect, world_delta);
        camera->target_rect = rect;
        camera->anim_rate = ik_state->animation.fast_rate;

        taken = 1;
      }

      if(taken)
      {
        ui_eat_event_node(ik_state->events, n);
      }
    }

    // do panning
    is_panning = is_panning && can_pan;
    if(is_panning)
    {
      IK_CameraDrag drag = *ik_get_drag_struct(IK_CameraDrag);
      Vec2F32 delta = sub_2f32(drag.drag_start_mouse, ik_state->mouse);
      delta.x *= ik_state->world_to_screen_ratio.x;
      delta.y *= ik_state->world_to_screen_ratio.y;
      Rng2F32 rect = shift_2f32(drag.drag_start_rect, delta);
      camera->target_rect = rect;
      camera->rect = rect;
    }
    camera->is_panning = is_panning;

    // camera animations
    camera->rect.x0 += camera->anim_rate * (camera->target_rect.x0-camera->rect.x0);
    camera->rect.x1 += camera->anim_rate * (camera->target_rect.x1-camera->rect.x1);
    camera->rect.y0 += camera->anim_rate * (camera->target_rect.y0-camera->rect.y0);
    camera->rect.y1 += camera->anim_rate * (camera->target_rect.y1-camera->rect.y1);
    camera->zoom_t  += ik_state->animation.slow_rate * ((F32)is_zooming-camera->zoom_t);
  }

  ////////////////////////////////
  //- ik state begin build

  // reset hot key every frame if not active key
  {
    B32 has_active = 0;
    for EachEnumVal(IK_MouseButtonKind, k)
    {
      if(!ik_key_match(ik_state->active_box_key[k], ik_key_zero()))
      {
        has_active = 1;
      }
    }
    if(!has_active)
    {
      ik_state->hot_box_key = ik_key_zero();
    }
  }

  // clear per-frame state
  ik_state->first_box_selected = 0;
  ik_state->last_box_selected = 0;
  ik_state->selected_box_count = 0;
  ik_state->selection_bounds = r2f32p(inf32(), inf32(), neg_inf32(), neg_inf32());

  ////////////////////////////////
  //- box interaction

  {
    // NOTE(k): since we are now using pixel-perfect object picking, the order don't matter here
    IK_Box *roots[3] = {frame->select, frame->box_list.last, frame->blank};

    ////////////////////////////////
    //~ First pass to calculate size in place & handle box signal

    for(U64 i = 0; i < ArrayCount(roots); i++)
    {
      IK_Box *root = roots[i];
      for(IK_Box *box = root, *next = 0; box != 0; box = next)
      {
        next = box->prev;
        box->sig = ik_signal_from_box(box);

        /////////////////////////////////
        // Handle deletion

        B32 delete = 0;
        if(!ik_key_match(box->key, ik_state->focus_active_box_key) && box->flags&IK_BoxFlag_PruneZeroText && box->string.size == 0)
        {
          delete = 1;
        }
        if((box->sig.f&IK_SignalFlag_Delete || box->deleted) && !(box->flags&IK_BoxFlag_OmitDeletion))
        {
          delete = 1;
        }

        if(delete)
        {
          ik_box_release(box);
          continue;
        }

        ////////////////////////////////
        // calc size in place

        if(box->flags&IK_BoxFlag_FitViewport)
        {
          box->position = camera->rect.p0;
          box->rect_size = dim_2f32(camera->rect);
          box->ratio = box->rect_size.x/box->rect_size.y;
        }

        if(box->flags & IK_BoxFlag_DrawText)
        {
          ik_update_text(box);
        }

        if(box->flags & IK_BoxFlag_DrawStroke)
        {
          ik_update_stroke(box);
        }

        if(box->flags & IK_BoxFlag_DrawArrow)
        {
          ik_update_arrow(box);
        }

        if(box->flags & IK_BoxFlag_FitText)
        {
          box->rect_size = box->text_bounds;
          box->ratio = box->rect_size.x/box->rect_size.y;
        }

        if(box->flags & IK_BoxFlag_ClampBotTextDimX)
        {
          box->rect_size.x = ClampBot(box->text_bounds.x, box->rect_size.x);
          box->ratio = box->rect_size.x/box->rect_size.y;
        }

        if(box->flags & IK_BoxFlag_ClampBotTextDimY)
        {
          box->rect_size.y = ClampBot(box->text_bounds.y, box->rect_size.y);
          box->ratio = box->rect_size.x/box->rect_size.y;
        }

        ////////////////////////////////
        // dragging -> reposition

        // TODO(Next): IK_BoxFlag_DragToPosition and IK_BoxFlag_FitChildren are in conflict
        if((box->sig.f&IK_SignalFlag_LeftDragging) &&
            box->flags&IK_BoxFlag_DragToPosition &&
            (!ik_key_match(box->key, ik_state->focus_active_box_key)))
        {
          if(box->sig.f & IK_SignalFlag_Pressed)
          {
            IK_BoxDrag drag = {ik_rect_from_box(box), ik_state->mouse, ik_state->mouse_in_world, 0};
            ik_store_drag_struct(&drag);
            ik_state->action_slot = IK_ActionSlot_Center;
          }
          else if(ik_state->action_slot == IK_ActionSlot_Center)
          {
            IK_BoxDrag drag = *ik_get_drag_struct(IK_BoxDrag);
            // NOTE(k): camera rect could be animating, casuing world mouse delta change even mouse in screen didn't change
            F32 epsilon = (ik_state->dpi/96.f)*2.f;
            F32 drag_px = length_2f32(sub_2f32(drag.drag_start_mouse, ik_state->mouse));
            if(drag.drag_started ? 1 : drag_px > epsilon)
            {
              Vec2F32 delta = sub_2f32(ik_state->mouse_in_world, drag.drag_start_mouse_in_world);
              Vec2F32 position = add_2f32(drag.drag_start_rect.p0, delta);
              Vec2F32 position_delta = sub_2f32(position, box->position);
              ik_box_do_translate(box, position_delta);

              drag.drag_started = 1;
              ik_store_drag_struct(&drag);
            }
          }
        }

        ////////////////////////////////
        // we can now compute box rect & calc ratio

        Rng2F32 rect = ik_rect_from_box(box);

        if(!(box->flags&IK_BoxFlag_FixedRatio))
        {
          box->ratio = box->rect_size.x/box->rect_size.y;
        }

        ////////////////////////////////
        // double clicked? -> set camera focus

        if(box->flags&IK_BoxFlag_DoubleClickToCenter && box->sig.f&IK_SignalFlag_DoubleClicked)
        {
          Vec2F32 viewport_center = center_2f32(frame->camera.target_rect);
          Vec2F32 box_center = center_2f32(rect);
          Vec2F32 delta = sub_2f32(box_center, viewport_center);
          ik_kill_action();

          frame->camera.target_rect.p0 = add_2f32(frame->camera.target_rect.p0, delta);
          frame->camera.target_rect.p1 = add_2f32(frame->camera.target_rect.p1, delta);
          frame->camera.anim_rate = ik_state->animation.slug_rate;
        }

        ////////////////////////////////
        // push to global selection list if box is selected

        if(box->sig.f&IK_SignalFlag_Select && box != frame->blank)
        {
          DLLPushFront_NP(ik_state->first_box_selected, ik_state->last_box_selected, box, select_next, select_prev);
          ik_state->selected_box_count++;
          ik_state->selection_bounds.x0 = Min(ik_state->selection_bounds.x0, rect.x0);
          ik_state->selection_bounds.x1 = Max(ik_state->selection_bounds.x1, rect.x1);
          ik_state->selection_bounds.y0 = Min(ik_state->selection_bounds.y0, rect.y0);
          ik_state->selection_bounds.y1 = Max(ik_state->selection_bounds.y1, rect.y1);
        }

        ////////////////////////////////
        // animation (hot_t, ...)
        
        B32 is_hot = ik_key_match(box->key, ik_state->hot_box_key);
        B32 is_active = ik_key_match(box->key, ik_state->active_box_key[IK_MouseButtonKind_Left]);
        B32 is_focus_hot = ik_key_match(box->key, ik_state->focus_hot_box_key[IK_MouseButtonKind_Left]);
        B32 is_focus_active = ik_key_match(box->key, ik_state->focus_active_box_key);
        B32 is_disabled = box->flags&IK_BoxFlag_Disabled;

        box->hot_t += ik_state->animation.fast_rate * ((F32)is_hot-box->hot_t);
        box->active_t += ik_state->animation.fast_rate * ((F32)is_active-box->active_t);
        box->focus_hot_t += ik_state->animation.fast_rate * ((F32)is_focus_hot-box->focus_hot_t);
        box->focus_active_t += ik_state->animation.fast_rate * ((F32)is_focus_active-box->focus_active_t);
        box->disabled_t += ik_state->animation.fast_rate * ((F32)is_disabled-box->disabled_t);

        F32 epsilon = 1e-2;
        box->hot_t = abs_f32(is_hot-box->hot_t) < epsilon ? (F32)is_hot : box->hot_t;
        box->active_t = abs_f32(is_hot-box->active_t) < epsilon ? (F32)is_hot : box->active_t;
        box->focus_hot_t = abs_f32(is_hot-box->focus_hot_t) < epsilon ? (F32)is_hot : box->focus_hot_t;
        box->focus_active_t = abs_f32(is_hot-box->focus_active_t) < epsilon ? (F32)is_hot : box->focus_active_t;
        box->disabled_t = abs_f32(is_hot-box->disabled_t) < epsilon ? (F32)is_hot : box->disabled_t;
      }
    }

    ////////////////////////////////
    //~ Second pass to calculate downwards-dependent size 

    for(U64 i = 0; i < ArrayCount(roots); i++)
    {
      IK_Box *root = roots[i];
      for(IK_Box *box = root, *next = 0; box != 0; box = next)
      {
        next = box->prev;

        if(box->flags&IK_BoxFlag_FitChildren)
        {
          Rng2F32 bounds = {inf32(), inf32(), neg_inf32(), neg_inf32()};
          for(IK_Box *child = box->group_first; child != 0; child = child->group_next)
          {
            Rng2F32 rect = ik_rect_from_box(child);
            bounds.x0 = Min(rect.x0, bounds.x0);
            bounds.x1 = Max(rect.x1, bounds.x1);
            bounds.y0 = Min(rect.y0, bounds.y0);
            bounds.y1 = Max(rect.y1, bounds.y1);
          }
          box->position = bounds.p0;
          box->rect_size = dim_2f32(bounds);
          box->ratio = box->rect_size.x/box->rect_size.y;
        }
      }
    }

    /////////////////////////////////
    //~ Reset active if active box is disabled

    for EachEnumVal(IK_MouseButtonKind, k)
    {
      if(!ik_key_match(ik_state->active_box_key[k], ik_key_zero()))
      {
        IK_Box *box = ik_box_from_key(ik_state->active_box_key[k]);
        if(box && box->flags&IK_BoxFlag_Disabled)
        {
          ik_state->active_box_key[k] = ik_key_zero();
        }
      }
    }

    /////////////////////////////////
    //~ Reset focus-hot key if focus-hot box is disabled

    for EachEnumVal(IK_MouseButtonKind, k)
    {
      if(!ik_key_match(ik_state->focus_hot_box_key[k], ik_key_zero()))
      {
        IK_Box *box = ik_box_from_key(ik_state->focus_hot_box_key[k]);
        if(box && box->flags&IK_BoxFlag_Disabled)
        {
          ik_state->focus_hot_box_key[k] = ik_key_zero();
        }
      }
    }

    /////////////////////////////////
    //~ Active/focus_hot box is pruned, reset keys

    for EachEnumVal(IK_MouseButtonKind, k)
    {
      if(!ik_key_match(ik_state->active_box_key[k], ik_key_zero()))
      {
        IK_Box *box = ik_box_from_key(ik_state->active_box_key[k]);
        if(!box) ik_state->active_box_key[k] = ik_key_zero();
      }
    }
    for EachEnumVal(IK_MouseButtonKind, k)
    {
      if(!ik_key_match(ik_state->focus_hot_box_key[k], ik_key_zero()))
      {
        IK_Box *box = ik_box_from_key(ik_state->focus_hot_box_key[k]);
        if(!box) ik_state->focus_hot_box_key[k] = ik_key_zero();
      }
    }

    /////////////////////////////////
    //~ Blank/Select box update and interaction

    IK_Box *blank = frame->blank;
    IK_Box *select = frame->select;

    /////////////////////////////////
    //~ Scrolled fall-through -> scroll viewport

    for(UI_EventNode *n = ik_state->events->first, *next = 0; n != 0; n = next)
    {
      B32 taken = 0;
      next = n->next;
      UI_Event *evt = &n->v;

      // up-down scroll
      if(evt->kind == UI_EventKind_Scroll && evt->modifiers != OS_Modifier_Ctrl)
      {
        Vec2F32 delta = evt->delta_2f32;
        Vec2S16 delta16 = v2s16((S16)(delta.x/30.f), (S16)(delta.y/30.f));
        if(delta.x > 0 && delta16.x == 0) { delta16.x = +1; }
        if(delta.x < 0 && delta16.x == 0) { delta16.x = -1; }
        if(delta.y > 0 && delta16.y == 0) { delta16.y = +1; }
        if(delta.y < 0 && delta16.y == 0) { delta16.y = -1; }

        F32 scroll_t = ui_anim(ui_key_from_stringf(ui_key_zero(), "camera_scroll_t"), 1, .reset = 0, .rate = ik_state->animation.slow_rate);
        F32 y_pct = mix_1f32(0.01, 0.04, scroll_t);
        F32 step_px = ik_state->window_dim.y*y_pct*(F32)delta16.y;
        F32 step_world = step_px * ik_state->world_to_screen_ratio.y;

        camera->anim_rate = ik_state->animation.fast_rate;
        camera->target_rect.y0 += step_world;
        camera->target_rect.y1 += step_world;
        taken = 1;
      }

      if(taken)
      {
        ui_eat_event_node(ik_state->events, n);
      }
    }

    /////////////////////////////////
    //~ Mouse right pressed on blank -> unset focus_hot & focus_active & open global ctx menu

    if(blank->sig.f&IK_SignalFlag_RightPressed)
    {
      ik_kill_focus();
      ik_state->g_ctx_menu_open = 1;
    }
    if(blank->sig.f&IK_SignalFlag_LeftPressed && ik_state->g_ctx_menu_open)
    {
      ik_state->g_ctx_menu_open = 0;
    }
    for EachEnumVal(IK_MouseButtonKind, k)
    {
      if(!ik_key_match(ik_state->focus_hot_box_key[k], ik_key_zero()))
      {
        ik_state->g_ctx_menu_open = 0;
        break;
      }
    }

    /////////////////////////////////
    //~ Hot keys

    if(os_window_is_focused(ik_state->os_wnd) && ik_key_match(ik_state->focus_active_box_key, ik_key_zero()))
    {
      for(UI_EventNode *evt_node = ik_state->events->first, *next_evt_node = 0;
          evt_node != 0;
          evt_node = next_evt_node)
      {
        next_evt_node = evt_node->next;
        UI_Event *evt = &evt_node->v;

        B32 eat = 0;
        if(evt->kind == UI_EventKind_Press && evt->key == OS_Key_1 && evt->modifiers == 0)
        {
          ik_state->tool = IK_ToolKind_Hand;
          eat = 1;
        }
        if(evt->kind == UI_EventKind_Press && evt->key == OS_Key_2 && evt->modifiers == 0)
        {
          ik_state->tool = IK_ToolKind_Selection;
          eat = 1;
        }
        if(evt->kind == UI_EventKind_Press && evt->key == OS_Key_3 && evt->modifiers == 0)
        {
          ik_state->tool = IK_ToolKind_Rectangle;
          eat = 1;
        }
        if(evt->kind == UI_EventKind_Press && evt->key == OS_Key_4 && evt->modifiers == 0)
        {
          ik_state->tool = IK_ToolKind_Draw;
          eat = 1;
        }
        if(evt->kind == UI_EventKind_Press && evt->key == OS_Key_5 && evt->modifiers == 0)
        {
          ik_state->show_man_page = !ik_state->show_man_page;
          eat = 1;
        }
        // if(evt->kind == UI_EventKind_Press && evt->key == OS_Key_I && evt->modifiers == 0)
        // {
        //   ik_state->tool = IK_ToolKind_InsertImage;
        //   eat = 1;
        // }
        // if(evt->kind == UI_EventKind_Press && evt->key == OS_Key_E && evt->modifiers == 0)
        // {
        //   ik_state->tool = IK_ToolKind_Eraser;
        //   eat = 1;
        // }
        if(evt->kind == UI_EventKind_Press && evt->key == OS_Key_Tick && (evt->modifiers & OS_Modifier_Ctrl))
        {
          ik_state->show_stats = !ik_state->show_stats;
          eat = 1;
        }
        if(eat) ui_eat_event_node(ik_state->events, evt_node);
      }
    }

    /////////////////////////////////
    //~ Selection

    //- mouse dragging on blank -> selecting boxes
    if(ik_tool() == IK_ToolKind_Selection && ik_dragging(blank->sig))
    {
      typedef struct IK_SelectionDrag IK_SelectionDrag;
      struct IK_SelectionDrag
      {
        Vec2F32 anchor;
      };

      if(ik_pressed(blank->sig))
      {
        IK_SelectionDrag drag = {.anchor = ik_state->mouse_in_world };
        ui_store_drag_struct(&drag);
      }
      IK_SelectionDrag drag = *ui_get_drag_struct(IK_SelectionDrag);

      Vec2F32 p0 = drag.anchor;
      Vec2F32 p1 = ik_state->mouse_in_world;
      if(p0.x > p1.x) Swap(F32, p0.x, p1.x);
      if(p0.y > p1.y) Swap(F32, p0.y, p1.y);

      // in world position
      ik_state->selection_rect.p0 = p0;
      ik_state->selection_rect.p1 = p1;

      Rng2F32 rect = {.p0 = ik_screen_pos_from_world(p0), .p1 = ik_screen_pos_from_world(p1)};
      UI_Rect(rect)
        UI_Flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_Floating|UI_BoxFlag_DrawDropShadow)
        UI_Transparency(0.6)
        ui_build_box_from_key(0, ui_key_zero());
    }

    //- selecting and mouse release -> commit state to select box
    if(ik_tool() == IK_ToolKind_Selection && ik_released(blank->sig))
    {
      if(ik_state->selected_box_count > 0)
      {
        ik_state->focus_hot_box_key[IK_MouseButtonKind_Left] = select->key;
        select->flags ^= IK_BoxFlag_Disabled;
        select->group_first = 0;
        select->group_last = 0;
        select->group_children_count = 0;

        for(IK_Box *child = ik_state->first_box_selected;
            child != 0;
            child = child->select_next)
        {
          // FIXME: bug here, if child is already within a group, it can't be selected, it's parent should be selected
          DLLPushFront_NP(select->group_first, select->group_last, child, group_next, group_prev);
          select->group_children_count++;
          child->group = select;
        }
      }
    }

    //- mode is not selection and select is on? -> disable select
    // if(tool != IK_ToolKind_Selection && !(select->flags&IK_BoxFlag_Disabled))
    // {
    //   select->flags |= IK_BoxFlag_Disabled;
    // }

    //- delete sig on select box? -> delete all grouped children
    if(select->sig.f&IK_SignalFlag_Delete || select->deleted)
    {
      for(IK_Box *child = select->group_first, *next = 0; child != 0; child = next)
      {
        next = child->group_next;
        ik_box_release(child);
      }
      select->flags |= IK_BoxFlag_Disabled;
      select->deleted = 0;
    }

    //- select box is not focused -> disable it
    if(!ik_key_match(ik_state->focus_hot_box_key[IK_MouseButtonKind_Left], select->key))
    {
      select->flags |= IK_BoxFlag_Disabled;
    }

    //- select box is disabled? -> rest its state
    if(select->flags&IK_BoxFlag_Disabled)
    {
      for(IK_Box *child = select->group_first, *next = 0; child != 0; child = next)
      {
        next = child->group_next;
        child->group = 0;
        child->group_next = 0;
        child->group_prev = 0;
      }

      select->rect_size = v2f32(0,0);
      select->group_first = 0;
      select->group_last = 0;
      select->group_children_count = 0;
    }

    //- select box is focused -> flag every selected box 
    if(ik_key_match(ik_state->focus_hot_box_key[IK_MouseButtonKind_Left], select->key))
    {
      for(IK_Box *child = select->group_first; child != 0; child = child->group_next)
      {
        child->sig.f |= IK_SignalFlag_Select;
      }
    }

    /////////////////////////////////
    //~ Tool panel

    //- pen
    if(ik_tool() == IK_ToolKind_Draw && ik_pressed(blank->sig))
    {
      IK_Box *b = ik_stroke();
      ik_kill_action();
      // ik_state->focus_hot_box_key[IK_MouseButtonKind_Left] = b->key;
      ik_state->focus_active_box_key = b->key;
    }

    //- rectangle tool
    if(ik_tool() == IK_ToolKind_Rectangle && ik_pressed(blank->sig))
    {
      F32 font_size = ik_top_font_size();
      F32 font_size_in_world = font_size * ik_state->world_to_screen_ratio.x * 2;
      IK_Box *box = ik_build_box_from_stringf(0, "rect###%I64u", os_now_microseconds());
      box->flags |= (IK_BoxFlag_DrawBackground|
                     IK_BoxFlag_MouseClickable|
                     IK_BoxFlag_ClickToFocus|
                     IK_BoxFlag_DrawText|
                     IK_BoxFlag_WrapText|
                     IK_BoxFlag_ClampBotTextDimX|
                     IK_BoxFlag_ClampBotTextDimY|
                     IK_BoxFlag_DragToPosition|
                     IK_BoxFlag_DragToScaleRectSize);
      box->position = ik_state->mouse_in_world;
      box->rect_size = v2f32(ik_state->world_to_screen_ratio.x, ik_state->world_to_screen_ratio.y);
      box->last_rect_size = box->rect_size;
      box->font_size = font_size_in_world;
      box->text_align = IK_TextAlign_HCenter|IK_TextAlign_VCenter;
      box->disabled_t = 1.0;
      box->ratio = 1.f;
      box->hover_cursor = OS_Cursor_UpDownLeftRight;
      box->draw_frame_index = ik_state->frame_index+1;

      ik_kill_action();
      ik_state->focus_hot_box_key[IK_MouseButtonKind_Left] = box->key;
      ik_state->hot_box_key = box->key;
      ik_state->active_box_key[IK_MouseButtonKind_Left] = box->key;
      ik_state->action_slot = IK_ActionSlot_DownRight;

      IK_BoxDrag drag =
      {
        .drag_start_rect = ik_rect_from_box(box),
        .drag_start_mouse = ik_state->mouse,
        .drag_start_mouse_in_world = ik_state->mouse_in_world,
      };
      ui_store_drag_struct(&drag);
      ik_state->tool = IK_ToolKind_Selection;
    }

    //- text tool (tool or double click on blank to trigger)
    {
      IK_ToolKind tool = ik_tool();
      if((tool == IK_ToolKind_Text && ik_pressed(blank->sig)) ||
         (tool == IK_ToolKind_Selection && blank->sig.f&IK_SignalFlag_LeftDoubleClicked))
      {
        IK_Box *box = ik_text(str8_lit(""), ik_state->mouse_in_world);
        box->draw_frame_index = ik_state->frame_index+1;
        box->disabled_t = 1.0;
        ik_state->focus_hot_box_key[IK_MouseButtonKind_Left] = box->key;
        ik_state->focus_active_box_key = box->key;
        ik_kill_action();
        ik_state->tool = IK_ToolKind_Selection;
      }
    }

    //- arrow tool
    if(ik_tool() == IK_ToolKind_Arrow && ik_pressed(blank->sig))
    {
      IK_Box *b = ik_arrow();
      ik_kill_action();
      ik_state->focus_hot_box_key[IK_MouseButtonKind_Left] = b->key;
      ik_state->focus_active_box_key = b->key;
    }

    //- file drop
    {
      Vec2F32 drop_position = ik_state->mouse;
      String8List paths = ik_file_drop(&drop_position);
      if(paths.node_count > 0)
      {
        // NOTE(k): drag on w32 could make window out of focus 
        os_window_focus(ik_state->os_wnd);

        Vec2F32 position = ik_screen_pos_in_world(ik_state->proj_mat_inv, drop_position);
        F32 offset_x = 0;
        for(String8Node *n = paths.first;
            n != 0;
            n = n->next)
        {
          String8 path = n->string; 

          OS_Handle file = os_file_open(OS_AccessFlag_ShareRead|OS_AccessFlag_Read, path);
          if(!os_handle_match(os_handle_zero(), file))
          {
            FileProperties prop = os_properties_from_file(file);
            String8 encoded = ik_str8_new(prop.size);
            encoded.size = prop.size;
            U64 read = os_file_read(file, (Rng1U64){0, prop.size}, (void*)encoded.str);
            os_file_close(file);


            // decide initial width&height
            F32 default_screen_width = ik_state->window_dim.x * 0.25;
            F32 width = default_screen_width * ik_state->world_to_screen_ratio.x;
            F32 height = width;
            F32 ratio = 1.0;

            // FIXME: we can't use content to hash the key, since we could drag multiple files with the same content
            // we would cause a bug, the first image is sent to decode, and it's dim haven't been solved, but we use the same cached image to create the remaning boxes
            IK_Key key = ik_key_make(ui_hash_from_string(0, path), 0);
            IK_Image *image = ik_image_from_key(key);
            if(!image)
            {
              image = ik_image_push(key);
              image->encoded = encoded;
              ik_image_decode_queue_push(image);
            }
            else
            {
              // image cached? -> set height based on ratio
              ratio = (F32)image->x/image->y;
              height = width/ratio;
            }

            IK_Box *box = ik_image(IK_BoxFlag_DrawBorder, v2f32(position.x+offset_x, position.y), v2f32(width, height), image);
            box->ratio = ratio;
            box->disabled_t = 1.0;
            // TODO(Next): not ideal, fix it later
            box->draw_frame_index = ik_state->frame_index+1;
            offset_x += width/2.0;
          }
        }
      }
    }

    //- paste text/image
    if(ik_paste())
    {
      Temp scratch = scratch_begin(0,0);

      // try pasting text first, then image
      B32 content_is_text = 0;
      B32 content_is_image = 0;
      String8 content = os_get_clipboard_text(scratch.arena);
      content_is_text = content.size > 0;
      if(!content_is_text)
      {
        content = os_get_clipboard_image(scratch.arena);
        content_is_image = content.size > 0;
      }

      // paste text
      if(content_is_text)
      {
        IK_Box *box = ik_text(content, ik_state->mouse_in_world);
        box->draw_frame_index = ik_state->frame_index+1;
        box->disabled_t = 1.0;
      }

      // paste image
      if(content_is_image)
      {
        // decide initial width&height
        F32 default_screen_width = ik_state->window_dim.x * 0.25;
        F32 width = default_screen_width * ik_state->world_to_screen_ratio.x;
        F32 height = width;
        F32 ratio = 1.0;

        IK_Key key = ik_key_make(ui_hash_from_string(0, content), 0);
        IK_Image *image = ik_image_from_key(key);
        if(!image)
        {
          image = ik_image_push(key);
          image->encoded = ik_push_str8_copy(content);
          ik_image_decode_queue_push(image);
        }
        else
        {
          // image cached? -> set height based on ratio
          // FIXME: bug here, this image could still being decoding
          ratio = (F32)image->x/image->y;
          height = width/ratio;
        }

        // build box
        IK_Box *box = ik_image(IK_BoxFlag_DrawBorder, ik_state->mouse_in_world, v2f32(width, height), image);
        box->ratio = ratio;
        box->disabled_t = 1.0;
        // TODO(Next): not ideal, fix it later
        box->draw_frame_index = ik_state->frame_index+1;
      }

      scratch_end(scratch);
    }

    /////////////////////////////////
    //~ Save

    if(ui_key_press(OS_Modifier_Ctrl, OS_Key_S))
    {
      ik_frame_to_tyml(frame);
      ik_message_push(push_str8f(ik_frame_arena(), "saved: %S", frame->save_path));
    }

    /////////////////////////////////
    //~ Build End

    /////////////////////////////////
    //- update last_rect_size & commit drag offset

    for(U64 i = 0; i < ArrayCount(roots); i++)
    {
      IK_Box *root = roots[i];
      for(IK_Box *box = root, *next = 0; box != 0; box = next)
      {
        next = box->prev;
        B32 is_active = ik_key_match(ik_state->active_box_key[IK_MouseButtonKind_Left], box->key);

        //- top left dragged? -> offset position by rect size delta
        B32 top_left_dragging = is_active && ik_state->action_slot == IK_ActionSlot_TopLeft;
        if(top_left_dragging)
        {
          Vec2F32 dim_delta = sub_2f32(box->last_rect_size, box->rect_size);
          ik_box_do_translate(box, dim_delta);
        }

        box->last_rect_size = box->rect_size;
      }
    }

    /////////////////////////////////
    //- hover cursor

    if(ik_tool() == IK_ToolKind_Selection)
    {
      IK_Box *hot = ik_box_from_key(ik_state->hot_box_key);
      IK_Box *active = ik_box_from_key(ik_state->active_box_key[IK_MouseButtonKind_Left]);
      IK_Box *box = active == 0 ? hot : active;
      if(box)
      {
        OS_Cursor cursor = box->hover_cursor;
        if(os_window_is_focused(ik_state->os_wnd) || active != 0)
        {
          // NOTE: ui_end_build will override cursor, so we have to override it again
          // TODO: still not ideal, find a better way
          next_cursor = cursor;
          cursor_override = 1;
        }
      }
    }

    if(ik_tool() == IK_ToolKind_Hand)
    {
      next_cursor = OS_Cursor_HandPoint;
      cursor_override = 1;
    }

    if(ik_tool() == IK_ToolKind_Text)
    {
      next_cursor = OS_Cursor_IBar;
      cursor_override = 1;
    }
  }

  /////////////////////////////////
  //~ Cook UI drawing bucket

  ui_end_build();
  DR_BucketScope(ik_state->bucket_ui)
  {
    ik_ui_draw();
  }

  // override cursor
  if(!ui_state->grab_cursor && cursor_override)
  {
    os_set_cursor(next_cursor);
  }

  /////////////////////////////////
  //~ Main scene drawing

  DR_BucketScope(ik_state->bucket_main)
  {
    /////////////////////////////////
    //- screen space drawing (background)

    dr_push_viewport(ik_state->window_dim);
    // draw a background
    {
      Rng2F32 rect = {0,0, ik_state->window_dim.x, ik_state->window_dim.y};
      // Vec4F32 clr = rgba_from_u32(0xFF0F0E0E);
      U32 src = 0x660B05FF;
      // U32 src = 0x8C1007FF;
      Vec4F32 clr = linear_from_srgba(rgba_from_u32(src));
      dr_rect_keyed(rect, clr, 0,0,0, frame->blank->key_3f32);
    }

    /////////////////////////////////
    //- world space drawing

    // unpack camera settings
    Vec2F32 viewport = dim_2f32(camera->rect);
    Mat3x3F32 xform2d = make_translate_3x3f32(negate_2f32(camera->rect.p0));

    dr_push_viewport(viewport);
    // TODO(Next): what heck? should it be column major?
    dr_push_xform2d(transpose_3x3f32(xform2d));

    // NOTE(k): since we are now using pixel-perfect object picking, the drawing order matters 
    IK_Box *roots[] = {frame->blank, frame->box_list.first, frame->select};
    if(ik_tool() != IK_ToolKind_Selection)
    {
      Swap(IK_Box*, roots[0], roots[2]);
      Swap(IK_Box*, roots[0], roots[1]);
    }

    for(U64 i = 0; i < ArrayCount(roots); i++)
    {
      IK_Box *root = roots[i];
      for(IK_Box *box = root, *next = 0; box != 0; box = next)
      {
        next = box->next;
        if(ik_state->frame_index >= box->draw_frame_index)
        {
          Rng2F32 dst = ik_rect_from_box(box);
          Vec2F32 center = center_2f32(dst);
          dst.p0 = mix_2f32(center, dst.p0, 1.0-box->disabled_t);
          dst.p1 = mix_2f32(center, dst.p1, 1.0-box->disabled_t);
          Vec2F32 dim = dim_2f32(dst);
          B32 zero_dim = dim.x == 0 && dim.y == 0;

          // draw drop_shadow
          if(box->flags & IK_BoxFlag_DrawDropShadow && !zero_dim)
          {
            Rng2F32 drop_shadow_rect = shift_2f32(pad_2f32(dst, 8), v2f32(4, 4));
            Vec4F32 drop_shadow_color = ik_rgba_from_theme_color(IK_ThemeColor_DropShadow);
            dr_rect(drop_shadow_rect, drop_shadow_color, 0.8f, 0, 8.f);
          }

          // draw rect background
          if(box->flags & IK_BoxFlag_DrawBackground && !zero_dim)
          {
            dr_rect_keyed(dst, box->background_color, 0, 0, 0, box->key_3f32);
          }
          
          // draw image
          if((box->flags&IK_BoxFlag_DrawImage) && box->image && !zero_dim)
          {
            IK_Image *image = box->image;
            if(!image->loaded)
            {
              AssertAlways(r_handle_match(image->handle, r_handle_zero()));
              // work done by decode thread
              if(!ins_atomic_u64_eval(&image->loading))
              {
                image->loaded = 1;

                // decode succeed
                if(image->decoded)
                {
                  image->handle = r_tex2d_alloc(R_ResourceKind_Static, R_Tex2DSampleKind_Linear, v2s32(image->x, image->y), R_Tex2DFormat_RGBA8, (void*)image->decoded);
                  image->decoded = 0;

                  // resize box based on image dim
                  box->ratio = (F32)image->x/image->y;
                  box->rect_size.y = box->rect_size.x/box->ratio;
                }

                // free stb artifacts
                stbi_image_free(image->decoded);
              }
            }

            if(image->loaded)
            {
              Rng2F32 src = {0,0, box->image->x, box->image->y};
              dr_img_keyed(dst, src, box->image->handle, v4f32(1,1,1,1), 0, 0, 0, box->key_3f32);
            }
          }

          // draw text
          if(box->flags & IK_BoxFlag_DrawText)
          {
            ik_draw_text(box);
          }

          // draw stroke
          if(box->flags & IK_BoxFlag_DrawStroke)
          {
            ik_draw_stroke(box);
          }

          // draw arrow
          if(box->flags & IK_BoxFlag_DrawArrow)
          {
            ik_draw_arrow(box);
          }

          // draw border
          if(box->flags & IK_BoxFlag_DrawBorder && !zero_dim)
          {
            F32 border_thickness = 1.5 * ik_state->world_to_screen_ratio.x;
            R_Rect2DInst *inst = dr_rect_keyed(pad_2f32(dst, 2*border_thickness), box->border_color, 0, border_thickness, border_thickness/2.0, box->key_3f32);
          }

          if(box->flags & IK_BoxFlag_DrawHotEffects)
          {
            F32 effective_active_t = box->active_t;
            if(!(box->flags & UI_BoxFlag_DrawActiveEffects))
            {
              effective_active_t = 0;
            }

            F32 t = box->hot_t * (1.f-effective_active_t);
            R_Rect2DInst *inst = dr_rect_keyed(dst, v4f32(0, 0, 0, 0), 0, 0, 1.f, box->key_3f32);
            Vec4F32 color = ik_rgba_from_theme_color(IK_ThemeColor_Hover);
            color.w *= t*0.1f;
            inst->colors[Corner_00] = color;
            inst->colors[Corner_01] = color;
            inst->colors[Corner_10] = color;
            inst->colors[Corner_11] = color;
            inst->colors[Corner_10].w *= t;
            inst->colors[Corner_11].w *= t;
            // MemoryCopyArray(inst->corner_radii, box->corner_radii);
          }

          // draw selection highlight
          if(box->sig.f & IK_SignalFlag_Select && !zero_dim)
          {
            dr_rect_keyed(pad_2f32(dst, 0*ik_state->world_to_screen_ratio.x), v4f32(1,1,0,0.1), 0, 0, 0, box->key_3f32);
            F32 border_thickness = 3*ik_state->world_to_screen_ratio.x;
            dr_rect_keyed(pad_2f32(dst, border_thickness*2), v4f32(0.1,0,1,1), 0, border_thickness, 0, box->key_3f32);
          }

          // draw key overlay
          if(box->flags & IK_BoxFlag_DrawKeyOverlay)
          {
            dr_rect_keyed(dst, v4f32(0,0,0,0), 0, 0, 0, box->key_3f32);
          }
        }
      }
    }

    // draw focus hot overlay
    if(ik_tool() == IK_ToolKind_Selection)
    {
      IK_Box *b = ik_box_from_key(ik_state->focus_hot_box_key[IK_MouseButtonKind_Left]);
      if(b)
      {
        Rng2F32 dst = ik_rect_from_box(b);
        dr_rect_keyed(dst, v4f32(0,0,0,0), 0, 0, 0, b->key_3f32);
      }
    }

    dr_pop_viewport();
    dr_pop_xform2d();

    /////////////////////////////////
    //- screen space drawing (overlay)

    IK_ToolKind tool = ik_tool();
    // draw tool indicator
    if(tool == IK_ToolKind_Draw)
    {
      U32 src = 0xF6FB05FF;
      Vec4F32 clr = linear_from_srgba(rgba_from_u32(src));
      clr.w = 0.8;
      Rng2F32 rect = {.p0 = ik_state->mouse, .p1 = ik_state->mouse};
      F32 stroke_size_px = (ik_top_stroke_size()/ik_state->world_to_screen_ratio.x)*0.5;
      rect = pad_2f32(rect, stroke_size_px);
      dr_rect(pad_2f32(rect, 2.0), v4f32(1,0,0,0.8), stroke_size_px+2.0, 0, 0.f);
      dr_rect(rect, clr, stroke_size_px, 0, 1.f);
    }
    dr_pop_viewport();
  }

  /////////////////////////////////
  //~ End of frame

  // end drag/drop if needed (no code handled drop)
  if(ik_state->drag_drop_state == IK_DragDropState_Dropping)
  {
    ik_state->drag_drop_state = IK_DragDropState_Null;
  }

  /////////////////////////////////
  //~ Submit work

  // NOTE(k): maybe we don't need dynamic drawlist
  // Build frame drawlist before we submit draw bucket
  // ProfScope("draw drawlist")
  // {
  //   ik_drawlist_build(ik_frame_drawlist());
  // }

  ik_state->pre_cpu_time_us = os_now_microseconds()-begin_time_us;

  // submit drawing bucket
  ProfScope("submit")
  if(os_window_is_focused(ik_state->os_wnd))
  {
    r_begin_frame();
    r_window_begin_frame(ik_state->os_wnd, ik_state->r_wnd);
    if(!dr_bucket_is_empty(ik_state->bucket_main))
    {
      dr_submit_bucket(ik_state->os_wnd, ik_state->r_wnd, ik_state->bucket_main);
    }
    if(!dr_bucket_is_empty(ik_state->bucket_ui))
    {
      if(ik_state->crt_enabled) DR_BucketScope(ik_state->bucket_ui)
      {
        dr_crt(0.25, 1.15, ik_state->time_in_seconds);
      }
      dr_submit_bucket(ik_state->os_wnd, ik_state->r_wnd, ik_state->bucket_ui);
    }
    Vec3F32 key_3f32 = r_window_end_frame(ik_state->os_wnd, ik_state->r_wnd, ik_state->mouse);
    ik_state->pixel_hot_key = ik_key_from_3f32(key_3f32);
    r_end_frame();
  }

  /////////////////////////////////
  //~ Wait if we still have some cpu time left

  U64 frame_time_target_cap_us = (U64)(1000000/target_hz);
  U64 work_us = os_now_microseconds()-begin_time_us;
  ik_state->cpu_time_us = work_us;
  if(work_us < frame_time_target_cap_us)
  {
    ProfScope("wait frame target cap")
    {
      while(work_us < frame_time_target_cap_us)
      {
        // TODO: check if os supports ms granular sleep
        B32 granular_sleep = 1;
        U64 remain_ms = (frame_time_target_cap_us-work_us)/1000;
        if(granular_sleep && remain_ms > 2000)
        {
          os_sleep_milliseconds(remain_ms-1000);
        }
        work_us = os_now_microseconds()-begin_time_us;
      }
    }
  }
  else
  {
    // Missed frame rate!
    // TODO(k): proper logging
    fprintf(stderr, "missed frame, over %06.2f ms from %06.2f ms\n", (work_us-frame_time_target_cap_us)/1000.0, frame_time_target_cap_us/1000.0);
    // TODO(k): not sure why spall why not flushing if we stopped to early, maybe there is a limit for flushing

    // tag it
    ProfBegin("MISSED FRAME");
    ProfEnd();
  }

  /////////////////////////////////
  //~ Determine frame time, record it into history

  U64 end_time_us = os_now_microseconds();
  U64 frame_time_us = end_time_us-begin_time_us;
  ik_state->frame_time_us_history[ik_state->frame_index%ArrayCount(ik_state->frame_time_us_history)] = frame_time_us;

  /////////////////////////////////
  //~ Bump frame time counters

  ik_state->frame_index++;
  ik_state->time_in_seconds += ik_state->frame_dt;
  ik_state->time_in_us += frame_time_us;

  ////////////////////////////////
  //~ Pop frame ctx

  // global stacks
  ik_pop_font();
  ik_pop_font_size();
  ik_pop_text_padding();
  ik_pop_palette();
  ik_pop_frame();

  // frame stacks
  ik_pop_background_color();
  ik_pop_stroke_size();
  ik_pop_stroke_color();
  ik_pop_font_slot();

  /////////////////////////////////
  //~ End

  scratch_end(scratch);
  ProfEnd();
  return !ik_state->window_should_close;
}

internal Arena *
ik_frame_arena()
{
  return ik_state->frame_arenas[ik_state->frame_index % ArrayCount(ik_state->frame_arenas)];
}

internal IK_DrawList *
ik_frame_drawlist()
{
  return ik_state->drawlists[ik_state->frame_index % ArrayCount(ik_state->drawlists)];
}

// editor

internal IK_ToolKind
ik_tool(void)
{
  IK_ToolKind ret = ik_state->tool;
  B32 space_is_down = os_key_is_down(OS_Key_Space);
  if(space_is_down && ik_key_match(ik_state->focus_active_box_key, ik_key_zero())) ret = IK_ToolKind_Hand;
  return ret;
}

//- colors
internal Vec4F32
ik_rgba_from_theme_color(IK_ThemeColor color)
{
  return ik_state->cfg_theme.colors[color];
}

//- code -> palette
internal IK_Palette *
ik_palette_from_code(IK_PaletteCode code)
{
  IK_Palette *ret = &ik_state->cfg_main_palettes[code];
  return ret;
}

//- code -> ui palette
internal UI_Palette *
ik_ui_palette_from_code(IK_PaletteCode code)
{
  UI_Palette *ret = &ik_state->cfg_ui_debug_palettes[code];
  return ret;
}

//- fonts/sizes
internal FNT_Tag
ik_font_from_slot(IK_FontSlot slot)
{
  return ik_state->cfg_font_tags[slot];
}

internal F32
ik_font_size_from_slot(IK_FontSlot slot)
{
  F32 result = 0;
  F32 dpi = ik_state->dpi;
  if(dpi != ik_state->last_dpi)
  {
    F32 old_dpi = ik_state->last_dpi;
    F32 new_dpi = dpi;
    ik_state->last_dpi = dpi;
    S32 *pt_sizes[] =
    {
      &ik_state->setting_vals[IK_SettingCode_MainFontSize].s32,
      &ik_state->setting_vals[IK_SettingCode_CodeFontSize].s32,
    };
    for(U64 idx = 0; idx < ArrayCount(pt_sizes); idx++)
    {
      F32 ratio = pt_sizes[idx][0] / old_dpi;
      F32 new_pt_size = ratio*new_dpi;
      pt_sizes[idx][0] = (S32)new_pt_size;
    }
  }

  switch(slot)
  {
    case IK_FontSlot_Code:
    {
      result = (F32)ik_state->setting_vals[IK_SettingCode_CodeFontSize].s32;
    }break;
    default:
    case IK_FontSlot_Main:
    case IK_FontSlot_Icons:
    {
      result = (F32)ik_state->setting_vals[IK_SettingCode_MainFontSize].s32;
    }break;
  }
  return result;
}

//- box lookup
internal IK_Box *
ik_box_from_key(IK_Key key)
{
  IK_Box *ret = 0;
  if(!ik_key_match(key, ik_key_zero()))
  {
    IK_Frame *frame = ik_top_frame();
    U64 slot_index = key.u64[0]%ArrayCount(frame->box_table);
    for(IK_Box *b = frame->box_table[slot_index].hash_first; b != 0; b = b->hash_next)
    {
      if(ik_key_match(b->key, key))
      {
        ret = b;
        break;
      }
    }
  }
  return ret;
}

//- drag data

internal Vec2F32
ik_drag_start_mouse(void)
{
  return ik_state->drag_start_mouse;
}

internal Vec2F32
ik_drag_delta(void)
{
  return sub_2f32(ik_state->mouse, ik_state->drag_start_mouse);
}

internal void
ik_store_drag_data(String8 string)
{
  arena_clear(ik_state->drag_state_arena);
  ik_state->drag_state_data = push_str8_copy(ik_state->drag_state_arena, string);
}

internal String8
ik_get_drag_data(U64 min_required_size)
{
  // NOTE: if left size is 0 and it's causing assertion failed
  //       check duplicated ui box keys
  AssertAlways(ik_state->drag_state_data.size >= min_required_size);

  // TODO: don't get it, what fuck is this
  // if(ik_state->drag_state_data.size < min_required_size)
  // {
  //     Temp scratch = scratch_begin(0, 0);
  //     String8 str = {push_array(scratch.arena, U8, min_required_size), min_required_size};
  //     ik_store_drag_data(str);
  //     scratch_end(scratch);
  // }
  return ik_state->drag_state_data;
}

//- selecting
internal inline B32
ik_is_selecting(void)
{
  Vec2F32 dim = dim_2f32(ik_state->selection_rect);
  B32 ret = ik_tool() == IK_ToolKind_Selection && ik_dragging(ik_state->active_frame->blank->sig) && (dim.x > 0 && dim.y > 0);
  return ret;
}

/////////////////////////////////
//~ OS event consumption helpers

internal B32
ik_paste(void)
{
  B32 ret = 0;
  for(UI_EventNode *n = ik_state->events->first, *next = 0; n != 0; n = next)
  {
    B32 taken = 0;
    next = n->next;
    UI_Event *evt = &n->v;

    if(evt->kind == UI_EventKind_Edit && (evt->flags&UI_EventFlag_Paste))
    {
      ret = 1;
      ui_eat_event_node(ik_state->events, n);
      break;
    }

  }
  return ret;
}

internal B32
ik_copy(void)
{
  B32 ret = 0;
  for(UI_EventNode *n = ik_state->events->first, *next = 0; n != 0; n = next)
  {
    B32 taken = 0;
    next = n->next;
    UI_Event *evt = &n->v;

    if(evt->kind == UI_EventKind_Edit && (evt->flags&UI_EventFlag_Copy))
    {
      ret = 1;
      ui_eat_event_node(ik_state->events, n);
      break;
    }

  }
  return ret;
}

internal String8List
ik_file_drop(Vec2F32 *return_mouse)
{
  String8List ret = {0};
  for(UI_EventNode *n = ik_state->events->first, *next = 0; n != 0; n = next)
  {
    B32 taken = 0;
    next = n->next;
    UI_Event *evt = &n->v;

    if(evt->kind == UI_EventKind_FileDrop)
    {
      ret = evt->paths;
      *return_mouse = evt->pos;
      ui_eat_event_node(ik_state->events, n);
      break;
    }

  }
  return ret;
}

/////////////////////////////////
//~ Dynamic drawing (in immediate mode fashion)

/////////////////////////////////
//- basic building API

internal IK_DrawList *
ik_drawlist_alloc(Arena *arena, U64 vertex_buffer_cap, U64 indice_buffer_cap)
{
  ProfBeginFunction();
  IK_DrawList *ret = push_array(arena, IK_DrawList, 1);
  ret->vertex_buffer_cap = vertex_buffer_cap;
  ret->indice_buffer_cap = indice_buffer_cap;
  ret->vertices = r_buffer_alloc(R_ResourceKind_Stream, vertex_buffer_cap, 0, 0);
  ret->indices = r_buffer_alloc(R_ResourceKind_Stream, indice_buffer_cap, 0, 0);
  ProfEnd();
  return ret;
}

internal IK_DrawNode *
ik_drawlist_push(Arena *arena, IK_DrawList *drawlist, R_Geo3D_Vertex *vertices_src, U64 vertex_count, U32 *indices_src, U64 indice_count)
{
  IK_DrawNode *ret = push_array(arena, IK_DrawNode, 1);

  U64 v_buf_size = sizeof(R_Geo3D_Vertex) * vertex_count;
  U64 i_buf_size = sizeof(U32) * indice_count;
  // TODO(XXX): we should use buffer block, we can't just release/realloc a new buffer, since the buffer handle is already handled over to IK_DrawNode
  AssertAlways(v_buf_size + drawlist->vertex_buffer_cmt <= drawlist->vertex_buffer_cap);
  AssertAlways(i_buf_size + drawlist->indice_buffer_cmt <= drawlist->indice_buffer_cap);

  U64 v_buf_offset = drawlist->vertex_buffer_cmt;
  U64 i_buf_offset = drawlist->indice_buffer_cmt;

  drawlist->vertex_buffer_cmt += v_buf_size;
  drawlist->indice_buffer_cmt += i_buf_size;

  // fill info
  ret->vertices_src = vertices_src;
  ret->vertex_count = vertex_count;
  ret->vertices = drawlist->vertices;
  ret->vertices_buffer_offset = v_buf_offset;
  ret->indices_src = indices_src;
  ret->indice_count = indice_count;
  ret->indices = drawlist->indices;
  ret->indices_buffer_offset = i_buf_offset;

  SLLQueuePush(drawlist->first_node, drawlist->last_node, ret);
  drawlist->node_count++;
  return ret;
}

internal void
ik_drawlist_build(IK_DrawList *drawlist)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(0,0);
  R_Geo3D_Vertex *vertices = (R_Geo3D_Vertex*)push_array(scratch.arena, U8, drawlist->vertex_buffer_cmt);
  U32 *indices = (U32*)push_array(scratch.arena, U8, drawlist->indice_buffer_cmt);

  // collect buffer
  R_Geo3D_Vertex *vertices_dst = vertices;
  U32 *indices_dst = indices;
  for(IK_DrawNode *n = drawlist->first_node; n != 0; n = n->next)  
  {
    MemoryCopy(vertices_dst, n->vertices_src, sizeof(R_Geo3D_Vertex)*n->vertex_count);
    MemoryCopy(indices_dst, n->indices_src, sizeof(U32)*n->indice_count);

    vertices_dst += n->vertex_count; 
    indices_dst += n->indice_count;
  }

  r_buffer_copy(drawlist->vertices, vertices, drawlist->vertex_buffer_cmt);
  r_buffer_copy(drawlist->indices, indices, drawlist->indice_buffer_cmt);
  scratch_end(scratch);
  ProfEnd();
}

internal void
ik_drawlist_reset(IK_DrawList *drawlist)
{
  // clear per-frame data
  drawlist->first_node = 0;
  drawlist->last_node = 0;
  drawlist->node_count = 0;
  drawlist->vertex_buffer_cmt = 0;
  drawlist->indice_buffer_cmt = 0;
}

/////////////////////////////////
//~ Message Functions

internal IK_Message *
ik_message_push(String8 string)
{
  // list is empty? -> reset message arena
  if(ik_state->messages.count == 0)
  {
    arena_clear(ik_state->message_arena);
  }
  
  IK_Message *msg = push_array(ik_state->message_arena, IK_Message, 1);
  msg->string = push_str8_copy(ik_state->message_arena, string);
  DLLPushBack(ik_state->messages.first, ik_state->messages.last, msg);
  ik_state->messages.count++;
  return msg;
}

/////////////////////////////////
//- high level building API 

internal IK_DrawNode *
ik_drawlist_push_rect(Arena *arena, IK_DrawList *drawlist, Rng2F32 dst, Rng2F32 src)
{
  R_Geo3D_Vertex *vertices = push_array(arena, R_Geo3D_Vertex, 4);
  U64 vertex_count = 4;
  U32 indice_count = 6;
  U32 *indices = push_array(arena, U32, 6);

  Vec4F32 color_zero = v4f32(0,0,0,0);
  // top left 0
  vertices[0].col = color_zero;
  vertices[0].nor = v3f32(0,0,-1);
  vertices[0].pos = v3f32(dst.x0, dst.y0, 0);
  vertices[0].tex = v2f32(src.x0, src.y0);

  // bottom left 1
  vertices[1].col = color_zero;
  vertices[1].nor = v3f32(0,0,-1);
  vertices[1].pos = v3f32(dst.x0, dst.y1, 0);
  vertices[1].tex = v2f32(src.x0, src.y1);

  // top right 2
  vertices[2].col = color_zero;
  vertices[2].nor = v3f32(0,0,-1);
  vertices[2].pos = v3f32(dst.x1, dst.y0, 0);
  vertices[2].tex = v2f32(src.x1, src.y0);

  // bottom right 3
  vertices[3].col = color_zero;
  vertices[3].nor = v3f32(0,0,-1);
  vertices[3].pos = v3f32(dst.x1, dst.y1, 0);
  vertices[3].tex = v2f32(src.x1, src.y1);

  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 2;

  indices[3] = 1;
  indices[4] = 3;
  indices[5] = 2;

  IK_DrawNode *ret = ik_drawlist_push(arena, drawlist, vertices, vertex_count, indices, indice_count);
  ret->topology = R_GeoTopologyKind_Triangles;
  ret->polygon = R_GeoPolygonKind_Fill;
  return ret;
}

/////////////////////////////////
//~ String Block Functions

internal String8
ik_push_str8_copy(String8 src)
{
  String8 ret = ik_str8_new(src.size+1); // NOTE: add 1 for null terminator
  MemoryCopy(ret.str, src.str, src.size);
  ret.size = src.size;
  ret.str[ret.size] = '\0';
  return ret;
}

internal String8
ik_str8_new(U64 size)
{
  ProfBeginFunction();
#define MIN_CAP 512
  String8 ret = {0};
  IK_Frame *frame = ik_top_frame();

  U64 required_bytes = Max(MIN_CAP, size);
  IK_StringBlock *block = 0;
  for(IK_StringBlock *b = frame->first_free_string_block; b != 0; b = b->free_next)
  {
    S64 remain_bytes = (S64)b->cap_bytes - (S64)required_bytes;
    F32 tolerance_pct = 0.25;
    B32 fit = remain_bytes > 0 && remain_bytes < (b->cap_bytes*tolerance_pct);
    if(fit)
    {
      block = b;
      DLLRemove_NP(frame->first_free_string_block, frame->last_free_string_block, b, free_next, free_prev);
      break;
    }
  }

  // didn't find a block -> alloc a new block
  if(block == 0)
  {
    block = push_array(frame->arena, IK_StringBlock, 1);
    // TODO: we can get away with no-zero
    block->p = push_array_fat_sized(frame->arena, required_bytes, block);
    block->cap_bytes = required_bytes;
  }

  ret.str = block->p;
  ret.size = size;
  ProfEnd();
  return ret;
#undef MIN_CAP
}

internal void
ik_string_block_release(String8 string)
{
  IK_Frame *frame = ik_top_frame();
  IK_StringBlock *block = ptr_from_fat(string.str);
  DLLPushFront_NP(frame->first_free_string_block, frame->last_free_string_block, block, free_next, free_prev);
}

/////////////////////////////////
//~ Box Type Functions

// internal IK_BoxRec
// ik_box_rec_df(IK_Box *box, IK_Box *root, U64 sib_member_off, U64 child_member_off)
// {
//   // depth first search starting from the current box 
//   IK_BoxRec result = {0};
//   result.next = 0;
//   if((*MemberFromOffset(IK_Box **, box, child_member_off)) != 0)
//   {
//     result.next = *MemberFromOffset(IK_Box **, box, child_member_off);
//     result.push_count = 1;
//   }
//   else for(IK_Box *p = box; p != 0 && p != root; p = p->parent)
//   {
//     if((*MemberFromOffset(IK_Box **, p, sib_member_off)) != 0)
//     {
//       result.next = *MemberFromOffset(IK_Box **, p, sib_member_off);
//       break;
//     }
//     result.pop_count += 1;
//   }
//   return result;
// }

/////////////////////////////////
//~ Frame Building API

internal IK_Frame *
ik_frame_alloc()
{
  IK_Frame *frame = ik_state->first_free_frame;
  Temp scratch = scratch_begin(0,0);
  if(frame)
  {
    SLLStackPop(ik_state->first_free_frame);
    Arena *arena = frame->arena;
    U64 arena_clear_pos = frame->arena_clear_pos;
    arena_pop_to(arena, arena_clear_pos);
    MemoryZeroStruct(frame);
    frame->arena = arena;
    frame->arena_clear_pos = arena_clear_pos;
  }
  else
  {
    Arena *arena = arena_alloc();
    frame = push_array(arena, IK_Frame, 1);
    frame->arena = arena;
    frame->arena_clear_pos = arena_pos(arena);
  }

  Arena *arena = frame->arena;

  // alloc action ring buffer
  frame->action_ring.slot_count = 1024;
  frame->action_ring.slots = push_array(arena, IK_Action, frame->action_ring.slot_count);

  /////////////////////////////////
  //~ Fill default settings

  String8 binary_path = os_get_process_info()->binary_path; // only directory
  // TODO(Next): may use proper filepath join
  String8 save_path = push_str8f(scratch.arena, "%S/default.tyml", binary_path);
  frame->save_path = push_str8_copy_static(save_path, frame->_save_path);
  frame->cfg = ik_cfg_default();

  // camera
  frame->camera.rect = ik_state->window_rect;
  frame->camera.target_rect = frame->camera.rect;
  frame->camera.zn = -0.1;
  frame->camera.zf = 1000000.0;
  frame->camera.anim_rate = ik_state->animation.fast_rate;
  frame->camera.min_zoom_step = 0.05;
  frame->camera.max_zoom_step = 0.35;

  // create blank box
  ik_push_frame(frame);
  // frame stacks
  ik_push_background_color(frame->cfg.background_color);
  ik_push_stroke_size(frame->cfg.stroke_size);
  ik_push_stroke_color(frame->cfg.stroke_color);
  frame->blank = ik_build_box_from_stringf(IK_BoxFlag_MouseClickable|
                                           IK_BoxFlag_FitViewport|
                                           IK_BoxFlag_DrawKeyOverlay|
                                           IK_BoxFlag_Orphan|
                                           IK_BoxFlag_OmitGroupSelection|
                                           IK_BoxFlag_OmitDeletion,
                                           "blank");
  frame->select = ik_build_box_from_stringf(IK_BoxFlag_MouseClickable|
                                            IK_BoxFlag_ClickToFocus|
                                            IK_BoxFlag_Orphan|
                                            IK_BoxFlag_DrawBorder|
                                            IK_BoxFlag_DrawBackground|
                                            IK_BoxFlag_DrawDropShadow|
                                            IK_BoxFlag_DragToPosition|
                                            IK_BoxFlag_OmitGroupSelection|
                                            IK_BoxFlag_OmitDeletion|
                                            IK_BoxFlag_FixedRatio|
                                            IK_BoxFlag_FitChildren,
                                            "select");
  frame->select->hover_cursor = OS_Cursor_UpDownLeftRight;
  frame->select->background_color.w = 0.05;

  ik_pop_frame();
  ik_pop_background_color();
  ik_pop_stroke_size();
  ik_pop_stroke_color();
  scratch_end(scratch);
  return frame;
}

internal void
ik_frame_release(IK_Frame *frame)
{
  SLLStackPush(ik_state->first_free_frame, frame);
}

/////////////////////////////////
//~ Cfg Functions

internal IK_Cfg
ik_cfg_default()
{
  IK_Cfg cfg = {0};
  // all in srgb space
  cfg.stroke_size = ik_bottom_stroke_size();
  cfg.stroke_color = ik_bottom_stroke_color();
  cfg.background_color = ik_bottom_background_color();
  cfg.text_font_slot = IK_FontSlot_HandWrite2;
  return cfg;
}

/////////////////////////////////
//~ Box Tree Building API

/////////////////////////////////
//- box node construction

internal IK_Box *
ik_build_box_from_key_(IK_BoxFlags flags, IK_Key key, B32 pre_order)
{
  ProfBeginFunction();

  IK_Frame *frame = ik_top_frame();
  IK_Box *box = frame->first_free_box;
  if(box != 0)
  {
    SLLStackPop(frame->first_free_box);
  }
  else
  {
    box = push_array_no_zero(frame->arena, IK_Box, 1);
  }
  MemoryZeroStruct(box);

  IK_Palette *palette = ik_top_palette();
  // fill box info
  box->key = key;
  box->key_3f32 = ik_2f32_from_key(key);
  box->flags = flags;
  box->frame = frame;
  box->font_size = ik_top_font_size();
  box->font_slot = ik_top_font_slot();
  box->tab_size = ik_top_tab_size();
  box->text_raster_flags = ik_top_text_raster_flags();
  box->stroke_size = ik_top_stroke_size();
  box->stroke_color = ik_top_stroke_color();
  box->background_color = ik_top_background_color();
  box->text_color = palette->text;
  box->border_color = palette->border;
  box->transparency = ik_top_transparency();
  box->text_padding = ik_top_text_padding();
  box->hover_cursor = ik_top_hover_cursor();

  // hook into lookup table
  U64 slot_index = key.u64[0]%ArrayCount(frame->box_table);
  IK_BoxHashSlot *slot = &frame->box_table[slot_index];
  DLLInsert_NPZ(0, slot->hash_first, slot->hash_last, slot->hash_last, box, hash_next, hash_prev);

  // push into top box list
  if(!(box->flags&IK_BoxFlag_Orphan))
  {
    IK_BoxList *list = &frame->box_list;
    if(pre_order)
    {
      DLLPushBack(list->first, list->last, box);
    }
    else
    {
      DLLPushFront(list->first, list->last, box);
    }
    list->count++;
  }

  ProfEnd();
  return box;
}

internal IK_Box *
ik_build_box_from_string(IK_BoxFlags flags, String8 string)
{
  ProfBeginFunction();
  IK_Key key = ik_key_from_string(ik_key_zero(), string);
  IK_Box *box = ik_build_box_from_key(flags, key);

  String8 name = ik_display_part_from_key_string(string);
  ik_box_equip_name(box, name);

  ProfEnd();
  return box;
}

internal IK_Box *
ik_build_box_from_stringf(IK_BoxFlags flags, char *fmt, ...)
{
  Temp scratch = scratch_begin(0,0);
  va_list args;
  va_start(args, fmt);
  String8 string = push_str8fv(scratch.arena, fmt, args);
  va_end(args);
  IK_Box *box = ik_build_box_from_string(flags, string);
  scratch_end(scratch);
  return box;
}

//- box node destruction
internal void
ik_box_release(IK_Box *box)
{ 
  IK_Frame *frame = box->frame;
  IK_Box *group = box->group;

  // recursively remove group children first
  for(IK_Box *c = box->group_first, *next = 0; c != 0; c = next)
  {
    next = c->group_next;
    ik_box_release(c);
  }

  if(group)
  {
    DLLRemove(group->group_first, group->group_last, box);
    group->group_children_count--;
  }

  /////////////////////////////////
  //~ Remove equipment buffers

  //- string block
  if(box->string.size > 0)
  {
    ik_string_block_release(box->string);
  }

  //- image
  if(box->image)
  {
    box->image->rc--;
    // TODO(Next): if image rc is 0, we may want to release it
  }

  //- points
  if(box->first_point)
  {
    for(IK_Point *p = box->first_point, *next = 0; p != 0; p = next)
    {
      next = p->next;
      ik_point_release(p);
    }
  }

  // remove from box list
  IK_BoxList *list = &frame->box_list;
  DLLRemove(list->first, list->last, box);
  list->count--;

  // remove from lookup table
  U64 slot_index = box->key.u64[0]%ArrayCount(frame->box_table);
  IK_BoxHashSlot *slot = &frame->box_table[slot_index];
  DLLRemove_NP(slot->hash_first, slot->hash_last, box, hash_next, hash_prev);

  // push to free list stack
  SLLStackPush(frame->first_free_box, box);
}

/////////////////////////////////
//- box node equipment


internal void
ik_box_equip_name(IK_Box *box, String8 name)
{
  String8 string = push_str8_copy_static(name, box->_name);
  box->name = string;
}

internal String8
ik_box_equip_display_string(IK_Box *box, String8 display_string)
{
  String8 ret = {0};
  if(box->string.size != 0)
  {
    ik_string_block_release(box->string);
  }
  box->string = ik_push_str8_copy(display_string);
  box->flags |= IK_BoxFlag_HasDisplayString;
  return ret;
}

/////////////////////////////////
//~ High Level Box Building

typedef struct IK_EditBoxTextRect IK_EditBoxTextRect;
struct IK_EditBoxTextRect
{
  Rng2F32 parent_rect;
  Rng2F32 dst;
  Rng2F32 src;
  R_Handle tex;
  Vec4F32 color;
  B32 highlight;
};

typedef struct IK_EditBoxDrawData IK_EditBoxDrawData;
struct IK_EditBoxDrawData
{
  Rng2F32 mark_rect;
  Rng2F32 cursor_rect;
  IK_EditBoxTextRect *text_rects;
};

IK_BOX_UPDATE(text)
{
  B32 is_focus_hot = ik_key_match(box->key, ik_state->focus_hot_box_key[IK_MouseButtonKind_Left]);
  B32 is_focus_active = ik_key_match(box->key, ik_state->focus_active_box_key);

  // data buffer
  IK_EditBoxDrawData *draw_data = push_array(ik_frame_arena(), IK_EditBoxDrawData, 1);
  box->draw_data = draw_data;

  TxtPt *cursor = &box->cursor;
  TxtPt *mark = &box->mark;

  ////////////////////////////////
  // Take navigation actions for editing

  if(is_focus_active)
  {
    Temp scratch = scratch_begin(0,0);
    UI_EventList *events = ik_state->events;
    for(UI_EventNode *n = events->first; n != 0; n = n->next)
    {
      UI_Event *evt = &n->v;

      if(evt->key == OS_Key_Return && evt->kind == UI_EventKind_Press)
      {
        evt->kind = UI_EventKind_Text;
        evt->string = str8_lit("\n");
      }

      // don't consume anything that doesn't fit a single|multi-line's operations
      if((evt->kind != UI_EventKind_Edit && evt->kind != UI_EventKind_Navigate && evt->kind != UI_EventKind_Text)) { continue; }

      // map this action to an TxtOp
      UI_TxtOp op = ik_multi_line_txt_op_from_event(scratch.arena, &n->v, box->string, *cursor, *mark);

      // perform replace range
      if(!txt_pt_match(op.range.min, op.range.max) || op.replace.size != 0)
      {
        String8 new_string = ui_push_string_replace_range(scratch.arena, box->string, r1s64(op.range.min.column, op.range.max.column), op.replace);
        ik_box_equip_display_string(box, new_string);
      }

      if((op.flags&UI_TxtOpFlag_Copy) && op.copy.size != 0)
      {
        os_set_clipboard_text(op.copy);
        ik_message_push(str8_lit("copied"));
      }

      // commit op's changed cursor & mark to caller-provided state
      *cursor = op.cursor;
      *mark = op.mark;

      // consume event
      ui_eat_event(&n->v);
    }
    scratch_end(scratch);
  }

  // unpack box rect
  Rng2F32 box_rect = ik_rect_from_box(box);
  Vec2F32 box_dim = dim_2f32(box_rect);

  // unpack font params
  F32 font_size = box->font_size;
  F32 font_size_scale = font_size / (F32)M_1;
  F32 text_padding_x = box->text_padding*font_size_scale;
  F32 max_x = box_dim.x - text_padding_x*2;
  F32 max_y = box_dim.y;

  ////////////////////////////////
  //~ Push line runs

  Vec2F32 total_text_dim = {0,0};
  Vec2F32 empty_text_dim = {0,0};
  {
    Temp scratch = scratch_begin(0,0);

    // push line runs
    char *by = "\n";
    String8List lines = str8_split(ik_frame_arena(), box->string, (U8*)by, 1, StringSplitFlag_KeepEmpties);
    DR_FRunList *line_fruns = 0;

    // unpack font params
    FNT_Tag font = ik_font_from_slot(box->font_slot);
    FNT_Metrics font_metrics = fnt_metrics_from_tag_size(font, M_1);

    U64 line_index = 0;
    for(String8Node *n = lines.first; n != 0; n = n->next, line_index++)
    {
      String8 string = n->string;
      for(;;)
      {
        DR_FStrList fstrs = {0};
        DR_FStr fstr = {0};
        fstr.string = string;
        fstr.params.font = font;
        fstr.params.color = box->text_color;
        fstr.params.size = M_1;
        fstr.params.underline_thickness = 0;
        fstr.params.strikethrough_thickness = 0;
        dr_fstrs_push(scratch.arena, &fstrs, &fstr);

        // TODO(Next): support different fstr
        DR_FRunList fruns = dr_fruns_from_fstrs(ik_frame_arena(), box->tab_size, &fstrs);

        // NOTE(k): for empty line, the dim are all zero, we need to ClampBot dim_y to line height
        if(string.size == 0)
        {
          fruns.first->v.run.dim.y = font_metrics.ascent+font_metrics.descent;
          fruns.dim.y = fruns.first->v.run.dim.y;
        }

        U64 cutoff = 0;
        U64 cutoff_size = 0;
        Vec2F32 run_dim_px = {fruns.dim.x*font_size_scale, fruns.dim.y*font_size_scale};
        // text_wrapping
        if((box->flags&IK_BoxFlag_WrapText) && run_dim_px.x > max_x)
        {
          FNT_PieceArray pieces = fruns.first->v.run.pieces;
          FNT_Piece *piece_first = pieces.v;
          FNT_Piece *piece_opl = piece_first+pieces.count;
          F32 cutoff_x = 0;
          for(FNT_Piece *piece = piece_opl-1;
              piece > piece_first && run_dim_px.x > max_x;
              piece--)
          {
            cutoff++;
            cutoff_size += piece->decode_size;
            cutoff_x += piece->advance;
            run_dim_px.x -= piece->advance*font_size_scale;
          }

          // commit new run dim and piece count
          fruns.first->v.run.pieces.count -= cutoff;
          fruns.dim.x -= cutoff_x;
          fruns.first->v.run.dim.x -= cutoff_x;
        }

        // has next line? -> push a line break piece
        if(cutoff == 0 && n->next != 0)
        {
          FNT_Piece *pieces = fruns.first->v.run.pieces.v;
          U64 piece_count = fruns.first->v.run.pieces.count;

          U64 next_piece_count = piece_count+1;
          FNT_Piece *next_pieces = push_array(ik_frame_arena(), FNT_Piece, next_piece_count);
          MemoryCopy(next_pieces, pieces, sizeof(FNT_Piece)*piece_count);
          next_pieces[next_piece_count-1].decode_size = 1;

          fruns.first->v.run.pieces.v = next_pieces;
          fruns.first->v.run.pieces.count = next_piece_count;
        }

        // skip to cutoff
        string.str += (string.size-cutoff_size);
        string.size = cutoff_size;

        darray_push(ik_frame_arena(), line_fruns, fruns);
        total_text_dim.x = Max(total_text_dim.x, fruns.dim.x);
        total_text_dim.y += fruns.dim.y;

        if(cutoff == 0) break;
      }
    }

    // push a empty run
    {
      DR_FStrList fstrs = {0};

      DR_FStr fstr = {0};
      fstr.string = str8_lit("  ");
      fstr.params.font = ik_font_from_slot(box->font_slot);
      fstr.params.color = box->text_color;
      fstr.params.size = M_1;
      fstr.params.underline_thickness = 0;
      fstr.params.strikethrough_thickness = 0;
      dr_fstrs_push(scratch.arena, &fstrs, &fstr);
      box->empty_fruns = dr_fruns_from_fstrs(ik_frame_arena(), box->tab_size, &fstrs);
      empty_text_dim = box->empty_fruns.dim;
    }

    // fill
    box->display_lines = lines;
    box->display_line_fruns = line_fruns;
    scratch_end(scratch);
  }

  ////////////////////////////////
  // rendering

  TxtPt mouse_pt = {1, 1};
  Rng2F32 mark_rect = {0};
  Rng2F32 cursor_rect = {0};
  Vec2F32 text_bounds = {0};
  {
    F32 best_mouse_offset = inf32();

    F32 advance_x = 0;
    F32 advance_y = 0;
    // FIXME: box rect is always one frame behind, so the text rendered will be one frame off
    //        we could use relative pos, then push a xform2d, too much work for now
    F32 x = box_rect.p0.x + text_padding_x;
    F32 y = box_rect.p0.y;
    IK_TextAlign text_align = box->text_align;

    // unpack lines
    DR_FRunList *line_fruns = box->string.size > 0 ? box->display_line_fruns : &box->empty_fruns;
    U64 line_count = box->string.size > 0 ? darray_size(box->display_line_fruns) : 1;
    Vec2F32 text_dim = box->string.size > 0 ? total_text_dim : empty_text_dim;
    text_dim.x *= font_size_scale;
    text_dim.y *= font_size_scale;

    // vertical align
    if(text_align&IK_TextAlign_VCenter)
    {
      y += (max_y-text_dim.y)/2.0;
      y = ClampBot(box_rect.p0.y, y);
    }
    if(text_align&IK_TextAlign_Bottom)
    {
      y += (max_y-text_dim.y);
    }

    // unpack mark & cursor
    mark_rect = (Rng2F32){x, y, x, box_rect.y1};
    cursor_rect = (Rng2F32){x, y, x, box_rect.y1};
    TxtRng selection_rng = is_focus_active ? txt_rng(*cursor, *mark) : (TxtRng){0};

    U64 c_index = 0;
    for(U64 line_index = 0; line_index < line_count; line_index++)
    {
      // one line
      DR_FRunList *fruns = &line_fruns[line_index];
      for(DR_FRunNode *n = fruns->first; n != 0; n = n->next)
      {
        DR_FRun run = n->v;
        Vec2F32 line_run_dim = scale_2f32(run.run.dim, font_size_scale);
        text_bounds.x = Max(text_bounds.x, line_run_dim.x+text_padding_x*2);
        text_bounds.y += line_run_dim.y;

        F32 line_x = x;
        F32 line_y = y + run.run.ascent*font_size_scale;;

        // horizontal align
        if(text_align&IK_TextAlign_HCenter)
        {
          line_x += (max_x-line_run_dim.x)/2.0;
          line_x = ClampBot(x, line_x);
        }
        if(text_align&IK_TextAlign_Right)
        {
          line_x += (max_x-line_run_dim.x);
          line_x = ClampBot(x, line_x);
        }

        B32 mouse_in_line_bounds = ik_state->mouse_in_world.y > y && ik_state->mouse_in_world.y < (y+line_run_dim.y);
        Rng2F32 line_cursor = {line_x,y,line_x,y+line_run_dim.y};

        FNT_Piece *piece_first = run.run.pieces.v;
        FNT_Piece *piece_opl = run.run.pieces.v + run.run.pieces.count;
        for(FNT_Piece *piece = piece_first; piece < piece_opl; piece++)
        {
          F32 this_advance_x = piece->advance*font_size_scale;
          Rng2F32 src = r2f32p((F32)piece->subrect.x0, (F32)piece->subrect.y0, (F32)piece->subrect.x1, (F32)piece->subrect.y1);
          Vec2F32 size = dim_2f32(src);
          Rng2F32 dst = r2f32p(piece->offset.x*font_size_scale + line_x + advance_x,
                               piece->offset.y*font_size_scale + line_y,
                               (piece->offset.x+size.x)*font_size_scale + line_x + advance_x,
                               (piece->offset.y+size.y)*font_size_scale + line_y);

          // issue draw
          B32 highlight = (c_index+1) >= selection_rng.min.column && (c_index+1) < selection_rng.max.column;
          Rng2F32 parent_rect = {line_x+advance_x, y, line_x+advance_x+piece->advance*font_size_scale, y+line_run_dim.y};
          IK_EditBoxTextRect text_rect = {parent_rect, dst, src, piece->texture, run.color, highlight};
          darray_push(ik_frame_arena(), draw_data->text_rects, text_rect);

          // cursor/mark on this glyph, set mark/cursor rect
          if(mark && mark->column == (c_index+1))
            mark_rect = line_cursor;
          if(cursor && cursor->column == (c_index+1))
            cursor_rect = line_cursor;

          // update mouse_pt if it's closer
          // Note(k): this won't work on empty line
          if(mouse_in_line_bounds)
          {
            F32 dist_to_mouse = length_2f32(sub_2f32(ik_state->mouse_in_world, center_2f32(parent_rect)));
            if(dist_to_mouse < best_mouse_offset)
            {
              // TODO(k): if we are handling unicode, we want the utf-8 decode_size too
              best_mouse_offset = dist_to_mouse;
              mouse_pt.column = abs_f32(ik_state->mouse_in_world.x-dst.x0) <= abs_f32(ik_state->mouse_in_world.x-dst.x0-this_advance_x) ? c_index+1 : c_index+piece->decode_size+1;
            }
          }

          advance_x += this_advance_x;
          line_cursor.x1 += this_advance_x;
          line_cursor.x0 += this_advance_x;

          // advance c_index
          c_index += piece->decode_size;
        }

        // NOTE(k): cursor or mark is end of line
        if(mark && mark->column == (c_index+1))     mark_rect = line_cursor;
        if(cursor && cursor->column == (c_index+1)) cursor_rect = line_cursor;

        // mouse is on empty line => mark mouse_pt (since we don't have any pieces to snap mouse)
        if(mouse_in_line_bounds && run.run.pieces.count == 0)
        {
          mouse_pt.column = c_index+1;
        }

        advance_x = 0;
        // TODO(Next): font metrics has line_gap, we should be considering that
        advance_y += line_run_dim.y;
        y += line_run_dim.y;
      }
    }

    // TODO(Next): fix it
    if(is_focus_active)
    {
      draw_data->mark_rect = mark_rect;
      draw_data->cursor_rect = cursor_rect;
    }
  }

  if(is_focus_active || box->string.size > 0) box->text_bounds = text_bounds;

  ////////////////////////////////
  // interaction

  IK_Signal sig = box->sig;

  // not focus active => double clicked or keyboard
  if(!is_focus_active && sig.f&(IK_SignalFlag_DoubleClicked|IK_SignalFlag_KeyboardPressed))
  {
    ik_state->focus_hot_box_key[IK_MouseButtonKind_Left] = box->key;
    ik_state->focus_active_box_key = box->key;

    // select all text
    mark->line = 1;
    mark->column = 1;
    cursor->line = 1;
    cursor->column = box->string.size+1;
    ik_kill_action();
  }

  // dragging => move cursor
  if(is_focus_active && ik_dragging(sig) && ik_state->action_slot == IK_ActionSlot_Null)
  {
    if(ik_pressed(sig))
    {
      *mark = mouse_pt;
    }
    *cursor = mouse_pt;
  }

  // focus active? -> set ime position to mouse pt
  if(is_focus_active)
  {
    Vec2F32 pos = {cursor_rect.x0, cursor_rect.y1};
    pos = ik_screen_pos_from_world(pos);
    pos.x = Clamp(0, pos.x, ik_state->window_dim.x);
    pos.y = Clamp(0, pos.y, ik_state->window_dim.y);
    os_set_ime_position(ik_state->os_wnd, v2s32(pos.x, pos.y));
  }

  // focus active and double clicked? -> select current word
  if(is_focus_active && sig.f&IK_SignalFlag_DoubleClicked && box->string.size > 0)
  {
    String8 string = box->string;
    TxtPt cursor = box->cursor;
    TxtPt mark = box->mark;

    char *first = (char*)string.str;
    char *opl = (char*)(string.str+string.size);
    char *curr = (char*)(string.str+cursor.column)-1;
    char *c = curr;

    if(char_is_space(*c))
    {
      // find next non-empty char
      for(; c < opl && char_is_space(*c); c++);
      // update cursor
      cursor.column = c-first;

      // reset c
      c = curr;

      // find prev non-empty char
      for(; c > first && char_is_space(*(c-1)); c--);
      mark.column = c-first+1;

      // update mark
    }
    else
    {
      // find next empty space
      for(; c < opl && !char_is_space(*c); c++);
      // update cursor
      cursor.column = c-first+1;

      // reset c
      c = curr;

      // find prev empty space
      for(; c > first && !char_is_space(*(c-1)); c--);
      mark.column = c-first+1;
    }

    ik_kill_action();
    // commit new cursor&mark
    box->cursor = cursor;
    box->mark = mark;
  }

  // focus active and triple clicked? -> select current line
  if(is_focus_active && sig.f&IK_SignalFlag_TripleClicked && box->string.size > 0)
  {
    String8 string = box->string;
    TxtPt cursor = box->cursor;
    TxtPt mark = box->mark;

    char *first = (char*)string.str;
    char *opl = (char*)(string.str+string.size);
    char *curr = (char*)(string.str+cursor.column)-1;
    char *c = curr;

    // find next linebreak
    for(; c < opl && *c != '\n'; c++);
    cursor.column = c-first+1;

    // reset c
    c = curr;

    // find prev linebreak
    for(; c > first && *(c-1) != '\n'; c--);
    mark.column = c-first+1;

    ik_kill_action();
    // commit  new cursor&mark
    box->cursor = cursor;
    box->mark = mark;
  }

  box->hover_cursor = is_focus_active ? OS_Cursor_IBar : OS_Cursor_UpDownLeftRight;
}

IK_BOX_DRAW(text)
{
  IK_EditBoxDrawData *draw_data = box->draw_data;

  // draw text rects
  for(U64 i = 0; i < darray_size(draw_data->text_rects); i++)
  {
    IK_EditBoxTextRect *text_rect = &draw_data->text_rects[i];
    // NOTE(k): we have \n run which is not renderable, and it will cutoff grouping continuation 
    if(!r_handle_match(r_handle_zero(), text_rect->tex))
    {
      dr_img_keyed(text_rect->dst, text_rect->src, text_rect->tex, text_rect->color, 0,0,0, box->key_3f32);
    }
    // dr_rect(text_rect->dst, v4f32(1,0,0,0.5), 0,1,0);

    if(text_rect->highlight)
    {
      dr_rect(text_rect->parent_rect, v4f32(0,0,1,0.2), 0, 0, 0);
    }
  }

  // TODO(Next): fix this
  // draw mark & cursor
  F32 line_height = draw_data->cursor_rect.y1-draw_data->cursor_rect.y0;
  F32 cursor_thickness = line_height*0.05;
  Rng2F32 cursor_rect = draw_data->cursor_rect;
  Rng2F32 mark_rect = draw_data->mark_rect;
  cursor_rect.x0 -= cursor_thickness;
  cursor_rect.x1 += cursor_thickness;
  mark_rect.x0 -= cursor_thickness;
  mark_rect.x1 += cursor_thickness;
  dr_rect(cursor_rect, v4f32(0,0,1,0.5), cursor_thickness, 0, 1.0*ik_state->world_to_screen_ratio.x);
  dr_rect(mark_rect, v4f32(0,1,0,0.5), cursor_thickness, 0, 1.0*ik_state->world_to_screen_ratio.x);
}

internal IK_Box *
ik_text(String8 string, Vec2F32 pos)
{
  IK_Box *box;

  F32 font_size = ik_top_font_size();
  F32 font_size_in_world = font_size * ik_state->world_to_screen_ratio.x * 2;

  box = ik_build_box_from_stringf(0, "text###%I64u", os_now_microseconds());
  box->flags |= IK_BoxFlag_MouseClickable|
                IK_BoxFlag_ClickToFocus|
                IK_BoxFlag_DrawText|
                IK_BoxFlag_DragToScaleFontSize|
                IK_BoxFlag_PruneZeroText|
                IK_BoxFlag_FixedRatio|
                IK_BoxFlag_FitText|
                IK_BoxFlag_DragToPosition;
  box->position = pos;
  box->rect_size = v2f32(font_size_in_world*3, font_size_in_world);
  box->last_rect_size = box->rect_size;
  box->hover_cursor = OS_Cursor_UpDownLeftRight;
  box->font_size = font_size_in_world;
  box->ratio = 0;
  box->disabled_t = 1.0;
  box->cursor = txt_pt(1, string.size+1);
  box->mark = box->cursor;

  if(string.size > 0)
  {
    ik_box_equip_display_string(box, string);
  }
  return box;
}

internal IK_Box *
ik_image(IK_BoxFlags flags, Vec2F32 pos, Vec2F32 rect_size, IK_Image *image)
{
  IK_Box *box = ik_build_box_from_stringf(0, "image###%I64u", os_now_microseconds());
  box->flags = flags|IK_BoxFlag_DrawImage|IK_BoxFlag_MouseClickable|IK_BoxFlag_ClickToFocus|IK_BoxFlag_FixedRatio|IK_BoxFlag_DragToPosition|IK_BoxFlag_DragToScaleRectSize|IK_BoxFlag_DoubleClickToCenter;
  box->position = pos;
  box->rect_size = rect_size;
  box->last_rect_size = rect_size;
  box->hover_cursor = OS_Cursor_UpDownLeftRight;
  box->image = image;
  image->rc++;
  return box;
}

IK_BOX_UPDATE(stroke)
{
  IK_Signal sig = box->sig;
  B32 is_focus_active = ik_key_match(ik_state->focus_active_box_key, box->key);
  if(is_focus_active)
  {
    for(UI_EventNode *n = ik_state->events->first, *next = 0; n != 0; n = next)
    {
      B32 taken = 0;
      next = n->next;
      UI_Event *evt = &n->v;

      if(evt->kind == UI_EventKind_Release && evt->key == OS_Key_LeftMouseButton)
      {
        // ik_state->focus_hot_box_key = ik_key_zero();
        is_focus_active = 0;
        ik_state->focus_active_box_key = ik_key_zero();

        // commited -> calculate bounds
        {
          F32 half_stroke_size = box->stroke_size/2.0;
          Rng2F32 bounds = {inf32(), inf32(), neg_inf32(), neg_inf32()};
          for(IK_Point *p = box->first_point; p != 0; p = p->next)
          {
            Vec2F32 pos = p->position;
            Rng2F32 rect = {pos.x-half_stroke_size, pos.y-half_stroke_size, pos.x+half_stroke_size, pos.y+half_stroke_size};

            // update bounds
            bounds.x0 = Min(rect.x0, bounds.x0);
            bounds.x1 = Max(rect.x1, bounds.x1);
            bounds.y0 = Min(rect.y0, bounds.y0);
            bounds.y1 = Max(rect.y1, bounds.y1);
          }
          Vec2F32 bounds_dim = dim_2f32(bounds);
          box->position = bounds.p0;
          box->rect_size = bounds_dim;
          box->ratio = bounds_dim.x/bounds_dim.y;
        }
      }

      if(taken)
      {
        ui_eat_event_node(ik_state->events, n);
      }
    }
  }
  if(is_focus_active)
  {
    Vec2F32 last_position = box->last_point ? box->last_point->position : v2f32(-1,-1);
    Vec2F32 next_position = ik_state->mouse_in_world;
    if(box->point_count < 3 || (last_position.x != next_position.x || last_position.y != next_position.y))
    {
      // capture current point
      {
        IK_Point *p = ik_point_alloc();
        DLLPushBack(box->first_point, box->last_point, p);
        p->position = next_position;
        box->point_count++;
      }

      // makeup to 3 points, we need at least 3 points to draw a bezer curve
      if(box->point_count < 3)
      {
        U64 makeup = 3-box->point_count;
        for(U64 i = 0; i < makeup; i++)
        {
          IK_Point *p = ik_point_alloc();
          DLLPushBack(box->first_point, box->last_point, p);
          p->position = next_position;
          box->point_count++;
        }
      }
      // purge redudant points
      else
      {
        // e.g. points basically on a line, we can just save two endpoint in this case
        IK_Point *p2 = box->last_point;
        IK_Point *p1 = p2->prev;
        IK_Point *p0 = p1 ? p1->prev : 0;
        if(p0)
        {
          // area formed by p0p1 and p0p2, we don't need to divide it by 2, since we only compare with epsilon
          F32 epsilon = 1e-2;
          Vec2F32 p0p1 = sub_2f32(p1->position, p0->position);
          Vec2F32 p0p2 = sub_2f32(p2->position, p0->position);
          F32 area = abs_f32(p0p1.x*p0p2.y - p0p1.y*p0p2.x); // cross product
          if(area < epsilon)
          {
            DLLRemove(box->first_point, box->last_point, p1);
            box->point_count--;
            ik_point_release(p1);
          }
        }
      }
    }
  }
}

IK_BOX_DRAW(stroke)
{
  F32 base_stroke_size = box->stroke_size;
  F32 min_visiable_stroke_size = (2*ik_state->dpi/96.f) * ik_state->world_to_screen_ratio.x;

  IK_Point *p0 = box->first_point;
  IK_Point *p1 = p0 ? p0->next : 0;
  IK_Point *p2 = p1 ? p1->next : 0;

  Vec4F32 stroke_color = linear_from_srgba(box->stroke_color);
  F32 edge_softness = 1.f * ik_state->world_to_screen_ratio.x;

  F32 last_scale = 1.0;
  F32 last_point_drawn = 0;
  while(p2)
  {
    // compute midpoints p0 -> p1 -> p2
    Vec2F32 m1 = {(p0->position.x + p1->position.x) * 0.5f, (p0->position.y + p1->position.y) * 0.5f};
    Vec2F32 m = p1->position;
    Vec2F32 m2 = {(p1->position.x + p2->position.x) * 0.5f, (p1->position.y + p2->position.y) * 0.5f};

    // scale stroke size based on dist, mimic ink pen effect
    F32 dist = length_2f32(sub_2f32(m1, m2));
    F32 t = round_f32(dist/base_stroke_size);
    F32 scale = mix_1f32(1.0, 0.85, t/2.0);
    scale = last_scale*0.9+0.1*scale;
    last_scale = scale;
    F32 stroke_size = base_stroke_size*scale;
    stroke_size = ClampBot(stroke_size, min_visiable_stroke_size);

    // draw quadratic curve: m1 -> m2 with p1 as control
    {
      Vec2F32 p0 = m1;
      Vec2F32 p1 = m;
      Vec2F32 p2 = m2;

      // decide step size
      F32 dist_px = length_2f32(sub_2f32(p0,p2))/ik_state->world_to_screen_ratio.x;
      U64 steps = floor_f32(dist_px/4.0);
      steps = Clamp(1, steps, 20);

      Vec2F32 prev = p0;
      for(U64 i = 1; i <= steps; i++)
      {
        F32 t = (F32)i / (F32)steps;
        F32 u = 1.0f - t;

        Vec2F32 pt = {
          u*u*p0.x + 2*u*t*p1.x + t*t*p2.x,
          u*u*p0.y + 2*u*t*p1.y + t*t*p2.y
        };

        // draw line segment (prev → pt) with thickness
#if 1
        if(length_2f32(sub_2f32(prev, pt)) > 1e-8)
        {
          dr_line_keyed(prev, pt, stroke_color, stroke_size, edge_softness, box->key_3f32);
        }
#else
        F32 half_stroke_size = stroke_size/2.0;
        {
          Vec2F32 pos = prev;
          Rng2F32 rect = {pos.x-half_stroke_size, pos.y-half_stroke_size, pos.x+half_stroke_size, pos.y+half_stroke_size};
          dr_rect(rect, stroke_color, half_stroke_size, 0, edge_softness);
        }
        {
          Vec2F32 pos = pt;
          Rng2F32 rect = {pos.x-half_stroke_size, pos.y-half_stroke_size, pos.x+half_stroke_size, pos.y+half_stroke_size};
          dr_rect(rect, stroke_color, half_stroke_size, 0, edge_softness);
        }
#endif
        prev = pt;
      }
    }

    p0 = p1;
    p1 = p2;
    p2 = p2->next;

    // no more points and last point haven't drawn? -> duplicate last point
    if(!p2 && !last_point_drawn)
    {
      p2 = p1;
      last_point_drawn = 1;
    }
  }
}

internal IK_Box *
ik_stroke()
{
  IK_Box *box = ik_build_box_from_stringf(0, "stroke###%I64u", os_now_microseconds());
  box->flags = IK_BoxFlag_MouseClickable|
               IK_BoxFlag_ClickToFocus|
               IK_BoxFlag_DragToPosition|
               IK_BoxFlag_DoubleClickToUnFocus|
               IK_BoxFlag_DragToScaleRectSize|
               IK_BoxFlag_DragToScalePoint|
               IK_BoxFlag_DrawStroke;
  box->hover_cursor = OS_Cursor_UpDownLeftRight;
  box->ratio = 1.0;
  return box;
}

IK_BOX_UPDATE(arrow)
{
  IK_Signal sig = box->sig;
  B32 is_focus_active = ik_key_match(ik_state->focus_active_box_key, box->key);

  if(is_focus_active)
  {
    for(UI_EventNode *n = ik_state->events->first, *next = 0; n != 0; n = next)
    {
      B32 taken = 0;
      next = n->next;
      UI_Event *evt = &n->v;

      if(evt->kind == UI_EventKind_Release && evt->key == OS_Key_LeftMouseButton)
      {
        is_focus_active = 0;
        ik_state->focus_active_box_key = ik_key_zero();
        ik_state->tool = IK_ToolKind_Selection;
      }

      if(taken)
      {
        ui_eat_event_node(ik_state->events, n);
      }
    }
  }

  if(is_focus_active)
  {
    IK_Point *a = box->first_point;
    IK_Point *m = a->next;
    IK_Point *b = box->last_point;
    b->position = ik_state->mouse_in_world;
    m->position = scale_2f32(add_2f32(a->position, b->position), 0.5);
  }

  /////////////////////////////////
  // calc point bounds
  {
    F32 half_stroke_size = box->stroke_size/2.0;
    Rng2F32 bounds = {inf32(), inf32(), neg_inf32(), neg_inf32()};
    for(IK_Point *p = box->first_point; p != 0; p = p->next)
    {
      Vec2F32 pos = p->position;
      Rng2F32 rect = {pos.x-half_stroke_size, pos.y-half_stroke_size, pos.x+half_stroke_size, pos.y+half_stroke_size};

      // update bounds
      bounds.x0 = Min(rect.x0, bounds.x0);
      bounds.x1 = Max(rect.x1, bounds.x1);
      bounds.y0 = Min(rect.y0, bounds.y0);
      bounds.y1 = Max(rect.y1, bounds.y1);
    }
    Vec2F32 bounds_dim = dim_2f32(bounds);
    box->position = bounds.p0;
    box->rect_size = bounds_dim;
    box->ratio = bounds_dim.x/bounds_dim.y;
  }
}

IK_BOX_DRAW(arrow)
{
  IK_Point *pa = box->first_point;
  IK_Point *pm = pa->next;
  IK_Point *pb = box->last_point;

  Vec2F32 a = pa->position;
  Vec2F32 m = pm->position;
  Vec2F32 b = pb->position;

  // calc control point
  // C = 2*P1 - (P0+P2)/2
  Vec2F32 c = sub_2f32(scale_2f32(m, 2), scale_2f32(add_2f32(a, b), 0.5));

  F32 stroke_size = box->stroke_size;
  F32 half_stroke_size = box->stroke_size*0.5;
  Vec4F32 stroke_clr = box->stroke_color;
  F32 edge_softness = 1.0 * ik_state->world_to_screen_ratio.x;

  // draw a
  {
    Rng2F32 rect = {.p0 = a, .p1 = a};
    rect = pad_2f32(rect, half_stroke_size*1.3);
    dr_rect_keyed(rect, stroke_clr, half_stroke_size*1.3, 0, edge_softness, box->key_3f32);
  }

  // draw b with arrow
  {
    Rng2F32 rect = {.p0 = b, .p1 = b};
    rect = pad_2f32(rect, half_stroke_size*1.3);
    dr_rect_keyed(rect, stroke_clr, half_stroke_size*1.3, 0, edge_softness, box->key_3f32);

    // draw arrow direction
    Vec2F32 dir = normalize_2f32(sub_2f32(b, c));
    Vec2F32 dir_inv = {-dir.x, -dir.y};
    F32 angle_turns = 0.08;
    F32 arrow_length = stroke_size * 6;

    // up
    Vec2F32 up =
    {
      dir_inv.x*cos_f32(angle_turns)-dir_inv.y*sin_f32(angle_turns),
      dir_inv.x*sin_f32(angle_turns)+dir_inv.y*cos_f32(angle_turns)
    };
    Vec2F32 up_end = add_2f32(b, scale_2f32(up, arrow_length));
    R_Rect2DInst *inst =  dr_line_keyed(b, up_end, stroke_clr, stroke_size, edge_softness, box->key_3f32);

    // down
    Vec2F32 down = 
    {
      dir_inv.x*cos_f32(-angle_turns)-dir_inv.y*sin_f32(-angle_turns),
      dir_inv.x*sin_f32(-angle_turns)+dir_inv.y*cos_f32(-angle_turns)
    };
    Vec2F32 down_end = add_2f32(b, scale_2f32(down, arrow_length));
    dr_line_keyed(b, down_end, stroke_clr, stroke_size, edge_softness, box->key_3f32);
  }

  // draw a line between a and b, m as control point
  {
    Vec2F32 p0 = a;
    Vec2F32 p1 = c;
    Vec2F32 p2 = b;

    // decide step size 
    F32 dist_px = length_2f32(sub_2f32(p0,p2))/ik_state->world_to_screen_ratio.x;
    U64 steps = floor_f32(dist_px/4.0);
    steps = Clamp(1, steps, 30);

    Vec2F32 prev = p0;
    F32 last_scaled_stroke_size = stroke_size;
    for(U64 i = 1; i <= steps; i++)
    {
      F32 scaled_stroke_size = last_scaled_stroke_size*0.96f;
      scaled_stroke_size = ClampBot(stroke_size*0.25, scaled_stroke_size);
      last_scaled_stroke_size = scaled_stroke_size;
      F32 t = (F32)i / (F32)steps;
      F32 u = 1.0f - t;

      Vec2F32 pt = {
        u*u*p0.x + 2*u*t*p1.x + t*t*p2.x,
        u*u*p0.y + 2*u*t*p1.y + t*t*p2.y
      };

      // draw line segment (prev → pt) with thickness
      dr_line_keyed(prev, pt, stroke_clr, scaled_stroke_size, edge_softness, box->key_3f32);
      prev = pt;
    }
  }
}

internal IK_Box *
ik_arrow()
{
  IK_Box *box = ik_build_box_from_stringf(0, "arrow###%I64u", os_now_microseconds());
  box->flags = IK_BoxFlag_MouseClickable|
               IK_BoxFlag_ClickToFocus|
               IK_BoxFlag_DragToPosition|
               IK_BoxFlag_DragToScaleStrokeSize|
               IK_BoxFlag_DrawArrow|
               IK_BoxFlag_DragPoint|
               IK_BoxFlag_DragToScalePoint;
  box->hover_cursor = OS_Cursor_UpDownLeftRight;
  box->ratio = 1.0;
  // NOTE(k): it's a hack, in this case relative point pos is just absolute pos
  // after commited, we set box position based on bounds, then convert point pos to relative space 
  box->position = v2f32(0,0);

  // alloc 3 points a, m, b
  for(U64 i = 0; i < 3; i++)
  {
    IK_Point *p = ik_point_alloc();
    p->position = ik_state->mouse_in_world;
    DLLPushBack(box->first_point, box->last_point, p);
    box->point_count++;
  }
  return box;
}

/////////////////////////////////
//~ Box Manipulation Functions

internal void
ik_box_do_scale(IK_Box *box, Vec2F32 scale, Vec2F32 origin)
{
  // rect size
  if(box->flags & IK_BoxFlag_DragToScaleRectSize)
  {
    box->rect_size.x *= scale.x;
    box->rect_size.y *= scale.y;
  }

  // font size
  if(box->flags&IK_BoxFlag_DragToScaleFontSize)
  {
    box->font_size *= scale.y;
  }

  // stroke_size
  if(box->flags&IK_BoxFlag_DragToScaleStrokeSize)
  {
    // FIXME: this won't cut it, stroke and arrow are fixed-ratio
    box->stroke_size *= scale.y;
  }

  // position scale
  {
    Vec2F32 pos_rel = sub_2f32(box->position, origin);
    pos_rel.x *= scale.x;
    pos_rel.y *= scale.y;
    box->position = add_2f32(pos_rel, origin);
  }

  // point scale
  if(box->flags & IK_BoxFlag_DragToScalePoint)
  {
    for(IK_Point *p = box->first_point; p != 0; p = p->next)
    {
      Vec2F32 pos_next = sub_2f32(p->position, origin);
      pos_next.x *= scale.x;
      pos_next.y *= scale.y;
      pos_next = add_2f32(pos_next, origin);
      p->position = pos_next;
    }
  }

  for(IK_Box *child = box->group_first;
      child != 0;
      child = child->group_next)
  {
    ik_box_do_scale(child, scale, origin);
  }
}

internal void
ik_box_do_translate(IK_Box *box, Vec2F32 translate)
{
  box->position = add_2f32(box->position, translate);

  // translate points
  for(IK_Point *p = box->first_point; p != 0; p = p->next)
  {
    p->position.x += translate.x;
    p->position.y += translate.y;
  }

  for(IK_Box *child = box->group_first;
      child != 0;
      child = child->group_next)
  {
    ik_box_do_translate(child, translate);
  }
}

/////////////////////////////////
//~ Action Type Functions

//- ring buffer operations

internal IK_Action *
ik_action_ring_write()
{
  IK_Frame *frame = ik_top_frame();
  IK_ActionRing *ring = &frame->action_ring;

  U64 next_head = (ring->head+1) % ring->slot_count;
  U64 next_tail = ring->tail;

  U64 written = 1;
  if(ring->write_count > 0 && ring->head == ring->tail)
  {
    next_tail = ring->head;
    written = 0;
  }

  IK_Action *ret = &ring->slots[ring->head];
  // commit head&tail
  ring->mark = ring->head;
  ring->head = next_head;
  ring->tail = next_tail;
  ring->write_count += written;
  return ret;
}

internal IK_Action *
ik_action_ring_backward()
{
  IK_Frame *frame = ik_top_frame();
  IK_ActionRing *ring = &frame->action_ring;

  IK_Action *ret = 0;
  U64 next_head = (ring->head-1) % ring->slot_count;
  if(ring->write_count > 0)
  {
    ret = &ring->slots[next_head];
    // commit head
    ring->head = next_head;
    ring->write_count--;
  }
  return ret;
}

internal IK_Action *
ik_action_ring_forward()
{
  IK_Frame *frame = ik_top_frame();
  IK_ActionRing *ring = &frame->action_ring;
  IK_Action *ret = 0;
  U64 next_head = (ring->head+1) % ring->slot_count;
  U64 marked_head = (ring->mark+1) % ring->slot_count;
  if(marked_head != next_head)
  {
    ret = &ring->slots[ring->head];
    ring->head = next_head;
    ring->write_count++;
  }
  return ret;
}

//- action undo/redo

internal void
ik_action_undo(IK_Action *action)
{
  switch(action->kind)
  {
    case IK_ActionKind_Create:
    {
      for(IK_Box *box = action->v.create.first, *next = 0; box != 0; box = next)
      {
        // next = box->create_next;
        // FIXME: soft delete
      }
    }break;
    case IK_ActionKind_Delete:
    {
      for(IK_Box *box = action->v.create.first, *next = 0; box != 0; box = next)
      {
        // next = box->delete_next;
        // FIXME: create
      }
    }break;
    default:{}break;
  }
}

//- state undo/redo

/////////////////////////////////
//~ Point Function

internal IK_Point *
ik_point_alloc()
{
  IK_Frame *frame = ik_top_frame();
  IK_Point *point = frame->first_free_point;

  if(point)
  {
    SLLStackPop(frame->first_free_point);
  }
  else
  {
    point = push_array_no_zero(frame->arena, IK_Point, 1);
  }
  MemoryZeroStruct(point);
  return point;
}

internal void
ik_point_release(IK_Point *point)
{
  IK_Frame *frame = ik_top_frame();
  SLLStackPush(frame->first_free_point, point);
}

/////////////////////////////////
//~ Image Function

internal String8
ik_decoded_from_bytes(Arena *arena, String8 bytes, Vec3F32 *return_size)
{
  ProfBeginFunction();
  String8 ret = {0};
  int x = 0;
  int y = 0;
  int z = 0;
  U8 *data = stbi_load_from_memory(bytes.str, bytes.size, &x, &y, &z, 4); // this is image data (U32 -> RBGA)
  if(data)
  {
    U64 size = x*y*4;
    ret.str = push_array(arena, U8, size);
    ret.size = size;
    MemoryCopy(ret.str, data, size);
    return_size->x = x;
    return_size->y = y;
    return_size->z = 4;
  }
  stbi_image_free(data);
  ProfEnd();
  return ret;
}

internal IK_Image *
ik_image_push(IK_Key key)
{
  IK_Frame *frame = ik_top_frame();
  IK_ImageCacheNode *cache_node = frame->first_free_image_cache_node;
  if(cache_node)
  {
    SLLStackPop(frame->first_free_image_cache_node);
  }
  else
  {
    cache_node = push_array_no_zero(frame->arena, IK_ImageCacheNode, 1);
  }
  MemoryZeroStruct(cache_node);

  // hook into cache table
  U64 slot_index = key.u64[0]%ArrayCount(frame->image_cache_table);
  IK_ImageCacheSlot *slot = &frame->image_cache_table[slot_index];
  DLLPushBack(slot->first, slot->last, cache_node);

  IK_Image *ret = &cache_node->v;
  ret->key = key;
  return ret;
}

internal IK_Image *
ik_image_from_key(IK_Key key)
{
  IK_Image *ret = 0;
  IK_Frame *frame = ik_top_frame();

  if(!ik_key_match(key, ik_key_zero()))
  {
    U64 slot_index = key.u64[0]%ArrayCount(frame->image_cache_table);
    IK_ImageCacheSlot *slot = &frame->image_cache_table[slot_index];

    IK_ImageCacheNode *node = 0;
    for(IK_ImageCacheNode *n = slot->first; n != 0; n = n->next)
    {
      if(ik_key_match(n->v.key, key))
      {
        node = n;
        break;
      }
    }

    if(node)
    {
      ret = &node->v;
    }
  }
  return ret;
}

/////////////////////////////////
//~ Image Decode Threads

internal void
ik_image_decode_thread__entry_point(void *ptr)
{
  ThreadNameF("[ik] image decode thread");

  IK_ImageDecodeQueue *queue = (IK_ImageDecodeQueue*)ptr;
  for(;;)
  {
    semaphore_take(queue->semaphore, max_U64);
    // TODO(Next): wrap cursor too, otherwise cursor could max out max_U64
    U64 index = (ins_atomic_u64_inc_eval(&queue->cursor)-1)%ArrayCount(queue->queue);
    IK_Image *image = queue->queue[index];

    AssertAlways(!image->loaded);

    int x = 0;
    int y = 0;
    int z = 0;
    U8 *data = stbi_load_from_memory(image->encoded.str, image->encoded.size, &x, &y, &z, 4); // this is image data (U32 -> RBGA)
    if(data)
    {
      image->x = x;
      image->y = y;
      image->comp = z;
      image->decoded = data;
    }

    // NOTE(k): we might need a write barrier here (cpu+compiler) to ensure all therads can see the writes above
    // or we could just use atom_store
    // #include <stdatomic.h>
    // atomic_thread_fence(memory_order_release);
    ins_atomic_u64_eval_assign(&image->loading, 0);
    ins_atomic_u64_dec_eval(&queue->queue_count);
  }
}

internal void
ik_image_decode_queue_push(IK_Image *image)
{
  IK_ImageDecodeQueue *queue = &ik_state->decode_queue;
  for(;queue->queue_count >= ArrayCount(queue->queue);) {}

  image->loading = 1;
  U64 next_mark = (queue->mark+1)%ArrayCount(queue->queue);
  queue->queue[queue->mark] = image;
  ins_atomic_u64_inc_eval(&queue->queue_count);
  queue->mark = next_mark;
  // NOTE(k): we might not need a write barrier(compiler+cpu), since semaphore could have fence itself
  semaphore_drop(ik_state->decode_queue.semaphore);
}

/////////////////////////////////
//~ User interaction

internal IK_Signal
ik_signal_from_box(IK_Box *box)
{
  IK_Signal sig = {0};
  sig.box = box;
  sig.event_flags |= os_get_modifiers();
  Rng2F32 rect = ik_rect_from_box(box);
  B32 pixel_hot = ik_key_match(box->key, ik_state->pixel_hot_key);

  /////////////////////////////////
  //~ Process events related to this box

  for(UI_EventNode *n = ik_state->events->first, *next = 0; n != 0; n = next)
  {
    B32 taken = 0;
    next = n->next;
    UI_Event *evt = &n->v;

    //- unpack event
    // Vec2F32 evt_mouse = evt->pos;
    Vec2F32 evt_mouse = ik_state->mouse_in_world;
    // B32 evt_mouse_in_bounds = contains_2f32(rect, evt_mouse);
    B32 evt_mouse_in_bounds = pixel_hot;
    IK_MouseButtonKind evt_mouse_button_kind = 
      evt->key == OS_Key_LeftMouseButton   ? IK_MouseButtonKind_Left   :
      evt->key == OS_Key_RightMouseButton  ? IK_MouseButtonKind_Right  :
      evt->key == OS_Key_MiddleMouseButton ? IK_MouseButtonKind_Middle :
      IK_MouseButtonKind_Left;
    B32 evt_key_is_mouse =
      evt->key == OS_Key_LeftMouseButton  || 
      evt->key == OS_Key_RightMouseButton ||
      evt->key == OS_Key_MiddleMouseButton;
    sig.event_flags |= evt->modifiers;

    //- mouse pressed in box -> set hot/active, mark signal accordingly
    if(box->flags & IK_BoxFlag_MouseClickable &&
       evt->kind == UI_EventKind_Press &&
       evt_mouse_in_bounds &&
       evt_key_is_mouse)
    {
      ik_state->hot_box_key = box->key;
      ik_state->active_box_key[evt_mouse_button_kind] = box->key;
      sig.f |= (IK_SignalFlag_LeftPressed<<evt_mouse_button_kind);
      ik_state->drag_start_mouse = evt->pos;

      // reset focus hot keys on mouse button press
      for EachEnumVal(IK_MouseButtonKind, k)
      {
        ik_state->focus_hot_box_key[k] = ik_key_zero();
      }

      if(ik_key_match(box->key, ik_state->press_key_history[evt_mouse_button_kind][0]) &&
         evt->timestamp_us-ik_state->press_timestamp_history_us[evt_mouse_button_kind][0] <= 1000000*os_get_gfx_info()->double_click_time)
      {
        sig.f |= (IK_SignalFlag_LeftDoubleClicked<<evt_mouse_button_kind);
      }

      if(ik_key_match(box->key, ik_state->press_key_history[evt_mouse_button_kind][0]) &&
         ik_key_match(box->key, ik_state->press_key_history[evt_mouse_button_kind][1]) &&
         evt->timestamp_us-ik_state->press_timestamp_history_us[evt_mouse_button_kind][0] <= 1000000*os_get_gfx_info()->double_click_time &&
         ik_state->press_timestamp_history_us[evt_mouse_button_kind][0] - ik_state->press_timestamp_history_us[evt_mouse_button_kind][1] <= 1000000*os_get_gfx_info()->double_click_time)
      {
        sig.f |= (IK_SignalFlag_LeftTripleClicked<<evt_mouse_button_kind);
      }

      MemoryCopy(&ik_state->press_key_history[evt_mouse_button_kind][1],
                 &ik_state->press_key_history[evt_mouse_button_kind][0],
                 sizeof(ik_state->press_key_history[evt_mouse_button_kind][0]) * (ArrayCount(ik_state->press_key_history)-1));
      MemoryCopy(&ik_state->press_timestamp_history_us[evt_mouse_button_kind][1],
                 &ik_state->press_timestamp_history_us[evt_mouse_button_kind][0],
                 sizeof(ik_state->press_timestamp_history_us[evt_mouse_button_kind][0]) * (ArrayCount(ik_state->press_timestamp_history_us)-1));
      MemoryCopy(&ik_state->press_pos_history[evt_mouse_button_kind][1], &ik_state->press_pos_history[evt_mouse_button_kind][0],
                 sizeof(ik_state->press_pos_history[evt_mouse_button_kind][0]) * ArrayCount(ik_state->press_pos_history[evt_mouse_button_kind])-1);

      ik_state->press_key_history[evt_mouse_button_kind][0] = box->key;
      ik_state->press_timestamp_history_us[evt_mouse_button_kind][0] = evt->timestamp_us;
      ik_state->press_pos_history[evt_mouse_button_kind][0] = evt_mouse;

      taken = 1;
    }

    //- mouse released in active box -> unset active
    if(box->flags & IK_BoxFlag_MouseClickable &&
       evt->kind == UI_EventKind_Release &&
       ik_key_match(box->key, ik_state->active_box_key[evt_mouse_button_kind]) &&
       evt_mouse_in_bounds &&
       evt_key_is_mouse)
    {
      ik_state->hot_box_key = box->key;
      ik_state->active_box_key[evt_mouse_button_kind] = ik_key_zero();
      sig.f |= (IK_SignalFlag_LeftReleased << evt_mouse_button_kind);
      sig.f |= (IK_SignalFlag_LeftClicked << evt_mouse_button_kind);
      taken = 1;
    }

    //- mouse released outside of active box -> unset hot & active
    if(box->flags & IK_BoxFlag_MouseClickable &&
       evt->kind == UI_EventKind_Release &&
       ik_key_match(box->key, ik_state->active_box_key[evt_mouse_button_kind]) &&
       !evt_mouse_in_bounds &&
       evt_key_is_mouse)
    {
      ik_state->hot_box_key = ik_key_zero();
      ik_state->active_box_key[evt_mouse_button_kind] = ik_key_zero();
      sig.f |= (IK_SignalFlag_LeftReleased << evt_mouse_button_kind);
      taken = 1;
    }

    //- scroll
    if(box->flags & IK_BoxFlag_Scroll &&
       evt->kind == UI_EventKind_Scroll &&
       evt->modifiers != OS_Modifier_Ctrl &&
       evt_mouse_in_bounds)
    {
      Vec2F32 delta = evt->delta_2f32;
      if(evt->modifiers & OS_Modifier_Shift)
      {
        Swap(F32, delta.x, delta.y);
      }
      Vec2S16 delta16 = v2s16((S16)(delta.x/30.f), (S16)(delta.y/30.f));
      if(delta.x > 0 && delta16.x == 0) { delta16.x = +1; }
      if(delta.x < 0 && delta16.x == 0) { delta16.x = -1; }
      if(delta.y > 0 && delta16.y == 0) { delta16.y = +1; }
      if(delta.y < 0 && delta16.y == 0) { delta16.y = -1; }
      sig.scroll.x += delta16.x;
      sig.scroll.y += delta16.y;
      taken = 1;
    }

    if(ik_key_match(ik_state->focus_hot_box_key[IK_MouseButtonKind_Left], box->key) &&
       evt->kind == UI_EventKind_Press &&
       evt->key == OS_Key_Delete)
    {
      sig.f |= IK_SignalFlag_Delete;
      taken = 1;
    }

    if(taken)
    {
      ui_eat_event_node(ik_state->events, n);
    }
  }

  // B32 mouse_in_this_rect = contains_2f32(rect, ik_state->mouse_in_world);
  B32 mouse_in_this_rect = pixel_hot;

  //////////////////////////////
  //~ Mouse is over this box's rect -> always mark mouse-over

  if(pixel_hot)
  {
    sig.f |= IK_SignalFlag_MouseOver;
  }

  //////////////////////////////
  //~ Mouse is over this box's rect, no other hot key? -> set hot key, mark hovering

  if(box->flags & IK_BoxFlag_MouseClickable &&
     mouse_in_this_rect &&
     (ik_key_match(ik_state->hot_box_key, ik_key_zero()) || ik_key_match(ik_state->hot_box_key, box->key)) &&
     (ik_key_match(ik_state->active_box_key[IK_MouseButtonKind_Left], ik_key_zero()) || ik_key_match(ik_state->active_box_key[IK_MouseButtonKind_Left], box->key)) &&
     (ik_key_match(ik_state->active_box_key[IK_MouseButtonKind_Middle], ik_key_zero()) || ik_key_match(ik_state->active_box_key[IK_MouseButtonKind_Middle], box->key)) &&
     (ik_key_match(ik_state->active_box_key[IK_MouseButtonKind_Right], ik_key_zero()) || ik_key_match(ik_state->active_box_key[IK_MouseButtonKind_Right], box->key)))
  {
    ik_state->hot_box_key = box->key;
    sig.f |= IK_SignalFlag_Hovering;
  }

  //////////////////////////////
  // TODO: Mouse is over this box's rect, drop site, no other drop hot key? -> set drop hot key

  // {
  //   if(box->flags & UI_BoxFlag_DropSite &&
  //      contains_2f32(rect, ui_state->mouse) &&
  //      (ui_key_match(ui_state->drop_hot_box_key, ui_key_zero()) || ui_key_match(ui_state->drop_hot_box_key, box->key)))
  //   {
  //     ui_state->drop_hot_box_key = box->key;
  //   }
  // }

  //////////////////////////////
  // TODO: Mouse is not over this box's rect, but this is the drop hot key? -> zero drop hot key

  // {
  //   if(box->flags & UI_BoxFlag_DropSite &&
  //      !contains_2f32(rect, ui_state->mouse) &&
  //      ui_key_match(ui_state->drop_hot_box_key, box->key))
  //   {
  //     ui_state->drop_hot_box_key = ui_key_zero();
  //   }
  // }

  //////////////////////////////
  //~ Active -> dragging

  if(box->flags & IK_BoxFlag_MouseClickable)
  {
    for EachEnumVal(IK_MouseButtonKind, k)
    {
      if(ik_key_match(ik_state->active_box_key[k], box->key) || sig.f & (IK_SignalFlag_LeftPressed<<k))
      {
        sig.f |= (IK_SignalFlag_LeftDragging<<k);
      }
    }
  }

  //////////////////////////////
  //~ Mouse is not over this box, hot key is the box? -> unset hot key

  if(!mouse_in_this_rect && ik_key_match(ik_state->hot_box_key, box->key))
  {
    ik_state->hot_box_key = ik_key_zero();
  }

  //////////////////////////////
  //~ Box is active, but focus_hot/focus_active is not this key? -> reset them to 0

  if(ik_key_match(ik_state->active_box_key[IK_MouseButtonKind_Left], box->key))
  {
    for EachEnumVal(IK_MouseButtonKind, k)
    {
      if(!ik_key_match(ik_state->focus_hot_box_key[k], box->key))
      {
        ik_state->focus_hot_box_key[k] = ik_key_zero();
      }
    }

    if(!ik_key_match(ik_state->focus_active_box_key, box->key))
    {
      ik_state->focus_active_box_key = ik_key_zero();
    }
  }

  //////////////////////////////
  //~ Clicked on this box and set focus_hot_key

  if(box->flags & IK_BoxFlag_ClickToFocus)
  {
    for EachEnumVal(IK_MouseButtonKind, k)
    {
      if(sig.f&IK_SignalFlag_LeftPressed<<k)
      {
        ik_state->focus_hot_box_key[k] = box->key;
        // TODO(k): hack, we just need this work for now
        if(k == IK_MouseButtonKind_Right)
        {
          ik_state->focus_hot_box_key[IK_MouseButtonKind_Left] = box->key;
        }
      }
    }
  }

  if(box->flags & IK_BoxFlag_DoubleClickToUnFocus)
  {
    for EachEnumVal(IK_MouseButtonKind, k)
    {
      if(sig.f&IK_SignalFlag_LeftDoubleClicked<<k)
      {
        ik_state->focus_hot_box_key[k] = ik_key_zero();
      }
    }
  }

  ////////////////////////////////
  //~ Box overlap with selection rect? -> add to selection list

  if(ik_is_selecting() && !(box->flags&IK_BoxFlag_OmitGroupSelection))
  {
    if(overlaps_2f32(ik_state->selection_rect, rect))
    {
      sig.f |= IK_SignalFlag_Select;
    }
  }

  return sig;
}

/////////////////////////////////
//~ UI Widget

internal void
ik_ui_stats(void)
{
  if(!ik_state->show_stats) return;

  IK_Frame *frame = ik_top_frame();
  IK_Camera *camera = &frame->camera;
  Vec2F32 viewport_dim = dim_2f32(camera->rect);

  UI_Box *container = 0;
  F32 width = ui_top_font_size()*25;
  Rng2F32 rect = {ik_state->window_dim.x-width, 0, ik_state->window_dim.x, ik_state->window_dim.y};
  UI_Rect(rect)
    UI_ChildLayoutAxis(Axis2_Y)
  {
    container = ui_build_box_from_stringf(0, "###stats_container");
  }

  UI_Box *body;
  UI_Parent(container)
  {
    ui_spacer(ui_em(0.5, 0.0));

    UI_WidthFill
      UI_PrefHeight(ui_children_sum(0.0))
      UI_Row
      UI_Padding(ui_em(0.5, 0.0))
      UI_PrefHeight(ui_pct(1.0, 0.0))
      UI_PrefHeight(ui_children_sum(0.0))
      UI_Flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawDropShadow|UI_BoxFlag_DrawBackground)
      UI_Transparency(0.4)
      UI_ChildLayoutAxis(Axis2_Y)
      UI_CornerRadius(1.0)
      body = ui_build_box_from_stringf(0, "###body");
  }

  UI_Parent(body)
    UI_TextAlignment(UI_TextAlign_Left)
    UI_TextPadding(9)
    UI_PrefWidth(ui_pct(1.0, 0.0))
  {
    // collect some values
    U64 last_frame_index = ik_state->frame_index > 0 ? ik_state->frame_index-1 : 0;
    U64 last_frame_us = ik_state->frame_time_us_history[last_frame_index%ArrayCount(ik_state->frame_time_us_history)];

    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("frame");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%.3fms", (F32)last_frame_us/1000.0);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("pre cpu time");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%.3fms", (F32)ik_state->pre_cpu_time_us/1000.0);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("cpu time");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%.3fms", (F32)ik_state->cpu_time_us/1000.0);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("fps");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%.2f", 1.0 / (last_frame_us/1000000.0));
    }
    ui_divider(ui_em(0.1, 0.0));
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ik_box_count");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%lu", frame->box_list.count);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ik_hot_key");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%lu", ik_state->hot_box_key.u64[0]);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ik_active_key");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%lu", ik_state->active_box_key[IK_MouseButtonKind_Left].u64[0]);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ik_focus_hot_key");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%lu", ik_state->focus_hot_box_key[IK_MouseButtonKind_Left].u64[0]);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ik_focus_active_key");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%lu", ik_state->focus_active_box_key.u64[0]);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ik_pixel_hot_key");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%I64u", ik_state->pixel_hot_key.u64[0]);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ik_mouse");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%.0f %.0f", ik_state->mouse.x, ik_state->mouse.y);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ik_mouse_in_world");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%.0f %.0f", ik_state->mouse_in_world.x, ik_state->mouse_in_world.y);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ik_drag_start_mouse");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%.0f %.0f", ik_state->drag_start_mouse.x, ik_state->drag_start_mouse.y);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("viewport_ratio");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%.2f", viewport_dim.x/viewport_dim.y);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("window_ratio");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%.2f", ik_state->window_dim.x/ik_state->window_dim.y);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("window_dim");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%.2f %.2f", ik_state->window_dim.x, ik_state->window_dim.y);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("world_to_screen_ratio");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%.2f %.2f", ik_state->world_to_screen_ratio.x, ik_state->world_to_screen_ratio.y);
    }
    ui_divider(ui_em(0.1, 0.0));
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ui_hot_key");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%I64u", ui_state->hot_box_key.u64[0]);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ui_active_key");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%I64u", ui_state->active_box_key[UI_MouseButtonKind_Left].u64[0]);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ui_last_build_box_count");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%lu", ui_state->last_build_box_count);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("ui_build_box_count");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%lu", ui_state->build_box_count);
    }
    UI_Row
      UI_PrefWidth(ui_text_dim(1, 1.0))
    {
      ui_labelf("drag start mouse");
      ui_spacer(ui_pct(1.0, 0.0));
      ui_labelf("%.2f, %.2f", ui_state->drag_start_mouse.x, ui_state->drag_start_mouse.y);
    }

    // cpu graph
    UI_WidthFill
    UI_PrefHeight(ui_em(2,0.f))
    UI_Row
    {
      F32 ref_hz = 1.f/60.f;
      F32 max = (ref_hz*1e6)*3;
      for(U64 i = 0; i < ArrayCount(ik_state->frame_time_us_history); i++)
      {
        U64 idx = (ik_state->frame_index+i)%ArrayCount(ik_state->frame_time_us_history);
        F32 pct = (F32)ik_state->frame_time_us_history[idx]/max;
        pct = Clamp(0, pct, 1.0);

        UI_HeightFill
        UI_PrefWidth(ui_pct(1.0, 0.0))
        UI_Column
        {
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefHeight(ui_pct(pct, 1.0))
          UI_Flags(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder)
            ui_build_box_from_stringf(0, "###frame_%I64u", idx);
        }
      }
    }
  }
}

internal void
ik_ui_man_page()
{
  F32 open_t = ui_anim(ui_key_from_stringf(ui_key_zero(), "man_page_open_t"), ik_state->show_man_page, .reset = 0, .rate = ik_state->animation.fast_rate);
  if(open_t < 1e-3) return;

  Rng2F32 rect = {0,0, ik_state->window_dim.x, ik_state->window_dim.y};

  UI_Box *container;
  UI_Rect(rect)
    UI_Flags(UI_BoxFlag_MouseClickable|UI_BoxFlag_Scroll|UI_BoxFlag_DrawBackground)
    UI_Transparency(mix_1f32(0, 0.2, open_t))
    UI_Squish(1.f-open_t)
    container = ui_build_box_from_stringf(0, "###man_page_container");

  UI_Box *body;
  UI_Parent(container)
    UI_WidthFill
    UI_HeightFill
    UI_Column
    UI_Padding(ui_pct(1.0, 0.0))
    UI_PrefHeight(ui_children_sum(1.0))
    UI_Row
    UI_Padding(ui_pct(1.0, 0.0))
    UI_Flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawDropShadow|UI_BoxFlag_MouseClickable)
    UI_PrefWidth(ui_children_sum(1.0))
    UI_CornerRadius(1.f)
    body = ui_build_box_from_stringf(0, "###body");

  UI_Box *padded;
  UI_Parent(body)
    UI_PrefHeight(ui_children_sum(1.0))
    UI_PrefWidth(ui_children_sum(1.0))
    UI_Column
    UI_Padding(ui_em(0.5, 0.0))
    UI_Row
    UI_Padding(ui_em(0.5, 0.0))
    UI_ChildLayoutAxis(Axis2_Y)
    UI_PrefWidth(ui_em(30.f,1.0))
    padded = ui_build_box_from_stringf(0, "###padded");

  UI_Parent(padded)
  {
    /////////////////////////////////
    // Header

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("Man Page");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        UI_Font(ik_font_from_slot(IK_FontSlot_Icons))
        if(ui_pressed(ik_ui_buttonf("x")))
        {
          ik_state->show_man_page = !ik_state->show_man_page;
        }
    }
    UI_WidthFill
    ui_divider(ui_em(1.5, 0.0));

    /////////////////////////////////
    // Navigation

    ui_set_next_pref_width(ui_pct(1.0, 0.0));
    ui_set_next_flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawDropShadow);
    ui_labelf("Camera");
    ui_spacer(ui_em(0.2,0.0));

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("Zoom");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("CTRL+SCROLL");
    }

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("Pan");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("Hold space and drag");
    }

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("PageDown/PageUp");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("Mouse scroll");
    }

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("Center");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("Double click image to center viewport");
    }

    /////////////////////////////////
    // Edit

    ui_set_next_pref_width(ui_pct(1.0, 0.0));
    ui_set_next_flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawDropShadow);
    ui_labelf("Edit");
    ui_spacer(ui_em(0.2,0.0));

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("Type");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("Double click on empty area to type");
    }

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("PasteText");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("CTRL+V to paste text from clipboard");
    }

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("PasteImage");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("CTRL+V to paste image from clipboard");
    }

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("FileDrop");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("TODO");
    }

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("Delete");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("Select box then press Delete");
    }

    /////////////////////////////////
    // File

    ui_set_next_pref_width(ui_pct(1.0, 0.0));
    ui_set_next_flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawDropShadow);
    ui_labelf("File");
    ui_spacer(ui_em(0.2,0.0));

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("Save");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("CTRL+S");
    }

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("SaveAs");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("TODO");
    }

    /////////////////////////////////
    // Debug

    ui_set_next_pref_width(ui_pct(1.0, 0.0));
    ui_set_next_flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawDropShadow);
    ui_labelf("Debug");
    ui_spacer(ui_em(0.2,0.0));

    UI_WidthFill
    UI_Row
    {
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("ShowStats");
      ui_spacer(ui_pct(1.0, 0.0));
      UI_PrefWidth(ui_text_dim(1, 1.0))
        ui_labelf("CTRL+`");
    }

    UI_WidthFill
    ui_divider(ui_em(1.5, 0.0));
  }

  ui_signal_from_box(body);
  UI_Signal sig = ui_signal_from_box(container);
  if(ui_pressed(sig))
  {
    ik_state->show_man_page = 0;
  }
}

internal void
ik_ui_toolbar(void)
{
  UI_Box *container;
  Rng2F32 rect = {0, 0, ik_state->window_dim.x, ui_top_font_size()*10};
  UI_Rect(rect)
    container = ui_build_box_from_stringf(0, "##toolbar_container");

  F32 cell_width = ui_top_font_size()*2.5;
  UI_Box *inner;
  UI_Parent(container)
    UI_HeightFill
    UI_WidthFill
    UI_Column
    UI_Padding(ui_pct(1.0, 0.0))
    UI_Row
    UI_Padding(ui_pct(1.0, 0.0))
    UI_Flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawDropShadow|UI_BoxFlag_DrawBackground)
    UI_PrefWidth(ui_px(cell_width*IK_ToolKind_COUNT, 1.0))
    UI_PrefHeight(ui_px(cell_width, 1.0))
    UI_ChildLayoutAxis(Axis2_X)
    inner = ui_build_box_from_stringf(0, "###inner");

  UI_Parent(inner)
  {
    String8 strs[IK_ToolKind_COUNT] =
    {
      str8_lit(" "),  // hand
      str8_lit("!"),  // selection
      str8_lit("\""), // rectangle
      str8_lit("#"),  // draw
      str8_lit("3"),  // text
      str8_lit("4"),  // arrow
      str8_lit("&"),  // man
    };

    for(U64 i = 0; i < IK_ToolKind_COUNT; i++)
    {
      UI_Box *b;

      UI_BoxFlags flags = UI_BoxFlag_DrawText|UI_BoxFlag_MouseClickable|UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawActiveEffects|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder;
      B32 is_active = ik_tool() == i;
      if(is_active)
      {
        ui_set_next_palette(ui_build_palette(ui_top_palette(),
                                             .border = ik_rgba_from_theme_color(IK_ThemeColor_BaseBackground),
                                             .background = ik_rgba_from_theme_color(IK_ThemeColor_Breakpoint)));
        ui_set_next_font_size(ui_top_font_size()*1.3);
        flags |= UI_BoxFlag_DrawDropShadow;
      }

      // icon
      UI_PrefWidth(ui_px(cell_width, 1.0))
        UI_PrefHeight(ui_px(cell_width, 1.0))
        UI_Font(ik_font_from_slot(IK_FontSlot_IconsExtra))
        UI_Flags(flags)
        UI_TextAlignment(UI_TextAlign_Center)
        b = ui_build_box_from_string(0, strs[i]);
      UI_Signal sig = ui_signal_from_box(b);

      // hot key indicator
      UI_Parent(b)
        UI_Flags(UI_BoxFlag_Floating|UI_BoxFlag_DisableTextTrunc|UI_BoxFlag_DrawText|UI_BoxFlag_DrawTextWeak)
        UI_FixedPos(v2f32(0,-3))
        UI_PrefWidth(ui_text_dim(0.0, 1.0))
        UI_PrefHeight(ui_text_dim(0.0, 1.0))
        UI_FontSize(ui_top_font_size()*0.7)
        ui_build_box_from_stringf(0, "%I64u", i+1);

      if(ui_pressed(sig))
      {
        switch(i)
        {
          default: {ik_state->tool = i;}break;
          case IK_ToolKind_Man:
          {
            ik_state->show_man_page = !ik_state->show_man_page;
          }break;
        }
      }
    }
  }
}

typedef struct UI_LineDrawData UI_LineDrawData;
struct UI_LineDrawData
{
  Vec2F32 a;
  Vec2F32 b;
  F32 stroke_size;
  Vec4F32 stroke_color;
};

internal UI_BOX_CUSTOM_DRAW(ui_line_draw)
{
  UI_LineDrawData *draw_data = (UI_LineDrawData *)user_data;
  dr_line(draw_data->a, draw_data->b, draw_data->stroke_color, draw_data->stroke_size, 1.0);
}

internal void
ik_ui_selection(void)
{
  ProfBeginFunction();
  IK_Frame *frame = ik_top_frame();
  IK_Camera *camera = &frame->camera;

  IK_Box *box = ik_box_from_key(ik_state->focus_hot_box_key[IK_MouseButtonKind_Left]);
  if(box && ik_tool() == IK_ToolKind_Selection && (box->flags&IK_BoxFlag_Dragable))
  {
    Rng2F32 camera_rect = camera->rect;

    /////////////////////////////////
    // world pos to screen

    Rng2F32 box_rect = ik_rect_from_box(box);
    Vec4F32 p0 = {box_rect.x0, box_rect.y0, 0, 1.0};
    Vec4F32 p1 = {box_rect.x1, box_rect.y1, 0, 1.0};
    p0 = transform_4x4f32(ik_state->proj_mat, p0);
    p1 = transform_4x4f32(ik_state->proj_mat, p1);
    // NOTE(k): ui position is floored, so we add it by 1, otherwise it will sometimes be off by 1 pixel
    p0.x = (p0.x*0.5+0.5) * ik_state->window_dim.x + 1;
    p0.y = (p0.y*0.5+0.5) * ik_state->window_dim.y + 1;
    p1.x = (p1.x*0.5+0.5) * ik_state->window_dim.x + 1;
    p1.y = (p1.y*0.5+0.5) * ik_state->window_dim.y + 1;

    Rng2F32 rect = r2f32p(p0.x, p0.y, p1.x, p1.y);
    Vec2F32 center = center_2f32(rect);
    F32 padding_px =  ui_top_font_size()*0.5;
    rect = pad_2f32(rect, padding_px);
    Vec2F32 rect_dim = dim_2f32(rect);

    UI_Box *container;
    UI_Rect(rect)
      UI_Flags(UI_BoxFlag_DrawBackground)
      UI_Palette(ui_build_palette(ui_top_palette(), .background = v4f32(0,0,0,0.15)))
      UI_Squish(1.0-box->focus_hot_t)
      container = ui_build_box_from_stringf(0, "###selection_box");

    B32 is_hot = contains_2f32(rect, ui_state->mouse);
    B32 is_active = ik_key_match(ik_state->active_box_key[IK_MouseButtonKind_Left], box->key);
    F32 hot_t = ui_anim(ui_key_from_stringf(ui_key_zero(), "selection_hot_t"), is_hot, .reset = 0, .rate = ik_state->animation.slow_rate);
    F32 active_t = ui_anim(ui_key_from_stringf(ui_key_zero(), "selection_active_t"), is_active, .reset = 0, .rate = ik_state->animation.slow_rate);

    Vec4F32 base_clr = ik_rgba_from_theme_color(IK_ThemeColor_BaseBorder);
    Vec4F32 hot_clr = ik_rgba_from_theme_color(IK_ThemeColor_MenuBarBorder);
    Vec4F32 clr = mix_4f32(base_clr, hot_clr, Max(active_t, hot_t));
    clr.w = 1.0;
    Vec4F32 background_clr = ik_rgba_from_theme_color(IK_ThemeColor_BaseBackground);

    UI_Parent(container)
      UI_Palette(ui_build_palette(ui_top_palette(), .border = clr, .background = background_clr))
    {
      /////////////////////////////////
      //~ Corners

      F32 dpi_scale = ik_state->dpi/96.f;
      F32 corner_thickness = dpi_scale*2.25;
      F32 edge_thickness = corner_thickness;
      F32 edge_length = dpi_scale*14;
      // top left
      {
        // anchor
        UI_Signal sig1;
        {
          Rng2F32 rect = {0};
          rect = pad_2f32(rect, corner_thickness);
          UI_Box *b;
          UI_Rect(rect)
            UI_Flags(UI_BoxFlag_DrawSideLeft|UI_BoxFlag_DrawSideTop|UI_BoxFlag_MouseClickable|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawActiveEffects)
            UI_HoverCursor(OS_Cursor_UpLeft)
            b = ui_build_box_from_stringf(0, "###top_left -> anchor");
          b->hot_t += hot_t;
          b->active_t += active_t;
          sig1 = ui_signal_from_box(b);
        }

        // => right edge
        UI_Signal sig2;
        {
          Rng2F32 rect = {corner_thickness, -edge_thickness, corner_thickness+edge_length, edge_thickness};
          UI_Box *b;
          UI_Rect(rect)
            UI_Flags(UI_BoxFlag_MouseClickable|UI_BoxFlag_DrawSideTop|UI_BoxFlag_DrawSideBottom|UI_BoxFlag_DrawSideRight|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawActiveEffects)
            UI_HoverCursor(OS_Cursor_UpLeft)
            b = ui_build_box_from_stringf(0, "###top_left -> right");
          b->hot_t += hot_t;
          b->active_t += active_t;
          sig2 = ui_signal_from_box(b);
        }

        // => down
        UI_Signal sig3;
        {
          Rng2F32 rect = {-edge_thickness, corner_thickness, edge_thickness, corner_thickness+edge_length};
          UI_Box *b;
          UI_Rect(rect)
            UI_Flags(UI_BoxFlag_MouseClickable|UI_BoxFlag_DrawSideLeft|UI_BoxFlag_DrawSideBottom|UI_BoxFlag_DrawSideRight|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawActiveEffects)
            UI_HoverCursor(OS_Cursor_UpLeft)
            b = ui_build_box_from_stringf(0, "##top_left -> down");
          b->hot_t += hot_t;
          b->active_t += active_t;
          sig3 = ui_signal_from_box(b);
        }

        // dragging
        UI_SignalFlags f = sig1.f|sig2.f|sig3.f;
        {
          if(f&UI_SignalFlag_LeftDragging || (is_active && ik_state->action_slot == IK_ActionSlot_TopLeft))
          {
            if(f&UI_SignalFlag_LeftPressed)
            {
              IK_BoxDrag drag =
              {
                .drag_start_rect = ik_rect_from_box(box),
                .drag_start_mouse = ik_state->mouse,
                .drag_start_mouse_in_world = ik_state->mouse_in_world,
              };
              ui_store_drag_struct(&drag);
              ik_state->hot_box_key = box->key;
              ik_state->action_slot = IK_ActionSlot_TopLeft;
              ik_state->active_box_key[IK_MouseButtonKind_Left] = box->key;
            }
            else
            {
              IK_BoxDrag drag = *ui_get_drag_struct(IK_BoxDrag);
              Vec2F32 delta = sub_2f32(ik_state->mouse_in_world, drag.drag_start_mouse_in_world);
              Rng2F32 old_rect = drag.drag_start_rect;
              Rng2F32 new_rect = old_rect;
              new_rect.p0 = add_2f32(new_rect.p0, delta);

              new_rect.x1 = ClampBot(old_rect.x0+1, new_rect.x1);
              new_rect.y1 = ClampBot(old_rect.y0+1, new_rect.y1);

              Vec2F32 old_rect_size = dim_2f32(old_rect);
              Vec2F32 new_rect_size = dim_2f32(new_rect);

              B32 keep_ratio = !!(box->flags&(IK_BoxFlag_FixedRatio|IK_BoxFlag_FitChildren|IK_BoxFlag_FitText));
              if(keep_ratio)
              {
                F32 area = new_rect_size.x*new_rect_size.y;
                new_rect_size.y = sqrt_f32(area/box->ratio);
                new_rect_size.x = new_rect_size.y*box->ratio;
                new_rect.x1 = new_rect.x0 + new_rect_size.x;
                new_rect.y1 = new_rect.y0 + new_rect_size.y;
              }

              Vec2F32 box_rect_dim = box->rect_size;
              Vec2F32 scale_delta = v2f32(new_rect_size.x/box_rect_dim.x, new_rect_size.y/box_rect_dim.y);

              if(scale_delta.x > 0 && scale_delta.y > 0)
              {
                ik_box_do_scale(box, scale_delta, box->position);
              }
            }
          }

          if(f&UI_SignalFlag_LeftReleased)
          {
            ik_kill_action();
          }
        }
      }

      // bottom right
      {
        Vec2F32 origin = {rect_dim.x, rect_dim.y};
        // anchor
        UI_Signal sig1;
        {
          Rng2F32 rect = {.p0 = origin, .p1 = origin};
          rect = pad_2f32(rect, corner_thickness);
          UI_Box *b;
          UI_Rect(rect)
            UI_Flags(UI_BoxFlag_DrawSideBottom|UI_BoxFlag_DrawSideRight|UI_BoxFlag_MouseClickable|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawActiveEffects)
            UI_HoverCursor(OS_Cursor_DownRight)
            b = ui_build_box_from_stringf(0, "###bottom_right -> anchor");
          b->hot_t += hot_t;
          b->active_t += active_t;
          sig1 = ui_signal_from_box(b);
        }

        // => left
        UI_Signal sig2;
        {
          Rng2F32 rect =
          {
            origin.x-corner_thickness-edge_length,
            origin.y-edge_thickness,
            origin.x-corner_thickness,
            origin.y+edge_thickness,
          };
          UI_Box *b;
          UI_Rect(rect)
            UI_Flags(UI_BoxFlag_DrawSideLeft|UI_BoxFlag_DrawSideBottom|UI_BoxFlag_DrawSideTop|UI_BoxFlag_MouseClickable|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawActiveEffects)
            UI_HoverCursor(OS_Cursor_DownRight)
            b = ui_build_box_from_stringf(0, "##bottom_right -> left");
          b->hot_t += hot_t;
          b->active_t += active_t;
          sig2 = ui_signal_from_box(b);
        }

        // => up
        UI_Signal sig3;
        {
          Rng2F32 rect =
          {
            origin.x-edge_thickness,
            origin.y-corner_thickness-edge_length,
            origin.x+edge_thickness,
            origin.y-corner_thickness,
          };
          UI_Box *b;
          UI_Rect(rect)
            UI_Flags(UI_BoxFlag_DrawSideTop|UI_BoxFlag_DrawSideLeft|UI_BoxFlag_DrawSideRight|UI_BoxFlag_MouseClickable|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawActiveEffects)
            UI_HoverCursor(OS_Cursor_DownRight)
            b = ui_build_box_from_stringf(0, "##bottom_right -> up");
          b->hot_t += hot_t;
          b->active_t += active_t;
          sig3 = ui_signal_from_box(b);
        }

        // dragging
        UI_SignalFlags f = sig1.f|sig2.f|sig3.f;
        {
          if(f&UI_SignalFlag_LeftDragging || (is_active && ik_state->action_slot == IK_ActionSlot_DownRight))
          { 
            if(f&UI_SignalFlag_LeftPressed)
            {
              IK_BoxDrag drag =
              {
                .drag_start_rect = ik_rect_from_box(box),
                .drag_start_mouse = ik_state->mouse,
                .drag_start_mouse_in_world = ik_state->mouse_in_world,
              };
              ui_store_drag_struct(&drag);
              ik_state->hot_box_key = box->key;
              ik_state->action_slot = IK_ActionSlot_DownRight;
              ik_state->active_box_key[IK_MouseButtonKind_Left] = box->key;
            }
            else
            {
              IK_BoxDrag drag = *ui_get_drag_struct(IK_BoxDrag);

              Vec2F32 delta = sub_2f32(ik_state->mouse_in_world, drag.drag_start_mouse_in_world);
              Rng2F32 old_rect = drag.drag_start_rect;
              Rng2F32 new_rect = old_rect;
              new_rect.p1 = add_2f32(new_rect.p1, delta);

              new_rect.x1 = ClampBot(old_rect.x0+1, new_rect.x1);
              new_rect.y1 = ClampBot(old_rect.y0+1, new_rect.y1);

              Vec2F32 old_rect_size = dim_2f32(old_rect);
              Vec2F32 new_rect_size = dim_2f32(new_rect);

              B32 keep_ratio = !!(box->flags&(IK_BoxFlag_FixedRatio|IK_BoxFlag_FitChildren));
              if(keep_ratio)
              {
                F32 area = new_rect_size.x*new_rect_size.y;
                new_rect_size.y = sqrt_f32(area/box->ratio);
                new_rect_size.x = new_rect_size.y*box->ratio;
                new_rect.x1 = new_rect.x0 + new_rect_size.x;
                new_rect.y1 = new_rect.y0 + new_rect_size.y;
              }

              Vec2F32 box_rect_dim = box->rect_size;
              Vec2F32 scale_delta = v2f32(new_rect_size.x/box_rect_dim.x, new_rect_size.y/box_rect_dim.y);

              if(scale_delta.x > 0 && scale_delta.y > 0)
              {
                ik_box_do_scale(box, scale_delta, box->position);
              }
            }
          }
          if(f&UI_SignalFlag_LeftReleased)
          {
            ik_kill_action();
          }
        }
      }

    }

    /////////////////////////////////
    //~ Borders

    UI_Rect(rect)
      UI_Flags(UI_BoxFlag_DrawBorder)
      UI_CornerRadius(4.f)
      ui_build_box_from_stringf(0, "###border");

    /////////////////////////////////
    //~ Drag Point

    if(box->flags&IK_BoxFlag_DragPoint)
    {
      Vec2F32 last_pos = {0};
      for(IK_Point *p = box->first_point; p != 0; p = p->next)
      {
        Vec2F32 pos = p->position;
        pos = ik_screen_pos_from_world(pos);

        F32 r = ui_top_font_size()*0.5;
        Rng2F32 rect = {.p0 = pos, .p1 = pos};
        rect = pad_2f32(rect, r);

        UI_Rect(rect)
          UI_CornerRadius(r)
          UI_Palette(ui_build_palette(ui_top_palette(), .background = v4f32(0.1,0.1,0.9,0.5)))
          UI_Flags(UI_BoxFlag_MouseClickable|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawDropShadow|UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawActiveEffects)
        {
          UI_Box *b = ui_build_box_from_stringf(0, "###%p", p);
          UI_Signal sig = ui_signal_from_box(b);

          if(p->prev)
          {

            UI_LineDrawData *draw_data = push_array(ui_build_arena(), UI_LineDrawData, 1);
            draw_data->a = last_pos;
            draw_data->b = pos;
            draw_data->stroke_size = 2.f;
            draw_data->stroke_color = v4f32(0.1,0.1,0.8, 0.25);
            ui_box_equip_custom_draw(b, ui_line_draw, draw_data);
          }

          if(ui_dragging(sig))
          {
            if(ui_pressed(sig))
            {
              ui_store_drag_struct(&p->position);
            }
            else
            {
              Vec2F32 drag_start_pos = *ui_get_drag_struct(Vec2F32);
              Vec2F32 delta = sub_2f32(ui_mouse(), ui_state->drag_start_mouse);
              delta.x *= ik_state->world_to_screen_ratio.x;
              delta.y *= ik_state->world_to_screen_ratio.y;
              p->position.x = drag_start_pos.x + delta.x;
              p->position.y = drag_start_pos.y + delta.y;
            }
          }
        }
        last_pos = pos;
      }
    }
  }
  ProfEnd();
}

internal void
ik_ui_inspector(void)
{
  ProfBeginFunction();
  IK_Box *b = ik_box_from_key(ik_state->focus_hot_box_key[IK_MouseButtonKind_Left]);

  B32 open = b != 0 || ik_state->tool == IK_ToolKind_Draw;
  F32 open_t = ui_anim(ui_key_from_stringf(ui_key_zero(), "inspector_open_t"), open, .reset = 0, .rate = ik_state->animation.fast_rate);
  IK_Frame *frame = ik_top_frame();

  if(open_t > 1e-3)
  {
    UI_Box *container;
    F32 padding_left = ui_top_font_size()*0.5;
    F32 width_px = ui_top_font_size() * 24;
    Rng2F32 rect = {padding_left, 0, padding_left+width_px, ik_state->window_dim.y};
    UI_Rect(rect)
      container = ui_build_box_from_stringf(0, "###inspector_container");

    UI_Box *body;
    UI_Parent(container)
      UI_WidthFill
      UI_HeightFill
      UI_Column
      UI_Padding(ui_pct(1.0, 0.0))
      UI_PrefHeight(ui_children_sum(1.0))
      UI_Transparency(0.2)
      UI_ChildLayoutAxis(Axis2_Y)
      UI_Flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawDropShadow|UI_BoxFlag_MouseClickable)
      UI_CornerRadius(4.0)
      UI_Squish(1.f-open_t)
      body = ui_build_box_from_stringf(0, "###body");

    UI_Box *padded;
    UI_Parent(body)
      UI_WidthFill
      UI_PrefHeight(ui_children_sum(0.0))
      UI_Row
      UI_Padding(ui_em(0.4, 0.0))
      UI_Column
      UI_Padding(ui_em(0.4, 0.0))
      UI_ChildLayoutAxis(Axis2_Y)
      padded = ui_build_box_from_stringf(0, "###padded");

    UI_Parent(padded)
    {
      ////////////////////////////////
      //~ Stacks

      IK_Cfg *cfg = &frame->cfg;

      B32 show_cfg = ik_state->tool == IK_ToolKind_Draw;
      if(show_cfg)
      {
        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("stroke_size");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ik_ui_slider_f32(str8_lit("stroke_size_val"), &cfg->stroke_size, 0.1);
        }

        ui_spacer(ui_em(0.2, 0.0));

        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("stroke_color");
          ui_spacer(ui_pct(1.0, 0.0));
          ik_ui_color_palette(str8_lit("stroke_clr"), stroke_colors, 3, &cfg->stroke_color);
        }
      }

      ////////////////////////////////
      //~ Focus Hot Box

      F32 focus_hot_t = b ? b->focus_hot_t : 0.f;

      UI_Box *box_info_container;
      UI_PrefHeight(ui_children_sum(0.0))
        UI_WidthFill
        UI_ChildLayoutAxis(Axis2_Y)
        UI_Flags((show_cfg&&b) ? UI_BoxFlag_DrawSideLeft|UI_BoxFlag_DrawSideTop : 0)
        box_info_container = ui_build_box_from_stringf(0, "box_info");

      if(b) UI_Parent(box_info_container)
      {
        ////////////////////////////////
        //~ Basic

        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("name");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("%S", b->name);
        }

        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("key");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("%I64u", b->key.u64[0]);
        }

        UI_WidthFill
        ui_divider(ui_em(0.5, 0.0));

        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("position");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("%.2f %.2f", b->position.x, b->position.y);
        }

        ui_spacer(ui_em(0.2, 0.0));

        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("size");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("%.2f %.2f", b->rect_size.x, b->rect_size.y);
        }

        ui_spacer(ui_em(0.2, 0.0));

        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("ratio");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("%.2f", b->ratio);
        }

        ui_spacer(ui_em(0.2, 0.0));

        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("stroke_size");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ik_ui_slider_f32(str8_lit("stroke_size_val"), &b->stroke_size, 0.1);
        }

        ui_spacer(ui_em(0.2, 0.0));

        if(!(b->flags&IK_BoxFlag_Orphan))
        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("layer");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_Row
          {
            ui_spacer(ui_pct(1.0, 0.0));
            UI_Font(ik_font_from_slot(IK_FontSlot_Icons))
            UI_PrefWidth(ui_text_dim(1, 1.0))
            if(ui_clicked(ik_ui_buttonf("+")))
            {
              IK_Box *next = b->next;
              if(next)
              {
                DLLRemove(frame->box_list.first, frame->box_list.last, b);
                DLLInsert(frame->box_list.first, frame->box_list.last, next, b);
              }
            }

            ui_spacer(ui_em(0.1, 0.0));
            UI_PrefWidth(ui_text_dim(1, 1.0))
            UI_Font(ik_font_from_slot(IK_FontSlot_Icons))
            if(ui_clicked(ik_ui_buttonf("-")))
            {
              IK_Box *prev = b;
              for(U64 i = 0; i < 2 && prev != 0; i++, prev = prev->prev);
              if(prev)
              {
                DLLRemove(frame->box_list.first, frame->box_list.last, b);
                DLLInsert(frame->box_list.first, frame->box_list.last, prev, b);
              }
            }
          }
        }

        UI_WidthFill
        ui_divider(ui_em(0.5, 0.0));

        ////////////////////////////////
        //~ Drawing

        UI_WidthFill
        UI_Row
        {
          IK_BoxFlags flag = IK_BoxFlag_DrawBorder;
          B32 on = b->flags&flag;
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("draw border");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_top_pref_height()) if(ui_clicked(ik_ui_checkbox(str8_lit("draw_border"), on)))
          {
            b->flags ^= flag;
          }
        }

        ui_spacer(ui_em(0.2, 0.0));

        UI_WidthFill
        UI_Row
        {
          IK_BoxFlags flag = IK_BoxFlag_DrawBackground;
          B32 on = b->flags&flag;
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("draw background");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_top_pref_height()) if(ui_clicked(ik_ui_checkbox(str8_lit("draw_background"), on)))
          {
            b->flags ^= flag;
          }
        }

        ui_spacer(ui_em(0.2, 0.0));

        UI_WidthFill
        UI_Row
        {
          IK_BoxFlags flag = IK_BoxFlag_DrawDropShadow;
          B32 on = b->flags&flag;
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("draw drop shadow");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_top_pref_height()) if(ui_clicked(ik_ui_checkbox(str8_lit("draw_dropshadow"), on)))
          {
            b->flags ^= flag;
          }
        }

        ui_spacer(ui_em(0.2, 0.0));

        UI_WidthFill
        UI_Row
        {
          IK_BoxFlags flag = IK_BoxFlag_DrawHotEffects;
          B32 on = b->flags&flag;
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("draw hot effects");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_top_pref_height()) if(ui_clicked(ik_ui_checkbox(str8_lit("draw_hot_effects"), on)))
          {
            b->flags ^= flag;
          }
        }

        ui_spacer(ui_em(0.2, 0.0));

        UI_WidthFill
        UI_Row
        {
          IK_BoxFlags flag = IK_BoxFlag_DrawActiveEffects;
          B32 on = b->flags&flag;
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("draw active effects");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_top_pref_height()) if(ui_clicked(ik_ui_checkbox(str8_lit("draw_active_effects"), on)))
          {
            b->flags ^= flag;
          }
        }

        ////////////////////////////////
        //~ Color

        UI_WidthFill
        ui_divider(ui_em(0.5, 0.0));

        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("background_color");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_em(3.0, 1.0))
            UI_Column
            UI_Padding(ui_em(0.1, 1.0))
            UI_CornerRadius(1.f)
            UI_Flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawDropShadow)
            UI_Palette(ui_build_palette(ui_top_palette(), .background = linear_from_srgba(b->background_color)))
            UI_PrefWidth(ui_pct(1.0, 0.0))
            UI_PrefHeight(ui_pct(1.0, 0.0))
            ui_build_box_from_stringf(0, "background_clr");
        }

        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("text_color");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_em(3.0, 1.0))
            UI_Column
            UI_Padding(ui_em(0.1, 1.0))
            UI_CornerRadius(1.f)
            UI_Flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawDropShadow)
            UI_Palette(ui_build_palette(ui_top_palette(), .background = linear_from_srgba(b->text_color)))
            UI_PrefWidth(ui_pct(1.0, 0.0))
            UI_PrefHeight(ui_pct(1.0, 0.0))
            ui_build_box_from_stringf(0, "text_clr");
        }

        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("border_color");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_PrefWidth(ui_em(3.0, 1.0))
            UI_Column
            UI_Padding(ui_em(0.1, 1.0))
            UI_CornerRadius(1.f)
            UI_Flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawDropShadow)
            UI_Palette(ui_build_palette(ui_top_palette(), .background = linear_from_srgba(b->border_color)))
            UI_PrefWidth(ui_pct(1.0, 0.0))
            UI_PrefHeight(ui_pct(1.0, 0.0))
            ui_build_box_from_stringf(0, "border_clr");
        }

        ////////////////////////////////
        //~ Text 

        if(b->flags & IK_BoxFlag_DrawText)
        {
          UI_WidthFill
          ui_divider(ui_em(0.5, 0.0));

          UI_WidthFill
          UI_Row
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("string_size");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("%I64u", b->string.size);
          }

          ui_spacer(ui_em(0.2, 0.0));

          UI_WidthFill
          UI_NamedRow(str8_lit("font_size"))
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("font_size");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ik_ui_slider_f32(str8_lit("font_size_val"), &b->font_size, 0.1);
          }

          ui_spacer(ui_em(0.2, 0.0));

          UI_WidthFill
          UI_NamedRow(str8_lit("font_slot"))
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("font");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_children_sum(1.0))
            UI_Row
            {
              IK_FontSlot slots[] = {IK_FontSlot_Code, IK_FontSlot_HandWrite1, IK_FontSlot_HandWrite2};
              char *displays[3] = {"code", "hand1", "hand2"};

              UI_PrefWidth(ui_text_dim(1, 1.0))
              for(U64 i = 0; i < ArrayCount(slots); i++)
              {
                char *display = displays[i];
                IK_FontSlot font_slot = slots[i];
                UI_BoxFlags flags = UI_BoxFlag_MouseClickable|UI_BoxFlag_DrawText|UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawActiveEffects;
                if(b->font_slot == font_slot)
                {
                  flags |= UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawDropShadow;
                }

                UI_Signal sig;
                UI_Flags(flags)
                UI_Palette(ui_build_palette(ui_top_palette(), .background = ik_rgba_from_theme_color(IK_ThemeColor_Breakpoint)))
                sig = ui_signal_from_box(ui_build_box_from_string(0, str8_cstring(display)));

                if(ui_clicked(sig))
                {
                  b->font_slot = font_slot;
                }
              }
            }
          }

          ui_spacer(ui_em(0.2, 0.0));

          UI_WidthFill
          UI_NamedRow(str8_lit("text_align"))
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("text_align");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_children_sum(1.0))
            UI_Row
            {
              UI_Font(ik_font_from_slot(IK_FontSlot_IconsExtra))
              UI_PrefWidth(ui_text_dim(1.0, 0.0))
              if(ui_clicked(ik_ui_buttonf("'")))
              {
                b->text_align |= IK_TextAlign_Left;
                b->text_align &= ~(IK_TextAlign_Right|IK_TextAlign_HCenter);
              }
              ui_spacer(ui_em(0.1,0.0));

              UI_Font(ik_font_from_slot(IK_FontSlot_IconsExtra))
              UI_PrefWidth(ui_text_dim(1.0, 0.0))
              if(ui_clicked(ik_ui_buttonf("(")))
              {
                b->text_align = IK_TextAlign_HCenter|IK_TextAlign_VCenter;
              }
              ui_spacer(ui_em(0.1,0.0));

              UI_Font(ik_font_from_slot(IK_FontSlot_IconsExtra))
              UI_PrefWidth(ui_text_dim(1.0, 0.0))
              if(ui_clicked(ik_ui_buttonf(")")))
              {
                b->text_align |= IK_TextAlign_Right;
                b->text_align &= ~(IK_TextAlign_Left|IK_TextAlign_HCenter);
              }
              ui_spacer(ui_em(0.1,0.0));

              UI_Font(ik_font_from_slot(IK_FontSlot_IconsExtra))
              UI_PrefWidth(ui_text_dim(1.0, 0.0))
              if(ui_clicked(ik_ui_buttonf("*")))
              {
                b->text_align |= IK_TextAlign_Top;
                b->text_align &= ~(IK_TextAlign_Bottom|IK_TextAlign_VCenter);
              }
              ui_spacer(ui_em(0.1,0.0));

              UI_Font(ik_font_from_slot(IK_FontSlot_IconsExtra))
              UI_PrefWidth(ui_text_dim(1.0, 0.0))
              if(ui_clicked(ik_ui_buttonf("+")))
              {
                b->text_align |= IK_TextAlign_Bottom;
                b->text_align &= ~(IK_TextAlign_Top|IK_TextAlign_VCenter);
              }
            }
          }

          ui_spacer(ui_em(0.2, 0.0));

          UI_WidthFill
          UI_Row
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("text_padding");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("%.2f", b->text_padding);
          }

          ui_spacer(ui_em(0.2, 0.0));

          UI_WidthFill
          UI_Row
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("cursor");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("%d %d", b->cursor.line, b->cursor.column);
          }

          ui_spacer(ui_em(0.2, 0.0));

          UI_WidthFill
          UI_Row
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("mark");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("%d %d", b->mark.line, b->mark.column);
          }
        }

        ////////////////////////////////
        //~ Image 

        if(b->flags & IK_BoxFlag_DrawImage && b->image)
        {
          UI_WidthFill
          ui_divider(ui_em(0.5, 0.0));

          UI_WidthFill
          UI_Row
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("image");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("%I64u %I64u", b->image->x, b->image->y);
          }

          UI_WidthFill
          UI_Row
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("kb");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("%.2f", (F32)b->image->encoded.size/(1024.f));
          }
        }

        ////////////////////////////////
        //~ Stroke

        if(b->flags & IK_BoxFlag_DrawStroke)
        {
          UI_WidthFill
          ui_divider(ui_em(0.5, 0.0));

          UI_WidthFill
          UI_Row
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("points");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("%I64u", b->point_count);
          }
        }

        ////////////////////////////////
        //~ Group

        if(b->group_children_count > 0)
        {
          UI_WidthFill
          ui_divider(ui_em(0.5, 0.0));

          // align
          UI_WidthFill
          UI_Row
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("align");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_Row
            {
              ui_spacer(ui_pct(1.0, 0.0));
              UI_PrefWidth(ui_em(1.0, 0.0))
              UI_Font(ik_font_from_slot(IK_FontSlot_Icons))
              UI_TextAlignment(UI_TextAlign_Center)
              // left
              if(ui_clicked(ik_ui_buttonf("<")))
              {
                for(IK_Box *child = b->group_first; child != 0; child = child->group_next)
                {
                  child->position.x = b->position.x;
                }
              }

              ui_spacer(ui_em(0.1, 0.0));
              UI_PrefWidth(ui_em(1.0, 0.0))
              UI_TextAlignment(UI_TextAlign_Center)
              UI_Font(ik_font_from_slot(IK_FontSlot_Icons))
              // top
              if(ui_clicked(ik_ui_buttonf("^")))
              {
                for(IK_Box *child = b->group_first; child != 0; child = child->group_next)
                {
                  child->position.y = b->position.y;
                }
              }

              ui_spacer(ui_em(0.1, 0.0));
              // UI_PrefWidth(ui_text_dim(1, 1.0))
              UI_PrefWidth(ui_em(1.0, 0.0))
              UI_TextAlignment(UI_TextAlign_Center)
              UI_Font(ik_font_from_slot(IK_FontSlot_Icons))
              // down
              if(ui_clicked(ik_ui_buttonf("v")))
              {
                for(IK_Box *child = b->group_first; child != 0; child = child->group_next)
                {
                  child->position.y = b->position.y + b->rect_size.y - child->rect_size.y;
                }
              }

              ui_spacer(ui_em(0.1, 0.0));
              UI_Font(ik_font_from_slot(IK_FontSlot_Icons))
              UI_PrefWidth(ui_em(1.0, 0.0))
              UI_TextAlignment(UI_TextAlign_Center)
              // right
              if(ui_clicked(ik_ui_buttonf(">")))
              {
                for(IK_Box *child = b->group_first; child != 0; child = child->group_next)
                {
                  child->position.x = b->position.x + b->rect_size.x - child->rect_size.x;
                }
              }
            }
          }

          // stretch
          UI_WidthFill
          UI_NamedRow(str8_lit("stretch"))
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("stretch");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_children_sum(1.0))
            UI_Row
            {
              // vertical
              ui_spacer(ui_em(0.1, 0.0));
              UI_Font(ik_font_from_slot(IK_FontSlot_IconsExtra))
              UI_PrefWidth(ui_text_dim(1.0, 0.0))
              UI_TextAlignment(UI_TextAlign_Center)
              if(ui_clicked(ik_ui_buttonf(",")))
              {
                F32 max_height = 0;
                for(IK_Box *child = b->group_first; child != 0; child = child->group_next)
                {
                  max_height = Max(max_height, child->rect_size.y);
                  ik_box_do_translate(child, v2f32(0, b->position.y-child->position.y));
                }

                for(IK_Box *child = b->group_first; child != 0; child = child->group_next)
                {
                  F32 scale = max_height/child->rect_size.y;
                  ik_box_do_scale(child, v2f32(scale,scale), b->position);
                }
              }

              // horizontal
              ui_spacer(ui_em(0.1, 0.0));
              UI_Font(ik_font_from_slot(IK_FontSlot_IconsExtra))
              UI_PrefWidth(ui_text_dim(1.0, 0.0))
              UI_TextAlignment(UI_TextAlign_Center)
              if(ui_clicked(ik_ui_buttonf("-")))
              {
                F32 max_width = 0;
                for(IK_Box *child = b->group_first; child != 0; child = child->group_next)
                {
                  max_width = Max(max_width, child->rect_size.x);
                  ik_box_do_translate(child, v2f32(b->position.x-child->position.x, 0));
                }

                for(IK_Box *child = b->group_first; child != 0; child = child->group_next)
                {
                  F32 scale = max_width/child->rect_size.x;
                  ik_box_do_scale(child, v2f32(scale,scale), b->position);
                }
              }
            }
          }

          // sort
          UI_WidthFill
          UI_NamedRow(str8_lit("sort"))
          {
            UI_PrefWidth(ui_text_dim(1, 1.0))
              ui_labelf("sort");
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_children_sum(1.0))
            UI_Row
            {
              // vertical
              ui_spacer(ui_em(0.1, 0.0));
              UI_Font(ik_font_from_slot(IK_FontSlot_IconsExtra))
              UI_PrefWidth(ui_text_dim(1.0, 0.0))
              UI_TextAlignment(UI_TextAlign_Center)
              if(ui_clicked(ik_ui_buttonf(",")))
              {
                F32 advance_y = 0;
                for(IK_Box *child = b->group_first; child != 0; child = child->group_next)
                {
                  F32 y = b->position.y+advance_y;
                  ik_box_do_translate(child, v2f32(0, y-child->position.y));
                  advance_y += child->rect_size.y;
                }
              }

              // horizontal
              ui_spacer(ui_em(0.1, 0.0));
              UI_Font(ik_font_from_slot(IK_FontSlot_IconsExtra))
              UI_PrefWidth(ui_text_dim(1.0, 0.0))
              UI_TextAlignment(UI_TextAlign_Center)
              if(ui_clicked(ik_ui_buttonf("-")))
              {
                F32 advance_x = 0;
                for(IK_Box *child = b->group_first; child != 0; child = child->group_next)
                {
                  F32 x = b->position.x+advance_x;
                  ik_box_do_translate(child, v2f32(x-child->position.x, 0));
                  advance_x += child->rect_size.x;
                }
              }
            }
          }
        }

        ////////////////////////////////
        //~ Actions

        UI_WidthFill
        ui_divider(ui_em(0.5, 0.0));

        UI_WidthFill
        UI_Row
        {
          UI_PrefWidth(ui_text_dim(1, 1.0))
            ui_labelf("actions");
          ui_spacer(ui_pct(1.0, 0.0));
          UI_Row
          {
            // copy
            ui_spacer(ui_pct(1.0, 0.0));
            UI_PrefWidth(ui_text_dim(0.f, 0.f))
            UI_Font(ik_font_from_slot(IK_FontSlot_Icons))
            UI_TextAlignment(UI_TextAlign_Center)
            if(ui_clicked(ik_ui_buttonf("F")))
            {
              ik_message_push(str8_lit("TODO"));
            }

            // delete
            ui_spacer(ui_em(0.2, 1.0));
            UI_PrefWidth(ui_text_dim(0.f, 0.f))
            UI_TextAlignment(UI_TextAlign_Center)
            UI_Font(ik_font_from_slot(IK_FontSlot_Icons))
            if(ui_clicked(ik_ui_buttonf("#")))
            {
              IK_Box *select = frame->select;
              if(b == select)
              {
                for(IK_Box *child = select->group_first, *next = 0; child != 0; child = next)
                {
                  next = child->group_next;
                  ik_box_release(child);
                }
                select->flags |= IK_BoxFlag_Disabled;
              }
              else ik_box_release(b);
            }
          }
        }
      }
    }
    ui_signal_from_box(body);
  }
  ProfEnd();
}

internal void
ik_ui_bottom_bar()
{
  IK_Frame *frame = ik_top_frame();
  UI_Box *container;

  F32 height = ui_top_font_size()*2.5;
  Rng2F32 rect = {0, ik_state->window_dim.y-height, ik_state->window_dim.x, ik_state->window_dim.y};
  rect = pad_2f32(rect, -ui_top_font_size()*0.5);
  UI_Rect(rect)
    UI_Flags(UI_BoxFlag_AllowOverflowY)
    container = ui_build_box_from_stringf(0, "bottom_bar_container");

  UI_Parent(container)
  {
    F32 zoom_level = round_f32(ik_state->world_to_screen_ratio.x*100);
    // camera zoom control
    UI_Transparency(0.3)
    UI_FontSize(ui_top_font_size()*1.35f)
    UI_PrefWidth(ui_text_dim(1, 1.0))
    if(ui_clicked(ik_ui_buttonf("%d%%", (int)(zoom_level))))
    {
      Vec2F32 p0 = frame->camera.target_rect.p0;

      Rng2F32 new_rect = frame->camera.target_rect;
      new_rect.p1.x = p0.x + ik_state->window_dim.x;
      new_rect.p1.y = p0.y + ik_state->window_dim.y;

      // move to previous center
      Vec2F32 old_center = center_2f32(frame->camera.target_rect);
      Vec2F32 delta = sub_2f32(old_center, center_2f32(new_rect));
      new_rect.p0 = add_2f32(new_rect.p0, delta);
      new_rect.p1 = add_2f32(new_rect.p1, delta);
      frame->camera.target_rect = new_rect;
    }

    ui_spacer(ui_pct(1.0, 0.0));

    // frame filepath info
    UI_PrefWidth(ui_text_dim(1.5, 1.0))
      UI_Flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawDropShadow|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawTextWeak)
      UI_CornerRadius(2.0)
      UI_Transparency(0.3)
      UI_FontSize(ui_top_font_size()*1.1f)
      ui_label(frame->save_path);
  }
}

internal void
ik_ui_notification(void)
{
  F32 width = ui_top_font_size()*20;
  F32 padding_right = ui_top_font_size()*1;
  F32 padding_vertical = ui_top_font_size()*1;
  Rng2F32 rect =
  {
    .x0 = ik_state->window_dim.x-width-padding_right,
    .y0 = padding_vertical,
    .x1 = ik_state->window_dim.x-padding_right,
    .y1 = ik_state->window_dim.y-padding_vertical,
  };

  UI_Box *container;
  UI_Rect(rect)
    UI_ChildLayoutAxis(Axis2_Y)
    container = ui_build_box_from_stringf(0, "###notification_container");

  UI_Parent(container)
  for(IK_Message *msg = ik_state->messages.first;
      msg != 0;
      msg = msg->next)
  {
    F32 t = msg->expired ? 1.f-msg->expired_t : msg->create_t;

    UI_Transparency(mix_1f32(1.0f, 0.1f, t))
    UI_PrefWidth(ui_pct(mix_1f32(0.f, 1.f, t), 0.0))
    UI_CornerRadius(1.f)
    UI_Palette(ui_build_palette(ui_top_palette(), .border = ik_rgba_from_theme_color(IK_ThemeColor_BaseBackground),
                                                  .text = v4f32(0,0,0,1.0),
                                                  .background = ik_rgba_from_theme_color(IK_ThemeColor_Breakpoint)))
    UI_Flags(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawDropShadow)
      ui_label(msg->string);

    if(msg->next) ui_spacer(ui_em(0.5,0.0));
  }
}

internal void
ik_ui_box_ctx_menu(void)
{
  IK_Key focus_hot_box_key = ik_state->focus_hot_box_key[IK_MouseButtonKind_Right];
  IK_Box *box = ik_box_from_key(focus_hot_box_key);
  B32 deleted = 0;
  B32 is_focus_active = ik_key_match(ik_state->focus_active_box_key, focus_hot_box_key);
  if(box && !(box->flags&IK_BoxFlag_OmitCtxMenu) && !is_focus_active)
  {
    UI_Key key = ui_key_from_stringf(ui_key_zero(), "box_ctx_menu_%I64u", box->key.u64[0]);
    UI_CtxMenu(key)
    {
      UI_Box *container;

      UI_PrefWidth(ui_children_sum(1.0))
      UI_PrefHeight(ui_children_sum(1.0))
        UI_ChildLayoutAxis(Axis2_Y)
        UI_Transparency(0.1)
        container = ui_build_box_from_key(0, ui_key_zero());

      UI_Parent(container)
        UI_TextPadding(ui_top_font_size()*0.5)
      {
        B32 taken = 0;
        if(ui_clicked(ui_buttonf("copy")))
        {
          ik_message_push(str8_lit("TODO"));
          taken = 1;
        }
        if(ui_clicked(ui_buttonf("paste")))
        {
          ik_message_push(str8_lit("TODO"));
          taken = 1;
        }
        if(ui_clicked(ui_buttonf("delete")))
        {
          deleted = 1;
          taken = 1;
        }
        if(box->flags&IK_BoxFlag_DrawImage && ui_clicked(ui_buttonf("export as png")))
        {
          String8 binary_path = os_get_process_info()->binary_path; // only directory
          String8 default_path = push_str8f(ik_frame_arena(), "%S/%I64u.png", binary_path, box->key.u64[0]);

          IK_Image *image = box->image;
          ik_image_to_png_file(image, default_path);
          ik_message_push(push_str8f(ik_frame_arena(), "%S saved", default_path));
          taken = 1;
        }

        if(taken)
        {
          ui_ctx_menu_close();
          ik_state->focus_hot_box_key[IK_MouseButtonKind_Right] = ik_key_zero();
        }
      }
    }

    // NOTE: other parts of ui could close the ctx menu
    B32 opened = ui_ctx_menu_is_open(key);
    B32 close_next_frame = ui_state->next_ctx_menu_open == 0;

    if(opened && close_next_frame)
    {
      ik_state->focus_hot_box_key[IK_MouseButtonKind_Right] = ik_key_zero();
    }
    else if(!opened)
    {
      ui_ctx_menu_open(key, ui_key_zero(), ui_state->mouse);
    }
    else if(opened && box->sig.f&IK_SignalFlag_RightPressed)
    {
      ui_ctx_menu_close();
    }

    if(deleted)
    {
      // TODO(Next): box could be select box, we don't handle deletion here
      //             just a hack for now, find a better way later
      box->deleted = 1;
    }
  }
}

internal void
ik_ui_g_ctx_menu()
{
  IK_Frame *frame = ik_top_frame();
  IK_Box *box = frame->blank;
  B32 is_open = ik_state->g_ctx_menu_open;

  UI_Key key = ui_key_from_stringf(ui_key_zero(), "box_g_ctx_menu");
  UI_CtxMenu(key)
  {
    UI_Box *container;

    UI_PrefWidth(ui_children_sum(1.0))
    UI_PrefHeight(ui_children_sum(1.0))
      UI_ChildLayoutAxis(Axis2_Y)
      UI_Transparency(0.1)
      container = ui_build_box_from_key(0, ui_key_zero());

    UI_Parent(container)
      UI_TextPadding(ui_top_font_size()*0.5)
    {
      B32 taken = 0;

      // crt
      {
        UI_Signal sig;
        ui_set_next_child_layout_axis(Axis2_X);
        ui_set_next_hover_cursor(OS_Cursor_HandPoint);
        UI_Box *b = ui_build_box_from_stringf(UI_BoxFlag_Clickable|
                                             UI_BoxFlag_DrawBackground|
                                             UI_BoxFlag_DrawBorder|
                                             UI_BoxFlag_DrawHotEffects|
                                             UI_BoxFlag_DrawActiveEffects,
                                             "###crt");
        UI_Parent(b)
        {
          UI_PrefWidth(ui_text_dim(0.0,1.0))
          ui_labelf("crt");
          ui_spacer(ui_pct(1.0,0.0));
          if(ik_state->crt_enabled)
          {
            UI_PrefWidth(ui_text_dim(0.0,1.0))
            UI_Font(ik_font_from_slot(IK_FontSlot_IconsExtra))
            ui_labelf("5");
          }
        }
        sig = ui_signal_from_box(b);

        if(ui_clicked(sig))
        {
          ik_state->crt_enabled = !ik_state->crt_enabled;
          taken = 1;
        }
      }

      // saving
      if(ui_clicked(ui_buttonf("save")))
      {
        ik_frame_to_tyml(frame);
        ik_message_push(push_str8f(ik_frame_arena(), "saved: %S", frame->save_path));
        taken = 1;
      }

      if(ui_clicked(ui_buttonf("export all images")))
      {
        U64 count = 0;
        for(U64 slot_idx = 0; slot_idx < ArrayCount(frame->image_cache_table); slot_idx++)
        {
          for(IK_ImageCacheNode *n = frame->image_cache_table[slot_idx].first; n != 0; n = n->next, count++)
          {
            IK_Image *image = &n->v;
            String8 binary_path = os_get_process_info()->binary_path; // only directory
            String8 default_path = push_str8f(ik_frame_arena(), "%S/%I64u.png", binary_path, image->key.u64[0]);
            ik_image_to_png_file(image, default_path);
          }
        }
        ik_message_push(push_str8f(ik_frame_arena(), "%I64u saved", count));
        taken = 1;
      }

      if(taken)
      {
        ui_ctx_menu_close();
        ik_state->g_ctx_menu_open = 0;
      }
    }
  }

  if((!is_open) && ui_ctx_menu_is_open(key))
  {
    ui_ctx_menu_close();
  }
  if((is_open) && ui_ctx_menu_is_open(key) && box->sig.f&IK_SignalFlag_RightPressed)
  {
    ui_ctx_menu_close();
  }
  if((is_open) && !ui_ctx_menu_is_open(key))
  {
    ui_ctx_menu_open(key, ui_key_zero(), ui_state->mouse);
  }
}

internal void
ik_ui_version()
{
  char *display = BUILD_TITLE_STRING_LITERAL;
  UI_FixedX(0) UI_FixedY(0)
    UI_Flags(UI_BoxFlag_DrawText)
    UI_PrefWidth(ui_text_dim(0.0,0.0))
    ui_build_box_from_stringf(0, "%s", BUILD_TITLE_STRING_LITERAL);
}

internal UI_Signal
ik_ui_checkbox(String8 key_string, B32 b)
{
  UI_Signal sig;

  UI_Box *container;
    UI_CornerRadius(1.0)
    UI_Flags(UI_BoxFlag_DrawBorder|
             UI_BoxFlag_DrawBackground|
             UI_BoxFlag_DrawDropShadow|
             UI_BoxFlag_MouseClickable|
             UI_BoxFlag_DrawHotEffects|
             UI_BoxFlag_DrawActiveEffects)
    UI_Palette(b ? ui_top_palette() : ui_state->widget_palette_info.scrollbar_palette)
    container = ui_build_box_from_string(0, key_string);

  if(b)
  {
    UI_Box *body;
    UI_Parent(container)
      UI_WidthFill
      UI_HeightFill
      UI_Column
      UI_Padding(ui_px(4,1.0))
      UI_Row
      UI_Padding(ui_px(4,1.0))
      UI_CornerRadius(1.0)
      UI_Flags(UI_BoxFlag_MouseClickable|
               UI_BoxFlag_DrawBorder|
               UI_BoxFlag_DrawBackground|
               UI_BoxFlag_DrawHotEffects|
               UI_BoxFlag_DrawActiveEffects|
               UI_BoxFlag_DrawDropShadow)
      UI_Palette(ui_state->widget_palette_info.scrollbar_palette)
      body = ui_build_box_from_stringf(0, "###body");
    sig = ui_signal_from_box(body);
  }
  else
  {
    sig = ui_signal_from_box(container);
  }

  return sig;
}

internal UI_Signal
ik_ui_button(String8 string)
{ 
  UI_Signal sig;
  UI_Size pref_width = ui_top_pref_width();
  F32 padding_px = 4.f;
  F32 font_size = ui_top_font_size() - padding_px;
  F32 corner_radius = 1.0;

  UI_Box *container;
  UI_CornerRadius(corner_radius)
    UI_Flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawDropShadow)
    UI_PrefWidth(ui_children_sum(1.0))
    container = ui_build_box_from_string(0, string);

  UI_Box *body;
  UI_Parent(container)
    UI_PrefWidth(ui_children_sum(1.0))
    UI_HeightFill
    UI_Column
    UI_Padding(ui_px(padding_px,1))
    UI_Row
    UI_Padding(ui_px(padding_px,1))
    UI_CornerRadius(corner_radius)
    UI_Flags(UI_BoxFlag_MouseClickable|
             UI_BoxFlag_DrawBorder|
             UI_BoxFlag_DrawBackground|
             UI_BoxFlag_DrawHotEffects|
             UI_BoxFlag_DrawText|
             UI_BoxFlag_DrawActiveEffects|
             UI_BoxFlag_DrawDropShadow)
    UI_Palette(ui_state->widget_palette_info.scrollbar_palette)
    UI_FontSize(font_size)
    UI_PrefWidth(pref_width)
    body = ui_build_box_from_string(0, string);

  sig = ui_signal_from_box(body);
  return sig;
}

internal UI_Signal
ik_ui_buttonf(char *fmt, ...)
{
  Temp scratch = scratch_begin(0,0);
  va_list args;
  va_start(args, fmt);
  String8 string = push_str8fv(scratch.arena, fmt, args);
  va_end(args);
  UI_Signal sig = ik_ui_button(string);
  scratch_end(scratch);
  return sig;
}

internal UI_Signal
ik_ui_slider_f32(String8 string, F32 *value, F32 px_to_val)
{
  UI_Box *container;

  UI_CornerRadius(1)
    UI_Flags(UI_BoxFlag_DrawBorder|UI_BoxFlag_Clickable|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawDropShadow|UI_BoxFlag_DrawText|UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawActiveEffects)
    UI_PrefWidth(ui_text_dim(ui_top_font_size(), 1.0))
    UI_TextAlignment(UI_TextAlign_Center)
    UI_HoverCursor(OS_Cursor_LeftRight)
    container = ui_build_box_from_stringf(0, "%.2f###%S", *value, string);

  UI_Signal sig = ui_signal_from_box(container);

  if(ui_dragging(sig))
  {
    if(ui_pressed(sig))
    {
      ui_store_drag_struct(value);
    }
    else
    {
      F32 drag_start_value = *ui_get_drag_struct(F32);
      Vec2F32 delta = ui_drag_delta();
      *value = drag_start_value + delta.x*px_to_val;
    }
  }
  return sig;
}

internal UI_Signal
ik_ui_range_slider_f32(String8 string, F32 *value, F32 max, F32 min)
{
  UI_Box *b;
  UI_HeightFill
    UI_Column
    UI_Center
    UI_Palette(ui_state->widget_palette_info.scrollbar_palette)
    UI_WidthFill
    UI_PrefHeight(ui_em(0.35,1.0))
    UI_CornerRadius(ui_top_font_size()*0.125)
    UI_Flags(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder)
    b = ui_build_box_from_string(0, string);
  UI_Signal sig = ui_signal_from_box(b);
  return sig;
}

internal UI_BOX_CUSTOM_DRAW(ik_ui_palette_current_draw)
{
  Rng2F32 dst = box->rect;
  // Vec4F32 src_clr = box->palette->background;
  // Vec4F32 src_hsv = hsva_from_rgba(src_clr);
  // Vec4F32 highlight_clr = src_hsv.z > 0.5 ? v4f32(0,0,0,1) : v4f32(1, 1, 1, 1);
  Vec4F32 highlight_clr = v4f32(0,0,0,0.9);
  dr_rect(dst, highlight_clr, 2, 4, 1.f);
}

internal void
ik_ui_color_palette(String8 string, Vec4F32 *colors, U64 color_count, Vec4F32 *current)
{
  UI_Box *container;
  UI_PrefWidth(ui_children_sum(1.0))
    UI_Flags(UI_BoxFlag_DrawBorder)
    UI_ChildLayoutAxis(Axis2_X)
    UI_CornerRadius(4.0)
    container = ui_build_box_from_string(0, string);

  UI_Parent(container)
  {
    for(U64 i = 0; i < color_count; i++)
    {
      Vec4F32 clr = colors[i];
      Vec4F32 clr_linear = linear_from_srgba(clr);
      UI_BoxFlags flags = UI_BoxFlag_DrawBackground|UI_BoxFlag_MouseClickable|UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawActiveEffects;
      B32 is_current = (clr.x == current->x && clr.y == current->y && clr.z == current->z && clr.w == current->w);
      if(is_current) flags |= UI_BoxFlag_DrawDropShadow;

      UI_Box *b;
      UI_PrefWidth(ui_top_pref_height())
        UI_CornerRadius(1.0)
        UI_Palette(ui_build_palette(ui_top_palette(), .background = clr_linear))
        UI_Flags(flags)
      b = ui_build_box_from_stringf(0, "%I64u", i);
      UI_Signal sig = ui_signal_from_box(b);
      if(ui_clicked(sig))
      {
        *current = clr;
      }

      if(is_current)
      {
        ui_box_equip_custom_draw(b, ik_ui_palette_current_draw, 0);
      }
      if(i != color_count-1)
      {
        ui_spacer(ui_em(0.1, 0.0));
      }
    }
  }
}

/////////////////////////////////
//~ Text Operation Functions

internal UI_TxtOp
ik_multi_line_txt_op_from_event(Arena *arena, UI_Event *event, String8 string, TxtPt cursor, TxtPt mark)
{
  TxtPt next_cursor = cursor;
  TxtPt next_mark = mark;
  TxtRng range = {0};
  String8 replace = {0};
  String8 copy = {0};
  UI_TxtOpFlags flags = 0;
  Vec2S32 delta = event->delta_2s32;

  // Resolve high-lelvel delta into byte delta, based on unit
  switch(event->delta_unit) 
  {
    default:{}break;
    case UI_EventDeltaUnit_Char:
    {
      // TODO(k): this should account for multi-byte characters in UTF-8... for now, just assume ASCII now
      // no-op
      U8 *first = string.str;
      U8 *opl = (string.str+string.size);
      U8 *curr = (string.str+cursor.column)-1;
      U8 *c = curr;
      S32 x_abs = abs(event->delta_2s32.x);
      c = event->delta_2s32.x > 0 ? utf8_step_forward(c, opl, x_abs) : utf8_step_backward(c, first, x_abs);
      delta.x = c - curr;
    }break;
    case UI_EventDeltaUnit_Word:
    {
      U8 *first = string.str;
      U8 *opl = (string.str+string.size);
      U8 *curr = (string.str+cursor.column)-1;
      U8 *c = curr;

      if(delta.x > 0)
      {
        if(char_is_space(*c))
        {
          // find next non-empty char
          for(; c < opl && char_is_space(*c); c++);
        }
        else
        {
          // find next empty space
          for(; c < opl && !char_is_space(*c); c++);
        }
      }
      else
      {
        if(c > first && char_is_space(*(c-1)))
        {
          // find prev non-empty char
          for(; c > first && char_is_space(*(c-1)); c--);
        }
        else
        {
          // find prev empty space
          for(; c > first && !char_is_space(*(c-1)); c--);
        }
      }
      delta.x = c-curr;
    }break;
    case UI_EventDeltaUnit_Line:
    {
      U8 *first = string.str;
      U8 *opl = (string.str+string.size);
      U8 *curr = (string.str+cursor.column)-1;
      U8 *c = curr;

      // skip to current line's head, find out current line's codepoint idx
      U64 line_codepoint_idx = 0;
      for(; c > first && *(c-1) != '\n';)
      {
        c = utf8_step_backward((U8*)c, first, 1);
        line_codepoint_idx++;
      }

      if(delta.y < 0)
      {
        // jump to last line end
        if(c > first) c--;

        // jump to last line head
        for(; c > first && *(c-1) != '\n'; c--);
    
        for(U64 i = 0; i < line_codepoint_idx && c < opl; i++)
        {
          c = utf8_step_forward((U8*)c, opl, 1);
          if(*c == '\n')
          {
            break;
          }
        }
        delta.x = c-curr;
      }
      else
      {
        for(U64 j = 0; j < delta.y && c < opl; j++)
        {
          // find next line head
          for(;c < opl;c++)
          {
            if(*c == '\n')
            {
              c++;
              break;
            }
          }
          for(U64 i = 0; i < line_codepoint_idx && c < opl; i++)
          {
            c = utf8_step_forward((U8*)c, opl, 1);
            if(*c == '\n')
            {
              break;
            }
          }
        }
        delta.x = c-curr;
      }
    }break;
    case UI_EventDeltaUnit_Whole:
    case UI_EventDeltaUnit_Page: 
    {
      U64 first_nonwhitespace_column = 1;
      for(U64 idx = 0; idx < string.size; idx++)
      {
        if(!char_is_space(string.str[idx]))
        {
          first_nonwhitespace_column = idx + 1;
          break;
        }
      }
      S64 home_dest_column = (first_nonwhitespace_column == cursor.column) ? 1 : first_nonwhitespace_column;
      delta.x = (delta.x > 0) ? ((S64)string.size+1 - cursor.column) : (home_dest_column - cursor.column);
    }break;
  }

  // Form next cursor
  if(txt_pt_match(cursor,mark))
  {
    next_cursor.column += delta.x;
  }

  if(!(event->flags & UI_EventFlag_KeepMark))
  {
    next_mark = next_cursor;
  }

  // Copying
  if(event->flags & UI_EventFlag_Copy)
  {
    copy = str8_substr(string, r1u64(cursor.column-1, mark.column-1));
    flags |= UI_TxtOpFlag_Copy;
  }

  // Pasting
  if(event->flags & UI_EventFlag_Paste)
  {
    range = txt_rng(cursor, mark);
    replace = os_get_clipboard_text(arena);
    next_cursor = next_mark = txt_pt(cursor.line, cursor.column+replace.size);
  }

  // Deletion
  if(event->flags & UI_EventFlag_Delete)
  {
    TxtPt new_pos = txt_pt_min(next_cursor, next_mark);
    range = txt_rng(next_cursor, next_mark);
    replace = str8_lit("");
    next_cursor = next_mark = new_pos;
  }

  // Insertion
  if(event->string.size != 0)
  {
    range = txt_rng(cursor, mark);
    replace = push_str8_copy(arena, event->string);
    next_cursor = next_mark = txt_pt(range.min.line, range.min.column+event->string.size);
  }

  //- rjf: determine if this event should be taken, based on bounds of cursor
  {
    if(next_cursor.column > string.size + 1 || 1 > next_cursor.column || event->delta_2s32.y != 0)
    {
      flags |= UI_TxtOpFlag_Invalid;
    }
    next_cursor.column = Clamp(1, next_cursor.column, string.size + replace.size + 1);
    next_mark.column   = Clamp(1, next_mark.column, string.size + replace.size + 1);
  }

  // Build+fill
  UI_TxtOp op = {0};
  op.flags = flags;
  op.replace = replace;
  op.copy = copy;
  op.range = range;
  op.cursor = next_cursor;
  op.mark = next_mark;
  return op;
}

internal Rng1U64
ik_replace_range_from_txtrng(String8 string, TxtRng txt_rng)
{
  char *first = (char*)string.str;
  char *opl = first+string.size;
  char *c = first;

  U64 min = 0;
  U64 max = 0;
  U64 line_index = 1;
  U64 column_index = 1;

  // skip to the min line
  for(; c < opl && line_index < txt_rng.min.line; c++)
  {
    if(*c == '\n')
    {
      line_index++;
      column_index = 1;
    }
    else
    {
      column_index++;
    }
  }

  // skip to the min column
  for(; c < opl && column_index < txt_rng.min.column; c++, column_index++)
  {
    AssertAlways(*c != '\n');
  }
  min = c - first;

  // skip to the last line
  for(; c < opl && line_index < txt_rng.max.line; c++)
  {
    if(*c == '\n')
    {
      line_index++;
      column_index = 1;
    }
    else
    {
      column_index++;
    }
  }

  // skip to the max column
  for(; c < opl && column_index < txt_rng.max.column; c++, column_index++)
  {
    AssertAlways(*c != '\n');
  }
  max = c - first;

  Rng1U64 ret = {min,max};
  return ret;
}

/////////////////////////////////
//~ Scene serialization/deserialization

//- frame 

internal String8
ik_frame_to_tyml(IK_Frame *frame)
{
  String8 ret = frame->save_path;
  Temp scratch = scratch_begin(0, 0);

  se_build_begin(scratch.arena);
  SE_Node *root = se_struct();

  U8 *blob = 0;
  U64 blob_size = 0;
  U64 blob_cap = 0;

  SE_Parent(root)
  {
    /////////////////////////////////
    // Image cache

    SE_Array_WithTag(str8_lit("images"))
    {
      for(U64 i = 0; i < ArrayCount(frame->image_cache_table); i++)
      {
        IK_ImageCacheSlot *slot = &frame->image_cache_table[i];
        for(IK_ImageCacheNode *n = slot->first; n != 0; n = n->next)
        {
          IK_Image *image = &n->v;
          if(image->rc > 0) SE_Struct()
          {
            se_v2u64_with_tag(str8_lit("key"), v2u64(image->key.u64[0], image->key.u64[1]));
            se_u64_with_tag(str8_lit("x"), image->x);
            se_u64_with_tag(str8_lit("y"), image->y);

            U64 head = blob_size;
            U64 tail = head+image->encoded.size;
            se_u64_with_tag(str8_lit("head"), head);
            se_u64_with_tag(str8_lit("tail"), tail);

            U64 size_required = blob_size+image->encoded.size;
            if(size_required > blob_cap)
            {
              U64 new_cap = size_required*2;
              U8 *next_blob = push_array(scratch.arena, U8, new_cap);
              MemoryCopy(next_blob, blob, blob_size);

              blob_cap = new_cap;
              blob = next_blob;
            }

            MemoryCopy(blob+blob_size, image->encoded.str, image->encoded.size);
            blob_size += image->encoded.size;
          }
        }
      }
    }

    /////////////////////////////////
    // Frame info

    // camera
    SE_Struct_WithTag(str8_lit("camera"))
    {
      IK_Camera camera = frame->camera;
      Vec4F32 rect = v4f32(camera.target_rect.x0, camera.target_rect.y0, camera.target_rect.x1, camera.target_rect.y1);
      se_v4f32_with_tag(str8_lit("rect"), rect);
    }

    /////////////////////////////////
    // Box

    SE_Array_WithTag(str8_lit("boxes"))
    {
      for(IK_Box *box = frame->box_list.first; box != 0; box = box->next)
      {
        IK_Box *group = box->group;

        SE_Struct()
        {
          /////////////////////////////////
          // Basic

          se_v2u64_with_tag(str8_lit("key"), v2u64(box->key.u64[0], box->key.u64[1]));
          if(group) se_v2u64_with_tag(str8_lit("group"), v2u64(group->key.u64[0], group->key.u64[1]));
          se_str_with_tag(str8_lit("name"), box->name);
          se_u64_with_tag(str8_lit("flags"), box->flags);
          se_v2f32_with_tag(str8_lit("position"), box->position);
          se_f32_with_tag(str8_lit("rotation"), box->rotation);
          se_v2f32_with_tag(str8_lit("rect_size"), box->rect_size);
          se_v4f32_with_tag(str8_lit("background_color"), box->background_color);
          se_v4f32_with_tag(str8_lit("text_color"), box->text_color);
          se_v4f32_with_tag(str8_lit("border_color"), box->border_color);
          se_v4f32_with_tag(str8_lit("stroke_color"), box->stroke_color);
          se_f32_with_tag(str8_lit("ratio"), box->ratio);
          se_u64_with_tag(str8_lit("hover_cursor"), box->hover_cursor);
          se_f32_with_tag(str8_lit("transparency"), box->transparency);
          se_f32_with_tag(str8_lit("stroke_size"), box->stroke_size);

          /////////////////////////////////
          // Image

          if(box->image)
          {
            se_v2u64_with_tag(str8_lit("image"), v2u64(box->image->key.u64[0], box->image->key.u64[1]));
          }

          /////////////////////////////////
          // Text

          if(box->string.size > 0) se_multiline_str_with_tag(str8_lit("string"), box->string);
          se_f32_with_tag(str8_lit("font_size"), box->font_size);
          se_u64_with_tag(str8_lit("font_slot"), box->font_slot);
          se_f32_with_tag(str8_lit("tab_size"), box->tab_size);
          se_u64_with_tag(str8_lit("text_raster_flags"), box->text_raster_flags);
          se_f32_with_tag(str8_lit("text_padding"), box->text_padding);
          se_u64_with_tag(str8_lit("text_align"), box->text_align);

          /////////////////////////////////
          // Points

          if(box->first_point != 0)
          {
            SE_Array_WithTag(str8_lit("points"))
            {
              for(IK_Point *p = box->first_point; p != 0; p = p->next)
              {
                se_v2f32(p->position);
              }
            }
          }
        }
      }
    }
  }
  se_build_end();
  String8List strs = se_yml_node_to_strlist(scratch.arena, root);

  /////////////////////////////////
  // Backup

  if(!frame->did_backup)
  {
    String8 backup = push_str8f(scratch.arena, "%S.bak", frame->save_path);
    os_copy_file_path(backup, frame->save_path);
    frame->did_backup = 1;
  }

  /////////////////////////////////
  // Write to file

  {
    U64 c = 0;
    OS_Handle file = os_file_open(OS_AccessFlag_Write, frame->save_path);
    Assert(!os_handle_match(file, os_handle_zero()));

    // write header
    String8 header = push_str8f(scratch.arena, "%I64u\n", blob_size);
    os_file_write(file, rng_1u64(c, c+header.size), header.str);
    c+=header.size;

    // write blob
    os_file_write(file, rng_1u64(c, c+blob_size), blob);
    c+=blob_size;
    // write a new line
    os_file_write(file, rng_1u64(c, c+1), "\n");
    c+=1;
    // write yml
    for(String8Node *n = strs.first; n != 0; n = n->next)
    {
      os_file_write(file, rng_1u64(c, c+n->string.size), n->string.str);
      c+=n->string.size;
    }
    os_file_close(file);
  }

  scratch_end(scratch);
  return ret;
}

internal IK_Frame *
ik_frame_from_tyml(String8 path)
{
  IK_Frame *frame = ik_frame_alloc();
  Arena *arena = frame->arena;
  Temp scratch = scratch_begin(0,0);
  ik_push_frame(frame);

  /////////////////////////////////
  //~ Read file

  OS_Handle f = os_file_open(OS_AccessFlag_Read, (path));
  FileProperties f_props = os_properties_from_file(f);
  U64 size = f_props.size;
  U8 *data = push_array(scratch.arena, U8, f_props.size);
  os_file_read(f, rng_1u64(0,size), data);
  os_file_close(f);

  /////////////////////////////////
  //~ Load blob

  U8 *c = data;
  U8 *opl = c+size;
  // read header
  for(; c < opl && *c != '\n'; c++);
  String8 header = str8_range(data, c);
  U64 blob_size = u64_from_str8(header, 10);
  c++;

  String8 blob = str8_range(c, c+blob_size);
  c+=blob_size;

  /////////////////////////////////
  //~ Load se node

  String8 yml_string = str8_range(c++, opl);
  SE_Node *se_node = se_yml_node_from_string(scratch.arena, yml_string);

  /////////////////////////////////
  //~ Camera

  SE_Node *camera_node = se_struct_from_tag(se_node, str8_lit("camera"));
  if(camera_node)
  {
    Vec4F32 src = se_v4f32_from_tag(camera_node, str8_lit("rect"));
    Rng2F32 rect = {src.x, src.y, src.z, src.w};
    Vec2F32 dim = dim_2f32(rect);
    dim.x = dim.y * (ik_state->window_dim.x/ik_state->window_dim.y);
    rect.x1 = rect.x0 + dim.x;
    frame->camera.target_rect = rect;
  }

  /////////////////////////////////
  //~ Load images

  SE_Node *first_image_node = se_arr_from_tag(se_node, str8_lit("images"));
  ProfScope("load images")
  {
    for(SE_Node *n = first_image_node; n != 0; n = n->next)
    {
      Vec2U64 key_src = se_v2u64_from_tag(n, str8_lit("key"));
      IK_Key key = ik_key_make(key_src.x, key_src.y);
      U64 head = se_u64_from_tag(n, str8_lit("head"));
      U64 tail = se_u64_from_tag(n, str8_lit("tail"));
      String8 image_blob = str8_range(blob.str+head, blob.str+tail);
      IK_Image *image = ik_image_push(key);
      image->encoded = ik_push_str8_copy(image_blob);
      ik_image_decode_queue_push(image);
    }
  }

  /////////////////////////////////
  //~ Build box

  SE_Node *first_box_node = se_arr_from_tag(se_node, str8_lit("boxes"));
  if(first_box_node && first_box_node->parent)
  {
    // NOTE(k): since we need to add group, and group box is always added after the group children, so we reverse the loading order
    for(SE_Node *n = first_box_node->parent->last; n != 0; n = n->prev)
    {
      // basic
      Vec2U64 key_src = se_v2u64_from_tag(n, str8_lit("key"));
      Vec2U64 group_key_src = se_v2u64_from_tag(n, str8_lit("group"));
      IK_Key key = ik_key_make(key_src.x, key_src.y);
      IK_Key group_key = ik_key_make(group_key_src.x, group_key_src.y);
      IK_Box *group = ik_box_from_key(group_key);
      U64 flags = se_u64_from_tag(n, str8_lit("flags"));

      // NOTE(k): push front
      IK_Box *box = ik_build_box_from_key_(flags, key, 0);

      if(group)
      {
        DLLPushFront_NP(group->group_first, group->group_last, box, group_next, group_prev);
        group->group_children_count++;
      }

      String8 name = se_str_from_tag(n, str8_lit("name"));
      ik_box_equip_name(box, name);

      // no info
      box->position = se_v2f32_from_tag(n, str8_lit("position"));
      box->rotation = se_f32_from_tag(n, str8_lit("rotation"));
      box->rect_size = se_v2f32_from_tag(n, str8_lit("rect_size"));
      box->background_color = se_v4f32_from_tag(n, str8_lit("background_color"));
      box->text_color = se_v4f32_from_tag(n, str8_lit("text_color"));
      box->border_color = se_v4f32_from_tag(n, str8_lit("border_color"));
      box->stroke_color = se_v4f32_from_tag(n, str8_lit("stroke_color"));
      box->ratio = se_f32_from_tag(n, str8_lit("ratio"));
      box->hover_cursor = se_u64_from_tag(n, str8_lit("hover_cursor"));
      box->transparency = se_f32_from_tag(n, str8_lit("transparency"));
      box->stroke_size = se_f32_from_tag(n, str8_lit("stroke_size"));

      // image
      Vec2U64 image_key = se_v2u64_from_tag(n, str8_lit("image"));
      if(image_key.x != 0)
      {
        IK_Key key = ik_key_make(image_key.x, image_key.y);
        IK_Image *image = ik_image_from_key(key);
        if(image)
        {
          box->image = image;
          image->rc++;
        }
      }

      // text
      SE_Node *first_string_node = se_arr_from_tag(n, str8_lit("string"));
      if(first_string_node)
      {
        String8List list = {0};
        for(SE_Node *n = first_string_node; n != 0; n = n->next)
        {
          if(n->kind == SE_NodeKind_String)
          {
            str8_list_push(scratch.arena, &list, n->v.se_str);
          }
        }
        StringJoin join = {str8_lit(""), str8_lit("\n"), str8_lit("")};
        String8 string_joined = str8_list_join(scratch.arena, &list, &join);
        box->string = ik_push_str8_copy(string_joined);
      }
      box->font_size = se_f32_from_tag(n, str8_lit("font_size"));
      box->font_slot = se_u64_from_tag(n, str8_lit("font_slot"));
      box->tab_size = se_f32_from_tag(n, str8_lit("tab_size"));
      box->text_raster_flags = se_u64_from_tag(n, str8_lit("text_raster_flags"));
      box->text_padding = se_f32_from_tag(n, str8_lit("text_padding"));
      box->text_align = se_u64_from_tag(n, str8_lit("text_align"));

      // points
      for(SE_Node *p = se_arr_from_tag(n, str8_lit("points")); p != 0 && p->kind == SE_NodeKind_Vec2F32; p = p->next)
      {
        IK_Point *point = ik_point_alloc();
        point->position = p->v.se_v2f32;
        DLLPushBack(box->first_point, box->last_point, point);
        box->point_count++;
      }
    }
  }

  ik_pop_frame();
  scratch_end(scratch);
  return frame;
}

//- image

internal void
ik_image_to_png_file(IK_Image *image, String8 path)
{
  // stbi_write_png((char const *)path.str, image->x, image->y, image->comp, (const void*)image->encoded.str, 0);
  if(image->encoded.size > 0)
  {
    os_write_data_to_file_path(path, image->encoded);
  }
}

/////////////////////////////////
//~ Helpers

// projection
internal Vec2F32
ik_screen_pos_in_world(Mat4x4F32 proj_view_mat_inv, Vec2F32 pos)
{
  // mouse ndc pos
  F32 mox_ndc = (pos.x / ik_state->window_dim.x) * 2.f - 1.f;
  F32 moy_ndc = (pos.y / ik_state->window_dim.y) * 2.f - 1.f;
  Vec4F32 pos_in_world_4 = transform_4x4f32(proj_view_mat_inv, v4f32(mox_ndc, moy_ndc, 1., 1.));
  Vec2F32 ret = v2f32(pos_in_world_4.x, pos_in_world_4.y);
  return ret;
}

internal Vec2F32
ik_screen_pos_from_world(Vec2F32 pos)
{
  Vec4F32 projected = transform_4x4f32(ik_state->proj_mat, v4f32(pos.x, pos.y, 0, 1.0));
  Vec2F32 ret = {(projected.x+1.f)/2.f, (projected.y+1.f)/2.f};
  ret.x = ret.x*ik_state->window_dim.x;
  ret.y = ret.y*ik_state->window_dim.y;
  return ret;
}

// encode/decode
internal String8
ik_b64string_from_bytes(Arena *arena, String8 src)
{
  ProfBeginFunction();
  // 3 bytes -> 4 chars, round up
  U64 enc_len = ((src.size+2)/3 * 4);
  U8 *out = push_array(arena, U8, enc_len+1); // +1 for null terminator

  U64 i = 0;
  U64 j = 0;
  while(i < src.size)
  {
    U32 octet_a = (i++) < src.size ? src.str[i-1] : 0;
    U32 octet_b = (i++) < src.size ? src.str[i-1] : 0;
    U32 octet_c = (i++) < src.size ? src.str[i-1] : 0;
    U32 triple = (octet_a<<16) | (octet_b<<8) | octet_c;

    out[j++] = base64[(triple>>18) & 0x3F];
    out[j++] = base64[(triple>>12) & 0x3F];
    out[j++] = (i - 1 > src.size) ? '=' : base64[(triple >> 6) & 0x3F];
    out[j++] = (i > src.size)     ? '=' : base64[triple & 0x3F];
  }
  out[j] = '\0';
  String8 ret = str8(out, enc_len);
  ProfEnd();
  return ret;
}

internal String8
ik_bytes_from_b64string(Arena *arena, String8 src)
{
  ProfBeginFunction();
  String8 ret = {0};
  U64 dec_size = (src.size/4)*3;
  U8 *out = push_array(arena, U8, dec_size);
  if(src.size > 0)
  {
    U64 i = 0;
    U64 j = 0;

    while(i < src.size)
    {
      U32 a = base64_reverse[src.str[i++]];
      U32 b = base64_reverse[src.str[i++]];
      U32 c = base64_reverse[src.str[i++]];
      U32 d = base64_reverse[src.str[i++]];

      U32 quadruple = (a<<18) | (b<<12) | (c<<6) | d;
      out[j++] = quadruple>>16;
      out[j++] = quadruple>>8;
      out[j++] = quadruple>>0;
    }

    if(src.str[src.size-1] == '=') j--;
    if(src.str[src.size-2] == '=') j--;
    ret = str8(out, j);
  }
  ProfEnd();
  return ret;
}

﻿/**
 * File:   vgcanvas.c
 * Author: Li XianJing <xianjimli@hotmail.com>
 * Brief:  vector graphics canvas base on nanovg
 *
 * Copyright (c) 2018 - 2018  Li XianJing <xianjimli@hotmail.com>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-04-14 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#define NANOVG_GL2_IMPLEMENTATION

#include "base/utf8.h"
#include "base/resource_manager.h"

#include "glad/glad.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#include "nanovg.h"
#include "nanovg_gl.h"
#include "base/mem.h"
#include "base/vgcanvas.h"
#define MAX_IMAGE_NR 256

typedef struct _vgcanvas_nanovg_t {
  vgcanvas_t base;

  int font_id;
  float_t ratio;
  NVGcontext* vg;
  uint32_t text_align_v;
  uint32_t text_align_h;
  SDL_Window* sdl_window;
  const uint8_t* images[MAX_IMAGE_NR];
} vgcanvas_nanovg_t;

ret_t vgcanvas_nanovg_begin_frame(vgcanvas_t* vgcanvas, rect_t* dirty_rect) {
  int ww = 0;
  int wh = 0;
  int fw = 0;
  int fh = 0;
  vgcanvas_nanovg_t* canvas = (vgcanvas_nanovg_t*)vgcanvas;
  SDL_Window* sdl_window = canvas->sdl_window;
  NVGcontext* vg = canvas->vg;

  SDL_GetWindowSize(sdl_window, &ww, &wh);
  SDL_GL_GetDrawableSize(sdl_window, &fw, &fh);
  canvas->ratio = (float)fw / (float)ww;

  glViewport(0, 0, fw, fh);
  glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  nvgBeginFrame(vg, ww, wh, canvas->ratio);

  return RET_OK;
}

ret_t vgcanvas_nanovg_end_frame(vgcanvas_t* vgcanvas) {
  vgcanvas_nanovg_t* canvas = (vgcanvas_nanovg_t*)vgcanvas;
  SDL_Window* sdl_window = canvas->sdl_window;
  NVGcontext* vg = canvas->vg;

  nvgEndFrame(vg);
  SDL_GL_SwapWindow(sdl_window);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_reset(vgcanvas_t* vgcanvas) {
  (void)vgcanvas;

  return RET_OK;
}

static ret_t vgcanvas_nanovg_flush(vgcanvas_t* vgcanvas) {
  (void)vgcanvas;

  return RET_OK;
}

static ret_t vgcanvas_nanovg_clear_rect(vgcanvas_t* vgcanvas, float_t x, float_t y, float_t w,
                                        float_t h, color_t c) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;
  color_t fill_color = vgcanvas->fill_color;
  vgcanvas_set_fill_color(vgcanvas, c);
  nvgBeginPath(vg);
  nvgRect(vg, x, y, w, h);
  nvgClosePath(vg);
  nvgFill(vg);
  nvgBeginPath(vg);
  vgcanvas_set_fill_color(vgcanvas, fill_color);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_begin_path(vgcanvas_t* vgcanvas) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgBeginPath(vg);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_move_to(vgcanvas_t* vgcanvas, float_t x, float_t y) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgMoveTo(vg, x, y);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_line_to(vgcanvas_t* vgcanvas, float_t x, float_t y) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgLineTo(vg, x, y);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_quadratic_curve_to(vgcanvas_t* vgcanvas, float_t cpx, float_t cpy,
                                                float_t x, float_t y) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgQuadTo(vg, cpx, cpy, x, y);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_bezier_curve_to(vgcanvas_t* vgcanvas, float_t cp1x, float_t cp1y,
                                             float_t cp2x, float_t cp2y, float_t x, float_t y) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgBezierTo(vg, cp1x, cp1y, cp2x, cp2y, x, y);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_arc_to(vgcanvas_t* vgcanvas, float_t x1, float_t y1, float_t x2,
                                    float_t y2, float_t r) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgArcTo(vg, x1, y1, x2, y2, r);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_arc(vgcanvas_t* vgcanvas, float_t x, float_t y, float_t r,
                                 float_t start, float_t end, bool_t ccw) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgArc(vg, x, y, r, start, end, ccw);

  return RET_OK;
}

static bool_t vgcanvas_nanovg_is_point_in_path(vgcanvas_t* vgcanvas, float_t x, float_t y) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;
  (void)vg;
  (void)x;
  (void)y;

  /*TODO*/

  return FALSE;
}

static ret_t vgcanvas_nanovg_rotate(vgcanvas_t* vgcanvas, float_t rad) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgRotate(vg, rad);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_scale(vgcanvas_t* vgcanvas, float_t x, float_t y) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgScale(vg, x, y);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_translate(vgcanvas_t* vgcanvas, float_t x, float_t y) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgTranslate(vg, x, y);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_transform(vgcanvas_t* vgcanvas, float_t a, float_t b, float_t c,
                                       float_t d, float_t e, float_t f) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgTransform(vg, a, b, c, d, e, f);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_transform(vgcanvas_t* vgcanvas, float_t a, float_t b, float_t c,
                                           float_t d, float_t e, float_t f) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgResetTransform(vg);
  nvgTransform(vg, a, b, c, d, e, f);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_round_rect(vgcanvas_t* vgcanvas, float_t x, float_t y, float_t w,
                                        float_t h, float_t r) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgRoundedRect(vg, x, y, w, h, r);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_ellipse(vgcanvas_t* vgcanvas, float_t x, float_t y, float_t rx,
                                     float_t ry) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgEllipse(vg, x, y, rx, ry);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_close_path(vgcanvas_t* vgcanvas) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgClosePath(vg);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_fill(vgcanvas_t* vgcanvas) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgFill(vg);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_clip_rect(vgcanvas_t* vgcanvas, float_t x, float_t y, float_t w,
                                       float_t h) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgScissor(vg, x, y, w, h);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_stroke(vgcanvas_t* vgcanvas) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgStroke(vg);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_font_size(vgcanvas_t* vgcanvas, float_t size) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;
  nvgFontSize(vg, size);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_font(vgcanvas_t* vgcanvas, const char* name) {
  int font_id = 0;
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;
  vgcanvas_nanovg_t* canvas = (vgcanvas_nanovg_t*)vgcanvas;

  if (name == NULL) {
    name = STR_DEFAULT_FONT;
  }

  font_id = nvgFindFont(vg, name);
  if (font_id < 0) {
    const resource_info_t* r = resource_manager_ref(RESOURCE_TYPE_FONT, name);
    if (r == NULL || r->subtype != RESOURCE_TYPE_FONT_TTF) {
      name = STR_DEFAULT_FONT;
      font_id = nvgFindFont(vg, name);
      if (font_id >= 0) {
        canvas->font_id = font_id;
        return RET_OK;
      }

      r = resource_manager_ref(RESOURCE_TYPE_FONT, name);
    }

    if (r != NULL && r->subtype == RESOURCE_TYPE_FONT_TTF) {
      canvas->font_id = nvgCreateFontMem(vg, name, (unsigned char*)r->data, r->size, 0);
      /*TODO*/
    }
  } else {
    canvas->font_id = font_id;
  }
  vgcanvas->font = name;
  nvgFontFaceId(vg, font_id);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_text_align(vgcanvas_t* vgcanvas, const char* text_align) {
  vgcanvas_nanovg_t* canvas = (vgcanvas_nanovg_t*)vgcanvas;

  if (text_align[0] == 'r') {
    canvas->text_align_h = NVG_ALIGN_RIGHT;
  } else if (text_align[0] == 'c') {
    canvas->text_align_h = NVG_ALIGN_CENTER;
  } else {
    canvas->text_align_h = NVG_ALIGN_LEFT;
  }

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_text_baseline(vgcanvas_t* vgcanvas, const char* text_baseline) {
  vgcanvas_nanovg_t* canvas = (vgcanvas_nanovg_t*)vgcanvas;

  if (text_baseline[0] == 'b') {
    canvas->text_align_v = NVG_ALIGN_BOTTOM;
  } else if (text_baseline[0] == 'm') {
    canvas->text_align_v = NVG_ALIGN_MIDDLE;
  } else {
    canvas->text_align_v = NVG_ALIGN_TOP;
  }

  return RET_OK;
}

static ret_t vgcanvas_nanovg_fill_text(vgcanvas_t* vgcanvas, const char* text, float_t x, float_t y,
                                       float_t max_width) {
  vgcanvas_nanovg_t* canvas = (vgcanvas_nanovg_t*)vgcanvas;
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgTextAlign(vg, canvas->text_align_v | canvas->text_align_h);
  nvgText(vg, x, y, text, text + strlen(text));

  return RET_OK;
}

static uint32_t vgcanvas_nanovg_measure_text(vgcanvas_t* vgcanvas, const char* text) {
  float bounds[4];
  vgcanvas_nanovg_t* canvas = (vgcanvas_nanovg_t*)vgcanvas;
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgTextAlign(vg, canvas->text_align_v | canvas->text_align_h);

  return nvgTextBounds(vg, 0, 0, text, text + strlen(text), bounds);
}

static int vgcanvas_nanovg_ensure_image(vgcanvas_nanovg_t* canvas, bitmap_t* img) {
  int32_t i = 0;
  for (i = 0; i < MAX_IMAGE_NR; i++) {
    if (canvas->images[i] == (img->data)) {
      return i;
    }
  }

  i = nvgCreateImageRGBA(canvas->vg, img->w, img->h, NVG_IMAGE_NEAREST, img->data);
  if (i >= 0) {
    assert(i < MAX_IMAGE_NR);
    canvas->images[i] = img->data;
  }

  return i;
}

static ret_t vgcanvas_nanovg_draw_image(vgcanvas_t* vgcanvas, bitmap_t* img, float_t sx, float_t sy,
                                        float_t sw, float_t sh, float_t dx, float_t dy, float_t dw,
                                        float_t dh) {
  int iw = img->w;
  int ih = img->h;
  NVGpaint imgPaint;
  float scaleX = (float)dw / sw;
  float scaleY = (float)dh / sh;
  vgcanvas_nanovg_t* canvas = (vgcanvas_nanovg_t*)vgcanvas;
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  int id = vgcanvas_nanovg_ensure_image(canvas, img);
  return_value_if_fail(id >= 0, RET_BAD_PARAMS);

  imgPaint = nvgImagePattern(vg, 0, 0, iw, ih, 0, id, 1);

  nvgSave(vg);
  nvgBeginPath(vg);
  nvgTranslate(vg, dx - (sx * scaleX), dy - (sy * scaleY));
  nvgScale(vg, scaleX, scaleY);
  nvgRect(vg, sx, sy, sw, sh);
  nvgFillPaint(vg, imgPaint);
  nvgClosePath(vg);
  nvgFill(vg);
  nvgRestore(vg);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_antialias(vgcanvas_t* vgcanvas, bool_t value) {
  (void)vgcanvas;
  (void)value;
  /*always*/

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_line_width(vgcanvas_t* vgcanvas, float_t value) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgStrokeWidth(vg, value);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_global_alpha(vgcanvas_t* vgcanvas, float_t value) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgGlobalAlpha(vg, value);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_fill_color(vgcanvas_t* vgcanvas, color_t c) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgFillColor(vg, nvgRGBA(c.rgba.r, c.rgba.g, c.rgba.b, c.rgba.a));

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_stroke_color(vgcanvas_t* vgcanvas, color_t c) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgStrokeColor(vg, nvgRGBA(c.rgba.r, c.rgba.g, c.rgba.b, c.rgba.a));

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_line_cap(vgcanvas_t* vgcanvas, const char* value) {
  int line_cap = NVG_BUTT;
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  if (*value == 'r') {
    line_cap = NVG_ROUND;
  } else if (*value == 's') {
    line_cap = NVG_SQUARE;
  }

  nvgLineCap(vg, line_cap);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_line_join(vgcanvas_t* vgcanvas, const char* value) {
  int line_join = NVG_MITER;
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  if (*value == 'r') {
    line_join = NVG_ROUND;
  } else if (*value == 'b') {
    line_join = NVG_BEVEL;
  }

  nvgLineJoin(vg, line_join);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_set_miter_limit(vgcanvas_t* vgcanvas, float_t value) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgMiterLimit(vg, value);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_save(vgcanvas_t* vgcanvas) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  nvgSave(vg);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_restore(vgcanvas_t* vgcanvas) {
  NVGcontext* vg = ((vgcanvas_nanovg_t*)vgcanvas)->vg;

  /*TODO*/
  nvgRestore(vg);

  return RET_OK;
}

static ret_t vgcanvas_nanovg_destroy(vgcanvas_t* vgcanvas) { return RET_OK; }

static const vgcanvas_vtable_t vt = {vgcanvas_nanovg_begin_frame,
                                     vgcanvas_nanovg_reset,
                                     vgcanvas_nanovg_flush,
                                     vgcanvas_nanovg_clear_rect,
                                     vgcanvas_nanovg_begin_path,
                                     vgcanvas_nanovg_move_to,
                                     vgcanvas_nanovg_line_to,
                                     vgcanvas_nanovg_arc,
                                     vgcanvas_nanovg_arc_to,
                                     vgcanvas_nanovg_bezier_curve_to,
                                     vgcanvas_nanovg_quadratic_curve_to,
                                     vgcanvas_nanovg_is_point_in_path,
                                     vgcanvas_nanovg_ellipse,
                                     vgcanvas_nanovg_round_rect,
                                     vgcanvas_nanovg_close_path,
                                     vgcanvas_nanovg_scale,
                                     vgcanvas_nanovg_rotate,
                                     vgcanvas_nanovg_translate,
                                     vgcanvas_nanovg_transform,
                                     vgcanvas_nanovg_set_transform,
                                     vgcanvas_nanovg_fill,
                                     vgcanvas_nanovg_clip_rect,
                                     vgcanvas_nanovg_stroke,
                                     vgcanvas_nanovg_set_font,
                                     vgcanvas_nanovg_set_font_size,
                                     vgcanvas_nanovg_set_text_align,
                                     vgcanvas_nanovg_set_text_baseline,
                                     vgcanvas_nanovg_fill_text,
                                     vgcanvas_nanovg_measure_text,
                                     vgcanvas_nanovg_draw_image,
                                     vgcanvas_nanovg_set_antialias,
                                     vgcanvas_nanovg_set_global_alpha,
                                     vgcanvas_nanovg_set_line_width,
                                     vgcanvas_nanovg_set_fill_color,
                                     vgcanvas_nanovg_set_stroke_color,
                                     vgcanvas_nanovg_set_line_join,
                                     vgcanvas_nanovg_set_line_cap,
                                     vgcanvas_nanovg_set_miter_limit,
                                     vgcanvas_nanovg_save,
                                     vgcanvas_nanovg_restore,
                                     vgcanvas_nanovg_end_frame,
                                     vgcanvas_nanovg_destroy};

vgcanvas_t* vgcanvas_create(uint32_t w, uint32_t h, void* sdl_window) {
  vgcanvas_nanovg_t* nanovg = (vgcanvas_nanovg_t*)MEM_ZALLOC(vgcanvas_nanovg_t);
  return_value_if_fail(nanovg != NULL, NULL);

  nanovg->base.w = w;
  nanovg->base.h = h;
  nanovg->base.vt = &vt;
  nanovg->sdl_window = (SDL_Window*)sdl_window;
  nanovg->vg = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

  return &(nanovg->base);
}
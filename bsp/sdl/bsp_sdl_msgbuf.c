/*****************************************************************************
 * Copyright (C) 2026 Sunny Matato
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar.
 * See http://www.wtfpl.net/ for more details.
 ****************************************************************************/
/*==========================================================================*/
#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "bsp_sdl_msgbuf.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*==========================================================================*/
/* Terminus-style 6x12 bitmap font, ASCII 0x20-0x7E + fallback.
 * Each glyph: 12 rows, each row stored in one byte,
 * bit 7 = leftmost (col 0) ... bit 2 = col 5; bits 1-0 unused.
 * Source: public domain, derived from the X11 Misc-Fixed 6x12 font. */
#define FONT_FIRST 0x20
#define FONT_LAST  0x7E
#define FONT_W  6
#define FONT_H  12
static const uint8_t s_font6x12[][FONT_H] = {
    /* 0x20 space */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x21 !     */  {0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x00,0x00,0x00},
    /* 0x22 "     */  {0x00,0x50,0x50,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x23 #     */  {0x00,0x50,0x50,0xF8,0x50,0xF8,0x50,0x50,0x00,0x00,0x00,0x00},
    /* 0x24 $     */  {0x00,0x20,0x70,0xA0,0x70,0x28,0x70,0x20,0x00,0x00,0x00,0x00},
    /* 0x25 %     */  {0x00,0x40,0xA8,0x50,0x20,0x50,0xA8,0x08,0x00,0x00,0x00,0x00},
    /* 0x26 &     */  {0x00,0x60,0x90,0x90,0x60,0x98,0x90,0x68,0x00,0x00,0x00,0x00},
    /* 0x27 '     */  {0x00,0x20,0x20,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x28 (     */  {0x00,0x10,0x20,0x40,0x40,0x40,0x20,0x10,0x00,0x00,0x00,0x00},
    /* 0x29 )     */  {0x00,0x40,0x20,0x10,0x10,0x10,0x20,0x40,0x00,0x00,0x00,0x00},
    /* 0x2A *     */  {0x00,0x00,0x20,0xA8,0x70,0xA8,0x20,0x00,0x00,0x00,0x00,0x00},
    /* 0x2B +     */  {0x00,0x00,0x00,0x20,0x20,0xF8,0x20,0x20,0x00,0x00,0x00,0x00},
    /* 0x2C ,     */  {0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x20,0x40,0x00,0x00,0x00},
    /* 0x2D -     */  {0x00,0x00,0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x2E .     */  {0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,0x00},
    /* 0x2F /     */  {0x00,0x08,0x10,0x10,0x20,0x20,0x40,0x40,0x80,0x00,0x00,0x00},
    /* 0x30 0     */  {0x00,0x70,0x88,0x98,0xA8,0xC8,0x88,0x70,0x00,0x00,0x00,0x00},
    /* 0x31 1     */  {0x00,0x20,0x60,0x20,0x20,0x20,0x20,0x70,0x00,0x00,0x00,0x00},
    /* 0x32 2     */  {0x00,0x70,0x88,0x08,0x30,0x40,0x80,0xF8,0x00,0x00,0x00,0x00},
    /* 0x33 3     */  {0x00,0xF8,0x08,0x10,0x30,0x08,0x88,0x70,0x00,0x00,0x00,0x00},
    /* 0x34 4     */  {0x00,0x10,0x30,0x50,0x90,0xF8,0x10,0x10,0x00,0x00,0x00,0x00},
    /* 0x35 5     */  {0x00,0xF8,0x80,0xF0,0x08,0x08,0x88,0x70,0x00,0x00,0x00,0x00},
    /* 0x36 6     */  {0x00,0x30,0x40,0x80,0xF0,0x88,0x88,0x70,0x00,0x00,0x00,0x00},
    /* 0x37 7     */  {0x00,0xF8,0x08,0x10,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x00},
    /* 0x38 8     */  {0x00,0x70,0x88,0x88,0x70,0x88,0x88,0x70,0x00,0x00,0x00,0x00},
    /* 0x39 9     */  {0x00,0x70,0x88,0x88,0x78,0x08,0x10,0x60,0x00,0x00,0x00,0x00},
    /* 0x3A :     */  {0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x00,0x00,0x00,0x00,0x00},
    /* 0x3B ;     */  {0x00,0x00,0x30,0x30,0x00,0x30,0x20,0x40,0x00,0x00,0x00,0x00},
    /* 0x3C <     */  {0x00,0x00,0x10,0x20,0x40,0x20,0x10,0x00,0x00,0x00,0x00,0x00},
    /* 0x3D =     */  {0x00,0x00,0x00,0xF8,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x3E >     */  {0x00,0x00,0x40,0x20,0x10,0x20,0x40,0x00,0x00,0x00,0x00,0x00},
    /* 0x3F ?     */  {0x00,0x70,0x88,0x08,0x10,0x20,0x00,0x20,0x00,0x00,0x00,0x00},
    /* 0x40 @     */  {0x00,0x70,0x88,0x98,0xA8,0xB0,0x80,0x70,0x00,0x00,0x00,0x00},
    /* 0x41 A     */  {0x00,0x20,0x50,0x88,0x88,0xF8,0x88,0x88,0x00,0x00,0x00,0x00},
    /* 0x42 B     */  {0x00,0xF0,0x88,0x88,0xF0,0x88,0x88,0xF0,0x00,0x00,0x00,0x00},
    /* 0x43 C     */  {0x00,0x70,0x88,0x80,0x80,0x80,0x88,0x70,0x00,0x00,0x00,0x00},
    /* 0x44 D     */  {0x00,0xE0,0x90,0x88,0x88,0x88,0x90,0xE0,0x00,0x00,0x00,0x00},
    /* 0x45 E     */  {0x00,0xF8,0x80,0x80,0xF0,0x80,0x80,0xF8,0x00,0x00,0x00,0x00},
    /* 0x46 F     */  {0x00,0xF8,0x80,0x80,0xF0,0x80,0x80,0x80,0x00,0x00,0x00,0x00},
    /* 0x47 G     */  {0x00,0x70,0x88,0x80,0xB8,0x88,0x88,0x70,0x00,0x00,0x00,0x00},
    /* 0x48 H     */  {0x00,0x88,0x88,0x88,0xF8,0x88,0x88,0x88,0x00,0x00,0x00,0x00},
    /* 0x49 I     */  {0x00,0x70,0x20,0x20,0x20,0x20,0x20,0x70,0x00,0x00,0x00,0x00},
    /* 0x4A J     */  {0x00,0x38,0x10,0x10,0x10,0x10,0x90,0x60,0x00,0x00,0x00,0x00},
    /* 0x4B K     */  {0x00,0x88,0x90,0xA0,0xC0,0xA0,0x90,0x88,0x00,0x00,0x00,0x00},
    /* 0x4C L     */  {0x00,0x80,0x80,0x80,0x80,0x80,0x80,0xF8,0x00,0x00,0x00,0x00},
    /* 0x4D M     */  {0x00,0x88,0xD8,0xA8,0x88,0x88,0x88,0x88,0x00,0x00,0x00,0x00},
    /* 0x4E N     */  {0x00,0x88,0xC8,0xA8,0x98,0x88,0x88,0x88,0x00,0x00,0x00,0x00},
    /* 0x4F O     */  {0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x00,0x00,0x00},
    /* 0x50 P     */  {0x00,0xF0,0x88,0x88,0xF0,0x80,0x80,0x80,0x00,0x00,0x00,0x00},
    /* 0x51 Q     */  {0x00,0x70,0x88,0x88,0x88,0xA8,0x90,0x68,0x00,0x00,0x00,0x00},
    /* 0x52 R     */  {0x00,0xF0,0x88,0x88,0xF0,0xA0,0x90,0x88,0x00,0x00,0x00,0x00},
    /* 0x53 S     */  {0x00,0x70,0x88,0x80,0x70,0x08,0x88,0x70,0x00,0x00,0x00,0x00},
    /* 0x54 T     */  {0x00,0xF8,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x00},
    /* 0x55 U     */  {0x00,0x88,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x00,0x00,0x00},
    /* 0x56 V     */  {0x00,0x88,0x88,0x88,0x50,0x50,0x20,0x20,0x00,0x00,0x00,0x00},
    /* 0x57 W     */  {0x00,0x88,0x88,0x88,0xA8,0xA8,0xD8,0x88,0x00,0x00,0x00,0x00},
    /* 0x58 X     */  {0x00,0x88,0x88,0x50,0x20,0x50,0x88,0x88,0x00,0x00,0x00,0x00},
    /* 0x59 Y     */  {0x00,0x88,0x88,0x50,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x00},
    /* 0x5A Z     */  {0x00,0xF8,0x08,0x10,0x20,0x40,0x80,0xF8,0x00,0x00,0x00,0x00},
    /* 0x5B [     */  {0x00,0x70,0x40,0x40,0x40,0x40,0x40,0x70,0x00,0x00,0x00,0x00},
    /* 0x5C \     */  {0x00,0x80,0x40,0x40,0x20,0x20,0x10,0x08,0x00,0x00,0x00,0x00},
    /* 0x5D ]     */  {0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x70,0x00,0x00,0x00,0x00},
    /* 0x5E ^     */  {0x00,0x20,0x50,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x5F _     */  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00},
    /* 0x60 `     */  {0x00,0x40,0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x61 a     */  {0x00,0x00,0x00,0x70,0x08,0x78,0x88,0x78,0x00,0x00,0x00,0x00},
    /* 0x62 b     */  {0x00,0x80,0x80,0xF0,0x88,0x88,0x88,0xF0,0x00,0x00,0x00,0x00},
    /* 0x63 c     */  {0x00,0x00,0x00,0x70,0x88,0x80,0x88,0x70,0x00,0x00,0x00,0x00},
    /* 0x64 d     */  {0x00,0x08,0x08,0x78,0x88,0x88,0x88,0x78,0x00,0x00,0x00,0x00},
    /* 0x65 e     */  {0x00,0x00,0x00,0x70,0x88,0xF8,0x80,0x70,0x00,0x00,0x00,0x00},
    /* 0x66 f     */  {0x00,0x30,0x40,0x40,0xF0,0x40,0x40,0x40,0x00,0x00,0x00,0x00},
    /* 0x67 g     */  {0x00,0x00,0x00,0x78,0x88,0x88,0x78,0x08,0x70,0x00,0x00,0x00},
    /* 0x68 h     */  {0x00,0x80,0x80,0xF0,0x88,0x88,0x88,0x88,0x00,0x00,0x00,0x00},
    /* 0x69 i     */  {0x00,0x20,0x00,0x60,0x20,0x20,0x20,0x70,0x00,0x00,0x00,0x00},
    /* 0x6A j     */  {0x00,0x10,0x00,0x30,0x10,0x10,0x10,0x90,0x60,0x00,0x00,0x00},
    /* 0x6B k     */  {0x00,0x80,0x80,0x90,0xA0,0xC0,0xA0,0x90,0x00,0x00,0x00,0x00},
    /* 0x6C l     */  {0x00,0x60,0x20,0x20,0x20,0x20,0x20,0x70,0x00,0x00,0x00,0x00},
    /* 0x6D m     */  {0x00,0x00,0x00,0xD0,0xA8,0xA8,0x88,0x88,0x00,0x00,0x00,0x00},
    /* 0x6E n     */  {0x00,0x00,0x00,0xF0,0x88,0x88,0x88,0x88,0x00,0x00,0x00,0x00},
    /* 0x6F o     */  {0x00,0x00,0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x00,0x00,0x00},
    /* 0x70 p     */  {0x00,0x00,0x00,0xF0,0x88,0x88,0xF0,0x80,0x80,0x00,0x00,0x00},
    /* 0x71 q     */  {0x00,0x00,0x00,0x78,0x88,0x88,0x78,0x08,0x08,0x00,0x00,0x00},
    /* 0x72 r     */  {0x00,0x00,0x00,0xB0,0xC8,0x80,0x80,0x80,0x00,0x00,0x00,0x00},
    /* 0x73 s     */  {0x00,0x00,0x00,0x70,0x80,0x70,0x08,0xF0,0x00,0x00,0x00,0x00},
    /* 0x74 t     */  {0x00,0x40,0x40,0xF0,0x40,0x40,0x40,0x30,0x00,0x00,0x00,0x00},
    /* 0x75 u     */  {0x00,0x00,0x00,0x88,0x88,0x88,0x98,0x68,0x00,0x00,0x00,0x00},
    /* 0x76 v     */  {0x00,0x00,0x00,0x88,0x88,0x50,0x50,0x20,0x00,0x00,0x00,0x00},
    /* 0x77 w     */  {0x00,0x00,0x00,0x88,0x88,0xA8,0xA8,0x50,0x00,0x00,0x00,0x00},
    /* 0x78 x     */  {0x00,0x00,0x00,0x88,0x50,0x20,0x50,0x88,0x00,0x00,0x00,0x00},
    /* 0x79 y     */  {0x00,0x00,0x00,0x88,0x88,0x78,0x08,0x70,0x00,0x00,0x00,0x00},
    /* 0x7A z     */  {0x00,0x00,0x00,0xF8,0x10,0x20,0x40,0xF8,0x00,0x00,0x00,0x00},
    /* 0x7B {     */  {0x00,0x18,0x20,0x20,0x40,0x20,0x20,0x18,0x00,0x00,0x00,0x00},
    /* 0x7C |     */  {0x00,0x20,0x20,0x20,0x00,0x20,0x20,0x20,0x00,0x00,0x00,0x00},
    /* 0x7D }     */  {0x00,0xC0,0x20,0x20,0x10,0x20,0x20,0xC0,0x00,0x00,0x00,0x00},
    /* 0x7E ~     */  {0x00,0x48,0xB0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
};

/*==========================================================================*/
/* ...Font and layout constants. */
#define FONT_SCALE   2
#define CELL_W       (FONT_W * FONT_SCALE)
#define CELL_H       (FONT_H * FONT_SCALE)
#define LINE_H       (CELL_H + 3)
#define PAD_X        12
#define PAD_Y        8
#define SCROLLBAR_W  10

/*==========================================================================*/
/* ...Circular line buffer. */
#define MAX_LINES    4096
#define MAX_LINE_LEN 1024

typedef struct {
    char  text[MAX_LINE_LEN];
    int   len;
} Line;

/*==========================================================================*/
/* ...Selection range: (row, col) pairs in buffer-line coordinates. */
typedef struct {
    int row;
    int col;
} SelPt;

/*==========================================================================*/
static SDL_Renderer *l_renderer     = NULL;
static int           l_panel_y      = 0;
static int           l_panel_w      = 0;
static int           l_panel_h      = 0;

/* Font atlas texture: all 95 printable ASCII glyphs in a single row.
 * Each glyph occupies CELL_W x CELL_H pixels.
 * Normal colour: RGB(171,178,191). Selected colour applied via ColorMod. */
#define ATLAS_GLYPH_COUNT (FONT_LAST - FONT_FIRST + 1)  /* 95 */
static SDL_Texture *l_font_atlas     = NULL;
static int          l_atlas_glyph_w  = CELL_W;
static int          l_atlas_glyph_h  = CELL_H;

static Line          l_lines[MAX_LINES];
static int           l_line_head    = 0;   /* index of oldest line */
static int           l_line_count   = 0;   /* number of valid lines */

static int           l_scroll_top   = 0;   /* first visible line index (buffer-absolute) */
static bool          l_auto_scroll  = true; /* follow tail when true */

static bool          l_sel_active   = false;
static SelPt         l_sel_start;
static SelPt         l_sel_end;
static bool          l_mouse_down   = false;

static SDL_Texture  *l_panel_cache  = NULL;  /* off-screen render target */
static bool          l_cache_dirty  = true;  /* repaint needed */

/*==========================================================================*/
static void create_panel_cache(void) {
    if (l_panel_cache) {
        SDL_DestroyTexture(l_panel_cache);
        l_panel_cache = NULL;
    }
    if (l_panel_w <= 0 || l_panel_h <= 0) return;
    l_panel_cache = SDL_CreateTexture(l_renderer,
                                      SDL_PIXELFORMAT_RGBA8888,
                                      SDL_TEXTUREACCESS_TARGET,
                                      l_panel_w, l_panel_h);
    l_cache_dirty = true;
}

/*==========================================================================*/
static int buf_line_count(void) {
    return l_line_count;
}

static Line *buf_line_at(int idx) {
    return &l_lines[(l_line_head + idx) % MAX_LINES];
}

static int visible_rows(void) {
    return (l_panel_h - PAD_Y * 2) / LINE_H;
}

static int text_area_w(void) {
    return l_panel_w - PAD_X * 2 - SCROLLBAR_W;
}

/*==========================================================================*/
static void split_and_append_raw(const char *text) {
    const char *p = text;
    while (*p) {
        const char *nl = strchr(p, '\n');
        size_t len = nl ? (size_t)(nl - p) : strlen(p);

        if (l_line_count < MAX_LINES) {
            int idx = (l_line_head + l_line_count) % MAX_LINES;
            int copy_len = (len >= MAX_LINE_LEN) ? MAX_LINE_LEN - 1 : (int)len;
            memcpy(l_lines[idx].text, p, (size_t)copy_len);
            l_lines[idx].text[copy_len] = '\0';
            l_lines[idx].len = copy_len;
            l_line_count++;
        } else {
            int idx = l_line_head % MAX_LINES;
            int copy_len = (len >= MAX_LINE_LEN) ? MAX_LINE_LEN - 1 : (int)len;
            memcpy(l_lines[idx].text, p, (size_t)copy_len);
            l_lines[idx].text[copy_len] = '\0';
            l_lines[idx].len = copy_len;
            l_line_head = (l_line_head + 1) % MAX_LINES;
        }

        p += len;
        if (nl) p++;
    }
}

/*==========================================================================*/
static void build_font_atlas(void) {
    int atlas_w = ATLAS_GLYPH_COUNT * l_atlas_glyph_w;
    int atlas_h = l_atlas_glyph_h;

    l_font_atlas = SDL_CreateTexture(l_renderer,
                                     SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_TARGET,
                                     atlas_w, atlas_h);
    if (!l_font_atlas) return;

    SDL_SetTextureBlendMode(l_font_atlas, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(l_renderer, l_font_atlas);
    SDL_SetRenderDrawColor(l_renderer, 0, 0, 0, 0);
    SDL_RenderClear(l_renderer);

    SDL_SetRenderDrawColor(l_renderer, 255, 255, 255, 255);
    for (int gi = 0; gi < ATLAS_GLYPH_COUNT; gi++) {
        const uint8_t *bitmap = s_font6x12[gi];
        int base_x = gi * l_atlas_glyph_w;
        for (int row = 0; row < FONT_H; row++) {
            uint8_t bits = bitmap[row];
            for (int col = 0; col < FONT_W; col++) {
                if (bits & (0x80u >> col)) {
                    SDL_Rect dst = {
                        base_x + col * FONT_SCALE,
                        row    * FONT_SCALE,
                        FONT_SCALE, FONT_SCALE
                    };
                    SDL_RenderFillRect(l_renderer, &dst);
                }
            }
        }
    }

    SDL_SetRenderTarget(l_renderer, NULL);
}

/*==========================================================================*/
void BSP_msgbuf_init(void *renderer, int panel_y, int panel_w, int panel_h) {
    l_renderer   = (SDL_Renderer *)renderer;
    l_panel_y    = panel_y;
    l_panel_w    = panel_w;
    l_panel_h    = panel_h;
    l_line_head  = 0;
    l_line_count = 0;
    l_scroll_top = 0;
    l_auto_scroll = true;
    l_sel_active  = false;
    l_mouse_down  = false;

    build_font_atlas();
    create_panel_cache();
}

/*==========================================================================*/
void BSP_msgbuf_append(char *msg) {
    if (!msg) return;
    split_and_append_raw(msg);
    free(msg);

    if (l_auto_scroll) {
        int rows = visible_rows();
        int total = buf_line_count();
        l_scroll_top = (total > rows) ? (total - rows) : 0;
    }
    l_cache_dirty = true;
}

/*==========================================================================*/
void BSP_msgbuf_clear(void) {
    l_line_head  = 0;
    l_line_count = 0;
    l_scroll_top = 0;
    l_auto_scroll = true;
    l_sel_active  = false;
    l_cache_dirty = true;
}

/*==========================================================================*/
static void draw_glyph(int px, int py, unsigned char c,
                        uint8_t r, uint8_t g, uint8_t b)
{
    if (!l_font_atlas) return;
    unsigned char gi = (c >= FONT_FIRST && c <= FONT_LAST)
                       ? (c - FONT_FIRST) : 0;
    SDL_Rect src = { gi * l_atlas_glyph_w, 0, l_atlas_glyph_w, l_atlas_glyph_h };
    SDL_Rect dst = { px, py, l_atlas_glyph_w, l_atlas_glyph_h };
    SDL_SetTextureColorMod(l_font_atlas, r, g, b);
    SDL_RenderCopy(l_renderer, l_font_atlas, &src, &dst);
}

/*==========================================================================*/
static bool pt_less(SelPt a, SelPt b) {
    return (a.row < b.row) || (a.row == b.row && a.col < b.col);
}

static SelPt sel_min(void) { return pt_less(l_sel_start, l_sel_end) ? l_sel_start : l_sel_end; }
static SelPt sel_max(void) { return pt_less(l_sel_start, l_sel_end) ? l_sel_end : l_sel_start; }

static bool char_in_selection(int line_idx, int col) {
    if (!l_sel_active) return false;
    SelPt lo = sel_min();
    SelPt hi = sel_max();
    if (line_idx < lo.row || line_idx > hi.row) return false;
    if (line_idx == lo.row && col < lo.col) return false;
    if (line_idx == hi.row && col >= hi.col) return false;
    return true;
}

/*==========================================================================*/
static void draw_scrollbar_to_cache(void) {
    int total  = buf_line_count();
    int rows   = visible_rows();
    if (total <= rows) return;

    int track_x = l_panel_w - SCROLLBAR_W + 1;
    int track_y = PAD_Y;
    int track_h = l_panel_h - PAD_Y * 2;

    SDL_SetRenderDrawColor(l_renderer, 30, 31, 40, 255);
    SDL_Rect track = { track_x, track_y, SCROLLBAR_W - 2, track_h };
    SDL_RenderFillRect(l_renderer, &track);

    float ratio   = (float)rows / (float)total;
    int   thumb_h = (int)(track_h * ratio);
    if (thumb_h < 8) thumb_h = 8;

    float pos_ratio = (float)l_scroll_top / (float)(total - rows);
    int   thumb_y   = track_y + (int)(pos_ratio * (track_h - thumb_h));

    SDL_SetRenderDrawColor(l_renderer, 86, 92, 114, 255);
    SDL_Rect thumb = { track_x + 2, thumb_y, SCROLLBAR_W - 4, thumb_h };
    SDL_RenderFillRect(l_renderer, &thumb);
}

/*==========================================================================*/
/* Banner: "SunnyMatato" rendered as large pixel-dot art.
 * Each glyph is a 5-wide x 7-tall bitmap (LSB = col 0).
 * Rendered only when the buffer is empty. */

#define BAN_DOT_SIZE  10
#define BAN_DOT_GAP    2
#define BAN_STEP      (BAN_DOT_SIZE + BAN_DOT_GAP)
#define BAN_CHAR_W    (5 * BAN_STEP - BAN_DOT_GAP)
#define BAN_CHAR_H    (7 * BAN_STEP - BAN_DOT_GAP)
#define BAN_LETTER_GAP 14

static const uint8_t s_ban_glyphs[][7] = {
    /* S */ { 0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E },
    /* u */ { 0x00, 0x11, 0x11, 0x11, 0x11, 0x13, 0x0D },
    /* n */ { 0x00, 0x16, 0x19, 0x11, 0x11, 0x11, 0x11 },
    /* n */ { 0x00, 0x16, 0x19, 0x11, 0x11, 0x11, 0x11 },
    /* y */ { 0x00, 0x11, 0x11, 0x11, 0x0F, 0x01, 0x0E },
    /* M */ { 0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11 },
    /* a */ { 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x13, 0x0D },
    /* t */ { 0x04, 0x04, 0x1F, 0x04, 0x04, 0x04, 0x03 },
    /* a */ { 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x13, 0x0D },
    /* t */ { 0x04, 0x04, 0x1F, 0x04, 0x04, 0x04, 0x03 },
    /* o */ { 0x00, 0x0E, 0x11, 0x11, 0x11, 0x11, 0x0E },
};
#define BAN_NLETTERS 11

static void draw_banner_to_cache(void) {
    int total_w = BAN_NLETTERS * BAN_CHAR_W + (BAN_NLETTERS - 1) * BAN_LETTER_GAP;
    int total_h = BAN_CHAR_H;
    int ox = (l_panel_w - total_w) / 2;
    int oy = (l_panel_h - total_h) / 2;

    SDL_SetRenderDrawColor(l_renderer, 86, 182, 255, 255);
    for (int li = 0; li < BAN_NLETTERS; li++) {
        int lx = ox + li * (BAN_CHAR_W + BAN_LETTER_GAP);
        for (int row = 0; row < 7; row++) {
            uint8_t bits = s_ban_glyphs[li][row];
            for (int col = 0; col < 5; col++) {
                if (bits & (1u << (4 - col))) {
                    SDL_Rect dot = {
                        lx + col * BAN_STEP,
                        oy + row * BAN_STEP,
                        BAN_DOT_SIZE, BAN_DOT_SIZE
                    };
                    SDL_RenderFillRect(l_renderer, &dot);
                }
            }
        }
    }
}

/*==========================================================================*/
void BSP_msgbuf_render(void) {
    if (!l_renderer) return;
    if (!l_panel_cache) return;

    if (l_cache_dirty) {
        SDL_SetRenderTarget(l_renderer, l_panel_cache);
        SDL_SetRenderDrawBlendMode(l_renderer, SDL_BLENDMODE_NONE);

        SDL_SetRenderDrawColor(l_renderer, 21, 22, 30, 255);
        SDL_Rect full = { 0, 0, l_panel_w, l_panel_h };
        SDL_RenderFillRect(l_renderer, &full);

        SDL_SetRenderDrawColor(l_renderer, 49, 50, 68, 255);
        SDL_RenderDrawLine(l_renderer, 0, 0, l_panel_w, 0);

        int rows     = visible_rows();
        int total    = buf_line_count();
        int max_cols = text_area_w() / CELL_W;

        if (total == 0) {
            draw_banner_to_cache();
        } else {
            for (int r = 0; r < rows; r++) {
                int line_idx = l_scroll_top + r;
                if (line_idx >= total) break;
                Line *ln = buf_line_at(line_idx);
                int py = PAD_Y + r * LINE_H;
                for (int c = 0; c < ln->len && c < max_cols; c++) {
                    int px = PAD_X + c * CELL_W;
                    if (char_in_selection(line_idx, c)) {
                        SDL_Rect sel_rect = { px, py, CELL_W, CELL_H };
                        SDL_SetRenderDrawColor(l_renderer, 62, 68, 81, 255);
                        SDL_RenderFillRect(l_renderer, &sel_rect);
                        draw_glyph(px, py, (unsigned char)ln->text[c], 229, 229, 229);
                    } else {
                        draw_glyph(px, py, (unsigned char)ln->text[c], 171, 178, 191);
                    }
                }
            }
            draw_scrollbar_to_cache();
        }

        SDL_SetRenderTarget(l_renderer, NULL);
        l_cache_dirty = false;
    }

    SDL_Rect dst = { 0, l_panel_y, l_panel_w, l_panel_h };
    SDL_RenderCopy(l_renderer, l_panel_cache, NULL, &dst);
}

/*==========================================================================*/
static int pixel_to_col(int px) {
    int col = (px - PAD_X) / CELL_W;
    return (col < 0) ? 0 : col;
}

static int pixel_to_row(int py) {
    int r = (py - l_panel_y - PAD_Y) / LINE_H;
    int total = buf_line_count();
    int row = l_scroll_top + r;
    if (row < 0) row = 0;
    if (row >= total) row = total - 1;
    return row;
}

static bool in_scrollbar(int x) {
    return x >= (l_panel_w - SCROLLBAR_W);
}

static bool in_panel(int x, int y) {
    (void)x;
    return (y >= l_panel_y && y < l_panel_y + l_panel_h);
}

/*==========================================================================*/
void BSP_msgbuf_mouse_button(int x, int y, bool down) {
    if (!in_panel(x, y)) return;

    l_mouse_down = down;

    if (down && !in_scrollbar(x)) {
        l_sel_active      = true;
        l_sel_start.row   = pixel_to_row(y);
        l_sel_start.col   = pixel_to_col(x);
        l_sel_end         = l_sel_start;
    }

    if (!down && in_scrollbar(x)) {
        int total  = buf_line_count();
        int rows   = visible_rows();
        if (total <= rows) return;

        int track_h  = l_panel_h - PAD_Y * 2;
        int rel_y    = y - (l_panel_y + PAD_Y);
        int new_top  = (int)((float)rel_y / (float)track_h * (total - rows));
        if (new_top < 0) new_top = 0;
        if (new_top > total - rows) new_top = total - rows;
        l_scroll_top  = new_top;
        l_auto_scroll = (new_top >= total - rows);
    }
    l_cache_dirty = true;
}

/*==========================================================================*/
void BSP_msgbuf_mouse_motion(int x, int y, bool pressed) {
    if (!in_panel(x, y)) return;

    if (pressed && l_mouse_down && !in_scrollbar(x)) {
        l_sel_end.row = pixel_to_row(y);
        l_sel_end.col = pixel_to_col(x);
    }

    if (pressed && l_mouse_down && in_scrollbar(x)) {
        int total    = buf_line_count();
        int rows     = visible_rows();
        if (total <= rows) return;

        int track_h  = l_panel_h - PAD_Y * 2;
        int rel_y    = y - (l_panel_y + PAD_Y);
        int new_top  = (int)((float)rel_y / (float)track_h * (total - rows));
        if (new_top < 0) new_top = 0;
        if (new_top > total - rows) new_top = total - rows;
        l_scroll_top  = new_top;
        l_auto_scroll = (new_top >= total - rows);
    }
    l_cache_dirty = true;
}

/*==========================================================================*/
void BSP_msgbuf_mouse_wheel(int dy) {
    int total = buf_line_count();
    int rows  = visible_rows();

    l_scroll_top -= dy * 3;
    if (l_scroll_top < 0) l_scroll_top = 0;
    if (total > rows && l_scroll_top > total - rows) l_scroll_top = total - rows;

    l_auto_scroll = (total <= rows) || (l_scroll_top >= total - rows);
    l_cache_dirty = true;
}

/*==========================================================================*/
void BSP_msgbuf_copy_selection(void) {
    if (!l_sel_active) return;

    SelPt lo = sel_min();
    SelPt hi = sel_max();
    if (lo.row == hi.row && lo.col == hi.col) return;

    int total = buf_line_count();

    size_t cap = 4096;
    char  *buf = (char *)malloc(cap);
    if (!buf) return;
    size_t used = 0;

    for (int r = lo.row; r <= hi.row && r < total; r++) {
        Line *ln   = buf_line_at(r);
        int  start = (r == lo.row) ? lo.col : 0;
        int  end   = (r == hi.row) ? hi.col : ln->len;
        if (end > ln->len) end = ln->len;

        for (int c = start; c < end; c++) {
            if (used + 2 >= cap) {
                cap *= 2;
                char *tmp = (char *)realloc(buf, cap);
                if (!tmp) { free(buf); return; }
                buf = tmp;
            }
            buf[used++] = ln->text[c];
        }
        if (r < hi.row) {
            if (used + 2 >= cap) {
                cap *= 2;
                char *tmp = (char *)realloc(buf, cap);
                if (!tmp) { free(buf); return; }
                buf = tmp;
            }
            buf[used++] = '\n';
        }
    }
    buf[used] = '\0';

    SDL_SetClipboardText(buf);
    free(buf);
}

/*==========================================================================*/
void BSP_msgbuf_resize(int new_w, int new_h) {
    l_panel_w = new_w;
    l_panel_h = new_h - l_panel_y;
    if (l_panel_h < 0) l_panel_h = 0;

    int total = buf_line_count();
    int rows  = visible_rows();
    if (l_auto_scroll && total > rows) {
        l_scroll_top = total - rows;
    }
    create_panel_cache();
}

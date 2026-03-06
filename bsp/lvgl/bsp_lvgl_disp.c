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
#include "qpc.h"
#include "bsp.h"
#include "application.h"

/*==========================================================================*/
Q_DEFINE_THIS_MODULE("bsp_lvgl_disp")

/*==========================================================================*/
/* ...SDL Texture and Renderer. */
static SDL_Texture  *l_texture      = NULL;
static SDL_Renderer *l_renderer_ref = NULL;

/*==========================================================================*/
/* ...LVGL draw buffer. */
#define DISP_BUF_SIZE (2650U * 1440U)  /* 2K */
static lv_color_t l_disp_buf_mem[DISP_BUF_SIZE];
static lv_disp_draw_buf_t l_draw_buf_dsc;

/*==========================================================================*/
/**
 * @brief LVGL flush callback instance.
 *
 * @param disp_drv
 *
 * @param area
 *
 * @param color_p
 */
static void disp_flush_cb(
    lv_disp_drv_t * disp_drv,
    const lv_area_t * area,
    lv_color_t * color_p)
{
    if (l_texture) {
        SDL_Rect rect;
        rect.x = area->x1;
        rect.y = area->y1;
        rect.w = (area->x2 - area->x1 + 1);
        rect.h = (area->y2 - area->y1 + 1);
        /* Update SDL's texture */
        SDL_UpdateTexture(l_texture, &rect, color_p, rect.w * 4);
    }
    lv_disp_flush_ready(disp_drv);  /* Inform LVGL's display driver */
}

/*==========================================================================*/
/* External initialization of input device. */
extern void lv_port_indev_init_internal(void);
void BSP_lvgl_init(const BSP_lvgl_cfg_t *cfg) {
    if (!cfg || !cfg->renderer) return;

    /* SDL's renderer */
    l_renderer_ref = (SDL_Renderer *)cfg->renderer;

    lv_init();  /* Core of LVGL */

    /* Texture of SDL */
    l_texture = SDL_CreateTexture(
                            l_renderer_ref,
                            SDL_PIXELFORMAT_ARGB8888,
                            SDL_TEXTUREACCESS_STREAMING,
                            cfg->screen_width,
                            cfg->screen_height);

    /* LVGL draw buffer */
    lv_disp_draw_buf_init(
        &l_draw_buf_dsc,
        l_disp_buf_mem, NULL,
        DISP_BUF_SIZE);

    /* Display driver device register... */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = cfg->screen_width;
    disp_drv.ver_res = cfg->screen_height;
    disp_drv.flush_cb = disp_flush_cb;
    disp_drv.draw_buf = &l_draw_buf_dsc;
    lv_disp_drv_register(&disp_drv);

    /* Input driver device register. */
    lv_port_indev_init_internal();
}
/*..........................................................................*/
void BSP_lvgl_task(uint32_t dt_ms) {
    lv_tick_inc(dt_ms);
    lv_timer_handler();
}
/*..........................................................................*/
void BSP_lvgl_render(void) {
    if (l_texture && l_renderer_ref) {
        /* Texture --> Renderer */
        SDL_RenderCopy(l_renderer_ref, l_texture, NULL, NULL);
    }
}

/*==========================================================================*/
void BSP_lvgl_resize(int width, int height) {
    if (l_texture) {
        SDL_DestroyTexture(l_texture);
        l_texture = NULL;
    }

    l_texture = SDL_CreateTexture(
                    l_renderer_ref,
                    SDL_PIXELFORMAT_ARGB8888,
                    SDL_TEXTUREACCESS_STREAMING,
                    width, height);
    Q_ENSURE(l_texture != NULL);

    lv_disp_t * disp = lv_disp_get_default();
    if (disp) {
        disp->driver->hor_res = width;
        disp->driver->ver_res = height;
        lv_disp_drv_update(disp, disp->driver);
    }
    lv_refr_now(NULL);  /* Refresh now. */
}


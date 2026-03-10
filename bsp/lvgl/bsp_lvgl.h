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
#ifndef BSP_LVGL_H_
#define BSP_LVGL_H_

/*==========================================================================*/
/* ...Default window size. */
#define LVGL_WIND_WIDTH  1200
#define LVGL_WIND_HEIGHT 132

/*==========================================================================*/
/* ...Renderer import. */
typedef struct {
    void *renderer;
    int screen_width;
    int screen_height;
} BSP_lvgl_cfg_t;

/*==========================================================================*/
/**
 * @brief Initialization of lvgl core, display driver and input driver.
 *
 * @param[in] cfg SDL Renderer configuration type.
 */
void BSP_lvgl_init(const BSP_lvgl_cfg_t *cfg);
/*..........................................................................*/
/**
 * @brief LVGL --> SDL_Renderer.
 *
 * @note Must called between SDL_RenderClear and SDL_RenderPresent.
 */
void BSP_lvgl_render(void);
/*..........................................................................*/
/**
 * @brief LVGL timer handler.
 *
 * @param[in] dt_ms Tick passing.
 */
uint32_t BSP_lvgl_task(uint32_t dt_ms);

/*==========================================================================*/
/**
 * @brief Returns true (and clears the flag) if LVGL flushed pixels since last call.
 */
bool BSP_lvgl_is_dirty(void);

/*==========================================================================*/
/**
 * @brief Update mouse event to LVGL.
 *
 * @param x Mouse_x.
 *
 * @param y Mouse_y.
 *
 * @param[in] is_down Left click pressed/released.
 */
void BSP_lvgl_feed_mouse(int x, int y, bool is_down);

/*==========================================================================*/
/**
 * @brief Regenerate the texture of SDL.
 *
 * @param width New window's width.
 *
 * @param height New window's height.
 */
void BSP_lvgl_resize(int width, int height);

#endif  /* BSP_LVGL_H_ */

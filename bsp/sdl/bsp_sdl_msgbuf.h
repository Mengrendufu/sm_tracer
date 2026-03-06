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
#ifndef BSP_SDL_MSGBUF_H_
#define BSP_SDL_MSGBUF_H_

/*==========================================================================*/
#include <stdbool.h>
#include <stdint.h>

/*==========================================================================*/
/* ...Pixel height of the SDL message buffer panel (below LVGL). */
#define MSGBUF_PANEL_HEIGHT  362

/*==========================================================================*/
/**
 * @brief Initialize the SDL message buffer panel.
 *
 * @param[in] renderer  SDL_Renderer* (void* to avoid SDL.h dependency here).
 * @param[in] panel_y   Top-y pixel of the panel inside the window.
 * @param[in] panel_w   Width  of the panel in pixels.
 * @param[in] panel_h   Height of the panel in pixels.
 */
void BSP_msgbuf_init(void *renderer, int panel_y, int panel_w, int panel_h);

/*==========================================================================*/
/**
 * @brief Append a message string to the buffer and (optionally) scroll.
 *        Takes ownership of @p msg — caller must NOT free it.
 *
 * @param[in] msg  Heap-allocated NUL-terminated string.
 */
void BSP_msgbuf_append(char *msg);

/*==========================================================================*/
/**
 * @brief Clear all messages from the buffer.
 */
void BSP_msgbuf_clear(void);

/*==========================================================================*/
/**
 * @brief Render the message buffer panel into the SDL renderer.
 *        Must be called between SDL_RenderClear and SDL_RenderPresent.
 */
void BSP_msgbuf_render(void);

/*==========================================================================*/
/**
 * @brief Feed a mouse motion event.
 *
 * @param x, y     Window-space cursor position.
 * @param pressed  Left button held.
 */
void BSP_msgbuf_mouse_motion(int x, int y, bool pressed);

/*==========================================================================*/
/**
 * @brief Feed a mouse button event.
 *
 * @param x, y    Window-space cursor position.
 * @param down    true = button pressed, false = released.
 */
void BSP_msgbuf_mouse_button(int x, int y, bool down);

/*==========================================================================*/
/**
 * @brief Feed a mouse wheel event.
 *
 * @param dy  Positive = scroll up, negative = scroll down.
 */
void BSP_msgbuf_mouse_wheel(int dy);

/*==========================================================================*/
/**
 * @brief Copy the currently selected text to the system clipboard.
 *        Intended to be called when Ctrl+C is pressed.
 */
void BSP_msgbuf_copy_selection(void);

/*==========================================================================*/
/**
 * @brief Notify the panel that the window was resized.
 *
 * @param new_w  New window width.
 * @param new_h  New window height.
 */
void BSP_msgbuf_resize(int new_w, int new_h);

#endif  /* BSP_SDL_MSGBUF_H_ */

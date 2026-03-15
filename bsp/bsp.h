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
#ifndef BSP_H_
#define BSP_H_

/*==========================================================================*/
#define BSP_TICKS_PER_SEC  100U  /* System tick frequency. */
#include <stdbool.h>

/*==========================================================================*/
/**
 * @brief Get the window always-on-top status.
 *
 * @return bool
 */
bool BSP_windowIsAlwaysOnTop(void);
/*..........................................................................*/
/**
 * @brief Set the window always-on-top status.
 *
 * @param on_top
 */
void BSP_windowSetAlwaysOnTop(bool on_top);

/*==========================================================================*/
/**
 * @brief
 */
void BSP_init(void);
/*..........................................................................*/
/**
 * @brief
 */
void BSP_ledOff(void);
/*..........................................................................*/
/**
 * @brief
 */
void BSP_ledOn(void);

/*==========================================================================*/
/**
 * @brief Deliver the text to system clipboard.
 *
 * @param text
 */
void BSP_deliverClipboard(const char *text);

/*==========================================================================*/
#include "bsp_lvgl.h"
#include "bsp_serial_port.h"
#include "bsp_sdl_msgbuf.h"

#endif  /* BSP_H_ */

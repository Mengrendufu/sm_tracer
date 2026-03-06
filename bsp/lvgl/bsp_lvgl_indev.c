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
/**
 * @defgroup LVGL mouse event.
 *
 * @{
 */

typedef struct DeviceEvt_Mouse {
    lv_coord_t l_mouse_x;
    lv_coord_t l_mouse_y;
    bool       l_mouse_down;
} DeviceEvt_Mouse;

/* instance */
static DeviceEvt_Mouse DevieceEvt_mouse = {
    .l_mouse_x    = 0,
    .l_mouse_y    = 0,
    .l_mouse_down = false
};

/** @} */

/*==========================================================================*/
/* ...Input device initialization. */
static void indev_read_cb(lv_indev_drv_t * drv, lv_indev_data_t * data) {
    (void)drv;
    /* ...Post mouse event to LVGL. */
    data->point.x = DevieceEvt_mouse.l_mouse_x;
    data->point.y = DevieceEvt_mouse.l_mouse_y;
    data->state   = DevieceEvt_mouse.l_mouse_down ?
                                        LV_INDEV_STATE_PRESSED :
                                        LV_INDEV_STATE_RELEASED;
}
/*..........................................................................*/
void lv_port_indev_init_internal(void) {
    static lv_indev_drv_t indev_drv;

    /* Input-device initialization */
    lv_indev_drv_init(&indev_drv);

    /* Mouse or finger touch */
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = indev_read_cb;
    lv_indev_drv_register(&indev_drv);
}

/*==========================================================================*/
void BSP_lvgl_feed_mouse(int x, int y, bool is_down) {
    DevieceEvt_mouse.l_mouse_x    = (lv_coord_t)x;
    DevieceEvt_mouse.l_mouse_y    = (lv_coord_t)y;
    DevieceEvt_mouse.l_mouse_down = is_down;
}

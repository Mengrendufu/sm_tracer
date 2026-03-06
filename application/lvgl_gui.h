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
#ifndef LVGL_GUI_H_
#define LVGL_GUI_H_

/*==========================================================================*/
/**
 * @brief
 *
 * @param finish_cb
 */
void GUI_init(void);

/*==========================================================================*/
/**
 * @defgroup
 *
 * @{
 */

enum DisplayState {
    DUMMY_LED_STATE,

    /* ...Panel states。 */
    DISPLAY_STATE_CLOSED,
    DISPLAY_STATE_OPENING,
    DISPLAY_STATE_OPENED,
    DISPLAY_STATE_CLOSING,

    MAX_LED_STATE
};

/**
 * @brief
 *
 * @param[in] state
 */
void GUI_updateSerialCnnState(uint8_t state);

/** @} */

/*==========================================================================*/
/**
 * @brief
 *
 * @param comLists
 */
void GUI_updateComLists(char *comLists);

/*==========================================================================*/
/**
 * @brief
 *
 * @param fileName
 */
void GUI_updateLoadedJsonFile(char *fileName);

/*==========================================================================*/
/**
 * @brief
 *
 * @param[in] dispMsg
 */
void GUI_updateRecvBoxMsg(char *dispMsg);
/*..........................................................................*/
/**
 * @brief
 */
void GUI_msgBoxClean(void);

#endif  /* LVGL_GUI_H_ */

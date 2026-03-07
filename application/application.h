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
#ifndef APPLICATION_H_
#define APPLICATION_H_

/*==========================================================================*/
/* ===SDL main thread interface. */
enum SDL_Signals {
    SDL_SIGNALS_DUMMY = SDL_USEREVENT,

    DISP_UPDATE_COM_DROPDOWN_BOX_INFO_SIG,

    DISP_UPDATE_OPENING_STATE_SIG,
    DISP_UPDATE_OPEN_STATE_SIG,
    DISP_UPDATE_CLOSING_STATE_SIG,
    DISP_UPDATE_CLOSE_STATE_SIG,

    DISP_UPDATE_LOADED_JSON_FILE_SIG,

    DISP_UPDATE_RECV_BOX_MSG_SIG,

    SDL_SIGNALS_MAX = SDL_LASTEVENT
};
/*..........................................................................*/
/* LVGL. */
#include "lvgl_gui.h"

/*==========================================================================*/
/* ===AOs interface. */
enum AO_Signals {
    AO_SIGNALS_DUMMY = Q_USER_SIG,

    MAX_PUB_SIG, /* The last published signal. */

    /*......................................................................*/
    /* ...User Signals. */

    /* Time event... */
    TIMEOUT_SIG,

    /* AO_GuiMngr... */
    GUI_CLICK_REFRESH_BUTTON_SIG,

    GUI_CLICK_OPEN_CLOSE_BUTTON_SIG,

    GUI_UPDATE_COM_SIG,
    GUI_UPDATE_BAUDRATE_SIG,
    GUI_UPDATE_DATABITS_SIG,
    GUI_UPDATE_STOPBITS_SIG,
    GUI_UPDATE_PARITY_SIG,
    GUI_UPDATE_FLOWCONTROL_SIG,

    GUI_GET_UPDATED_COM_INFO_SIG,

    GUI_OPEN_INVALID_COM_SIG,
    GUI_OPEN_COM_SUCCESS_SIG,
    GUI_OPEN_COM_FAIL_SIG,

    GUI_MNGR_PORT_ERR_SHUT_DOWN_SIG,
    GUI_MNGR_CLOSE_COM_SUCCESS_SIG,

    GUI_MNGR_RECV_MSG_SIG,

    GUI_MNGR_UPDATE_SELECTED_JSON_FILE_SIG,
    GUI_MNGR_LOAD_SELECTED_JSON_FILE_SIG,
    GUI_MNGR_UPDATE_LOADED_JSON_FILE_SIG,

    /* AO_SpMngr... */
    SP_MNGR_REFRESH_SERIAL_PORT_SIG,

    SP_MNGR_UPDATE_COM_SELECTION_SIG,
    SP_MNGR_UPDATE_BAUDRATE_SIG,
    SP_MNGR_UPDATE_DATABITS_SIG,
    SP_MNGR_UPDATE_STOPBITS_SIG,
    SP_MNGR_UPDATE_PARITY_SIG,
    SP_MNGR_UPDATE_FLOWCONTROL_SIG,

    SP_MNGR_OPEN_SERIAL_PORT_SIG,
    SP_MNGR_CLOSE_SERIAL_PORT_SIG,

    SP_MNGR_GET_UPDATED_COM_INFO_SIG,

    SP_MNGR_OPEN_COM_SUCCESS_SIG,
    SP_MNGR_OPEN_COM_FAIL_SIG,

    SP_MNGR_PORT_ERR_SHUT_DOWN_SIG,
    SP_MNGR_CLOSE_COM_SUCCESS_SIG,

    SP_MNGR_RECV_PACKET_SIG,
    SP_MNGR_RECV_MSG_SIG,

    SP_MNGR_UPDATE_SELECTED_JSON_FILE_SIG,
    SP_MNGR_LOAD_SELECTED_JSON_FILE_SIG,

    AO_SIGNALS_MAX /* The last signal. */
};
/*..........................................................................*/
/* ...AO_Blinky. */
typedef struct {
    QEvt super;
} BlinkyEvt;
#include "blinky.h"
/*..........................................................................*/
/* ...AO_GuiMngr. */
typedef struct {
    QEvt super;
    char *txt;
} GuiMngrEvt;
#include "gui_mngr.h"
/*..........................................................................*/
/* ...AO_SpMngr. */
typedef struct {
    /* 8B. */
    QEvt super;
    /* 8B. */
    union {
        char *txt;
        char *portLst;
        uint8_t *packBuf;
        uint8_t *msgFrm;
    } pld;
    /* 4B. */
    union {
        int packSize;
        int msgFrmSize;
    } pldSize;
} SpMngrEvt;
#include "sp_mngr.h"

/*==========================================================================*/
/* ===Instantiate and start AOs/threads. */
void AO_start(void);

#endif  /* APPLICATION_H_ */

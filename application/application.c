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
void AO_start(void) {
    /* blinky... */
    static QEvtPtr blinkyQueueSto[256];
    Blinky_ctor();
    QActive_start(
		AO_Blinky,
        1U | (1U << 8),                         // QP prio. of the AO
        blinkyQueueSto, Q_DIM(blinkyQueueSto),  // event queue storage
        (void *)0, 0U,                          // no stack storage
        (void *)0);                             // no initialization param

    /* Gui Manager... */
    static QEvtPtr guiMngrQueueSto[256];
    GuiMngr_ctor();
    QActive_start(
		AO_GuiMngr,
        2U | (1U << 8),                           // QP prio. of the AO
        guiMngrQueueSto, Q_DIM(guiMngrQueueSto),  // event queue storage
        (void *)0, 0U,                            // no stack storage
        (void *)0);                               // no initialization param

    /* SerialPort Manager... */
    static QEvtPtr spMngrQueueSto[256];
    SpMngr_ctor();
    QActive_start(
        AO_SpMngr,
        3U | (1U << 8),                           // QP prio. of the AO
        spMngrQueueSto, Q_DIM(spMngrQueueSto),    // event queue storage
        (void *)0, 0U,                            // no stack storage
        (void *)0);                               // no initialization param

    /* Raw serial port... */
    BSP_Serial_Init();
}

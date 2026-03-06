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
#ifndef BSP_SERIAL_PORT_H_
#define BSP_SERIAL_PORT_H_

/*==========================================================================*/
#include "application.h"

/*==========================================================================*/
/**
 * @brief
 */
void BSP_Serial_Init(void);
/*..........................................................................*/
/**
 * @brief
 *
 * @param sig_
 *
 * @param portName
 *
 * @param baudrate
 *
 * @param dataBits
 *
 * @param stopBits
 *
 * @param parity
 *
 * @param flowControl
 */
void BSP_Post2LspThread(
    uint16_t sig_,
    const char *portName,
    int baudrate,
    int dataBits,
    SerialStopBits_t stopBits,
    SerialParity_t parity,
    SerialFlow_t flowControl);

#endif  /* BSP_SERIAL_PORT_H_ */

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
#ifndef SPTHREAD_EVT_H_
#define SPTHREAD_EVT_H_

/*==========================================================================*/
/* ===SP thread event signals. */
enum SpThread_Signals {
    DUMMY_SPTHREAD_SIGNALS,

    SP_TRD_UPDATE_SERIAL_PORTS_SIG,

    SP_TRD_OPEN_PORT_SIG,
    SP_TRD_CLOSE_PORT_SIG,

    SP_TRD_PORT_ERR_SHUT_DOWN_SIG,

    MAX_SPTHREAD_SIGNALS
};

/*==========================================================================*/
typedef enum {
    SP_STOPBITS_1_OP   = 1,
    SP_STOPBITS_2_OP   = 2,
    SP_STOPBITS_1_5_OP = 3
} SerialStopBits_t;
/*..........................................................................*/
typedef enum {
    SP_PARITY_NONE_OP = 0,
    SP_PARITY_ODD_OP,
    SP_PARITY_EVEN_OP,
    SP_PARITY_MARK_OP,
    SP_PARITY_SPACE_OP
} SerialParity_t;
/*..........................................................................*/
typedef enum {
    SP_FLOW_NONE_OP = 0,
    SP_FLOW_XON_XOFF_OP,
    SP_FLOW_RTS_CTS_OP,
    SP_FLOW_DTR_DSR_OP
} SerialFlow_t;
/*..........................................................................*/
#define SP_TRD_COM_LEN 128
typedef struct {
    char portName[SP_TRD_COM_LEN];
    int baudrate;
    int dataBits;
    SerialStopBits_t stopBits;
    SerialParity_t parity;
    SerialFlow_t flowControl;
} SerialConfig_t;

/*==========================================================================*/
typedef struct {
    uint16_t sig;
    SerialConfig_t spConfig;
} SpThreadEvt;

/*==========================================================================*/
/**
 * @brief
 *
 * @param str
 *
 * @return int
 */
int str_to_baud(const char *str);
/*..........................................................................*/
/**
 * @brief
 *
 * @param str
 *
 * @return int
 */
int str_to_databits(const char *str);
/*..........................................................................*/
/**
 *
 */
SerialStopBits_t str_to_stopbits(const char *str);
/*..........................................................................*/
/**
 *
 */
SerialParity_t str_to_parity(const char *str);
/*..........................................................................*/
/**
 *
 */
SerialFlow_t str_to_flowcontrol(const char *str);

/*==========================================================================*/
/**
 * @brief
 *
 * @param comName
 *
 * @return true
 *
 * @return false
 */
bool SpMngr_comNameIsInvalid(char *comName);

#endif  /* SPTHREAD_EVT_H_ */

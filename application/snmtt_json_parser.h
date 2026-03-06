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
#ifndef SNMTT_JSON_PARSER_H_
#define SNMTT_JSON_PARSER_H_

/*==========================================================================*/
#include "snmtt_itoa.h"

/* RECV tokens =============================================================*/
/**
 * @defgroup Args striper
 *
 * @{
 */

enum SNMTT_FormatArgs {
    /* Default invalid. */
    SNMTT_FORMATARGS_DUMMY = 0,

    /* Dec... */
    SM_INT8,
    SM_INT16, SM_INT32, SM_INT64,

    SM_UINT8, SM_UINT16, SM_UINT32, SM_UINT64,

    /* Hex... */
    SM_HEX8, SM_HEX16, SM_HEX32, SM_HEX64,

    /* Bin... */
    SM_BIN8, SM_BIN16, SM_BIN32, SM_BIN64,

    /* Float... */
    SM_F32, SM_F64,

    /* String... */
    SM_STRING,

    SNMTT_FORMAT_ARGS_MAX
};

typedef struct {
    uint8_t argLen;
    enum SNMTT_Radix radix; /* Radix of itoa */
} SNMTT_ArgFormat;

/** @} */

/**
 * @defgroup Restore the info of RX tokens:
 *           ITOA output format string;
 *           Strategy of parsing payload;
 *
 * @{
 */

#define SNMTT_ARGS_NUM_MAX 16

typedef struct SNMTT_PayloadParser {
    char *format;
    SNMTT_ArgFormat *args[SNMTT_ARGS_NUM_MAX];
} SNMTT_PayloadParser;

#define SNMTT_REC_ID_MAX 256

/**
 * @brief You receive a frame with RecID, you will want to know how to
 *        parse it.
 *
 * @param RecID
 *
 * @return SNMTT_PayloadParser *
 */
SNMTT_PayloadParser *SNMTT_RX_getRecID(int RecID);

/** @} */

/*==========================================================================*/
/**
 * @brief When you want to update your config from a JSON.
 *
 * @param jsonPath File path of a json file.
 */
void SNMTT_loadJSONConfig(char const * const jsonPath);

#endif  /* SNMTT_JSON_PARSER_H_ */

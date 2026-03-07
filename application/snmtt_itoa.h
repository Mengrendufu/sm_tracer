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
#ifndef SNMTT_ITOA_H_
#define SNMTT_ITOA_H_

/*==========================================================================*/
/**
 * @defgroup Radix of itoa implementation
 *
 * @{
 */

enum SNMTT_Radix {
    SNMTT_RADIX_DUMMY     = 0,
    SNMTT_RADIX_DEC_S     = 11,
    SNMTT_RADIX_DEC_U     = 10,
    SNMTT_RADIX_DEC_S_F32 = 32,
    SNMTT_RADIX_DEC_S_F64 = 64,
    SNMTT_RADIX_HEX       = 16,
    SNMTT_RADIX_BIN       = 2,
    SNMTT_RADIX_STRING    = 0xFF
};

/** @} */

/**
 * @brief Transform the big-endian intStack[numSize] data to string.
 *
 * @param[in] intStack Restore the byte stream of a value.
 *
 * @param[in] numSize Number of bytes of the value.
 *
 * @param[in] optRdx The format that value represents.
 *
 *                   Decimal: is like %3d,  %5d,  %10lu, %20llu;
 *
 *                                    %.7g, %.16g;
 *
 *                   Hexadecimal: is like %2X, %4X, %8X, %16X.
 *
 *                   Binary: is like %8b, %16b, %32b, %64b.
 *
 * @return char * String malloc inside, FREE is needed after used.
 */
char *SNMTT_itoa(
    uint8_t *intStack, uint8_t const numSize,
    enum SNMTT_Radix optRdx);

#endif  /* SNMTT_ITOA_H_ */

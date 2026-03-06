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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "sm_assert.h"
#include "snmtt_itoa.h"

/*==========================================================================*/
SM_DEFINE_MODULE("snmtt_itoa")

/*==========================================================================*/
char *SNMTT_itoa(
    uint8_t *valStream, uint8_t const parSize,
    enum SNMTT_Radix optRdx)
{
    char *res = (char *)0;
    uint8_t start = 0;
    uint8_t end;

    if (!((parSize == 1) || (parSize == 2)
            || (parSize == 4) || (parSize == 8)))
    {
        return res;
    }
    if (!((optRdx == SNMTT_RADIX_DEC_U) || (optRdx == SNMTT_RADIX_DEC_S)
            || (optRdx == SNMTT_RADIX_BIN) || (optRdx == SNMTT_RADIX_HEX)
            || (optRdx == SNMTT_RADIX_DEC_S_F32)
            || (optRdx == SNMTT_RADIX_DEC_S_F64)))
    {
        return res;
    }

    /**
     * Memory allocation
     */
    if (optRdx == SNMTT_RADIX_DEC_U) {
        if (parSize == 1) {
            /* 000 ~ 255 */
            end = 3 + 1;
        } else if (parSize == 2) {
            /* 00000 ~ 65535 */
            end = 5 + 1;
        } else if (parSize == 4) {
            /* 0000000000 ~ 4294967295 */
            end = 10 + 1;
        } else if (parSize == 8) {
            /* 00000000000000000000 ~ 18446744073709551615 */
            end = 20 + 1;
        }
    } else if (optRdx == SNMTT_RADIX_DEC_S) {
        if (parSize == 1) {
            /* -127 ~  127 */
            end = 3 + 1 + 1;
        } else if (parSize == 2) {
            /* -32767 ~  32767 */
            end = 5 + 1 + 1;
        } else if (parSize == 4) {
            /* -2147483647 ~  2147483647 */
            end = 10 + 1 + 1;
        } else if (parSize == 8) {
            /* -9223372036854775807 ~  9223372036854775807 */
            end = 19 + 1 + 1;
        }
    } else if (optRdx == SNMTT_RADIX_DEC_S_F32) {
        if (parSize == 4) {
            end = 16;
        } else {
            return res;  /* NULL */
        }
    } else if (optRdx == SNMTT_RADIX_DEC_S_F64) {
        if (parSize == 8) {
            end = 32;
        } else {
            return res;  /* NULL */
        }
    } else if (optRdx == SNMTT_RADIX_HEX) {
        /* 0F 32 3F 98 */
        end = 2*parSize + parSize-1 + 1;
    } else if (optRdx == SNMTT_RADIX_BIN) {
        /* 11111111'00111111'11111111'11111111 */
        end = 8*parSize + parSize-1 + 1;
    }
    res = malloc(end);
    SM_ENSURE(res != (char *)0);

    /**
     * Transfrom
     */
    if (optRdx == SNMTT_RADIX_DEC_U) {
        /* Num combination */
        uint64_t numComb = 0;
        for (uint8_t i = 0; i < parSize; ++i) {
            numComb += ((uint64_t)valStream[i]) << ((parSize - 1 - i) * 8);
        }
        if (parSize == 1) {
            snprintf(res, end, "%03u", (uint8_t)numComb);
        } else if (parSize == 2) {
            snprintf(res, end, "%05u", (uint16_t)numComb);
        } else if (parSize == 4) {
            snprintf(res, end, "%010u", (uint32_t)numComb);
        } else if (parSize == 8) {
            snprintf(res, end, "%020llu", (uint64_t)numComb);
        }
    } else if (optRdx == SNMTT_RADIX_DEC_S) {
        /* Num combination */
        uint64_t numComb = 0;
        for (uint8_t i = 0; i < parSize; ++i) {
            numComb += ((uint64_t)valStream[i]) << ((parSize - 1 - i) * 8);
        }
        if (parSize == 1) {
            snprintf(res, end, "%+04d", (int8_t)numComb);
        } else if (parSize == 2) {
            snprintf(res, end, "%+06d", (int16_t)numComb);
        } else if (parSize == 4) {
            snprintf(res, end, "%+011d", (int32_t)numComb);
        } else if (parSize == 8) {
            snprintf(res, end, "%+020lld", (int64_t)numComb);
        }
    } else if (optRdx == SNMTT_RADIX_DEC_S_F32) {
        union {
            uint32_t u32;
            float    f32;
        } f32Cvt;
        f32Cvt.u32 = 0;
        for (uint8_t i = 0; i < parSize; ++i) {
            f32Cvt.u32 += ((uint32_t)valStream[i]) << ((parSize - 1 - i) * 8);
        }
        snprintf(res, end, "%.7g", f32Cvt.f32);
    } else if (optRdx == SNMTT_RADIX_DEC_S_F64) {
        union {
            uint64_t u64;
            double d64;
        } f64Cvt;
        f64Cvt.u64 = 0;
        for (uint8_t i = 0; i < parSize; ++i) {
            f64Cvt.u64 += ((uint64_t)valStream[i]) << ((parSize - 1 - i) * 8);
        }
        snprintf(res, end, "%.16g", f64Cvt.d64);
    } else if (optRdx == SNMTT_RADIX_HEX) {
        static char halfByteToA[] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'A', 'B', 'C', 'D', 'E', 'F'
        };
        for (int i = start; i < parSize; ++i) {
            res[start++] = halfByteToA[(valStream[i] >> 4) & 0x0F];
            res[start++] = halfByteToA[(valStream[i] >> 0) & 0x0F];
            res[start++] = ' ';
        }
        res[start > 0 ? start - 1 : 0] = '\0';  /* Make it beautiful */
    } else if (optRdx == SNMTT_RADIX_BIN) {
        for (int i = start; i < parSize; ++i) {
            res[start++] = ((valStream[i] >> 7) & 0x01) ? '1' : '0';
            res[start++] = ((valStream[i] >> 6) & 0x01) ? '1' : '0';
            res[start++] = ((valStream[i] >> 5) & 0x01) ? '1' : '0';
            res[start++] = ((valStream[i] >> 4) & 0x01) ? '1' : '0';
            res[start++] = ((valStream[i] >> 3) & 0x01) ? '1' : '0';
            res[start++] = ((valStream[i] >> 2) & 0x01) ? '1' : '0';
            res[start++] = ((valStream[i] >> 1) & 0x01) ? '1' : '0';
            res[start++] = ((valStream[i] >> 0) & 0x01) ? '1' : '0';
            res[start++] = '\'';
        }
        res[start > 0 ? start - 1 : 0] = '\0';  /* Make it beautiful */
    }
    return res;
}

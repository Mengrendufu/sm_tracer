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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <dirent.h>  /* Directory. */
#include "sm_assert.h"
#include "snmtt_itoa.h"
#include "cJSON.h"  /* cJSON. */
#include "snmtt_json_parser.h"

/*==========================================================================*/
SM_DEFINE_MODULE("json_parser")

/*==========================================================================*/
/**
 * @struct enum ParserState of JsonContentStriper
 */
typedef enum ParserState {
    STATE_NORMAL,          // 普通模式：正在读取 JSON 结构或空白
    STATE_IN_STRING,       // 字符串模式：正在 "..." 内部
    STATE_LINE_COMMENT,    // 行注释模式：正在 // ... 内部
    STATE_BLOCK_COMMENT    // 块注释模式：正在 /* ... */ 内部
} ParserState;

/**
 * @brief 智能去除 (in-place) JSON 字符串中的 C 风格注释 (// 和 / * * /)
 *
 * @note 处理逻辑：
 *       1. 能够识别双引号字符串，保护字符串内部的 // 和 / *
 *       2. 能够识别字符串内的转义引号 \"
 *       3. 将注释内容替换为 ' ' (空格)，而不是删除，以保持报错时的列号偏移量
 *       4. 保留块注释内的换行符 \n，以保持报错时的行号准确
 *
 * @param[in] json_str 原始 JSON 字符串 (会直接在原内存上修改)
 */
static void jsonContentStrip(char *json_str) {
    if (json_str == NULL) {
        return;
    }
    char *p = json_str;
    ParserState state = STATE_NORMAL;
    while (*p != '\0') {
        switch (state) {
            // =========================================================
            // case 0: 普通模式 (寻找入口)
            // =========================================================
            case STATE_NORMAL:
                if (*p == '\"') {
                    state = STATE_IN_STRING;
                    p++;
                }
                else if (*p == '/' && *(p + 1) == '/') {
                    state = STATE_LINE_COMMENT;
                    *p = ' ';       // 清除 /
                    *(p + 1) = ' '; // 清除 /
                    p += 2;
                }
                else if (*p == '/' && *(p + 1) == '*') {
                    state = STATE_BLOCK_COMMENT;
                    *p = ' ';       // 清除 /
                    *(p + 1) = ' '; // 清除 *
                    p += 2;
                }
                else {
                    p++; // 普通字符，跳过
                }
                break;
            // =========================================================
            // case 1: 字符串内部 "..."
            // =========================================================
            case STATE_IN_STRING:
                if (*p == '\\') {
                    // 遇到转义符 (如 \" 或 \\)，直接跳过当前和下一个字符
                    p++;
                    if (*p != '\0') p++;
                }
                else if (*p == '\"') {
                    // 遇到结束引号，回归普通模式
                    state = STATE_NORMAL;
                    p++;
                }
                else {
                    p++;
                }
                break;
            // =========================================================
            // case 2: 行注释内部 // ...
            // =========================================================
            case STATE_LINE_COMMENT:
                if (*p == '\n') {
                    // 遇到换行，保留换行符，回归普通模式
                    state = STATE_NORMAL;
                    p++;
                }
                else {
                    *p = ' '; // 抹除注释内容
                    p++;
                }
                break;
            // =========================================================
            // case 3: 块注释内部 /* ... */
            // =========================================================
            case STATE_BLOCK_COMMENT:
                if (*p == '*' && *(p + 1) == '/') {
                    // 遇到结束符 */，清除并回归
                    state = STATE_NORMAL;
                    *p = ' ';       // 清除 *
                    *(p + 1) = ' '; // 清除 /
                    p += 2;
                }
                else {
                    // 核心细节：保留换行符以维持行号
                    if (*p != '\n') {
                        *p = ' ';
                    }
                    p++;
                }
                break;
        }
    }
}

/**
 * @brief Read the content of .json file and return it to the heap memory.
 *        The Json file can be json with comment, inside the function
 *        jsonContentStrip clean all the comments.
 *
 * @param[in] filename Path of .json file.
 *
 * @return char * Pointer to the memory holding the content of .json file.
 *
 * @sa jsonContentStrip
 */
static char *SNMTT_loadJson(char const * const filename) {
    FILE *f = fopen(filename, "rb");
    if (f == (FILE *)0) return (char *)0;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    /* Memory alloc and reset to 0 */
    char *buf = calloc(1, len + 1);
    SM_ENSURE(buf != (char *)0);
    fread(buf, 1, len, f);
    fclose(f);
    jsonContentStrip(buf);  /* In-place */
    return buf;
}

/**
 * @brief
 *
 * @param jsonPath
 *
 * @return cJSON *
 */
static cJSON *SNMTT_getCJSONRoot(char const * const jsonPath) {
    cJSON *cjsonRoot = NULL;
    char *prtcStr = SNMTT_loadJson(jsonPath);
    if (prtcStr) {  /* Valid file content */
        cjsonRoot = cJSON_Parse(prtcStr);
        free(prtcStr);
    }
    return cjsonRoot;
}

/* RECV tokens =============================================================*/
static SNMTT_ArgFormat SNMTT_argFmtTbl_[SNMTT_FORMAT_ARGS_MAX] = {
    /* Dummy */
    { 0U, SNMTT_RADIX_DUMMY },

    /* DEC_signed_int... */
    { 1U, SNMTT_RADIX_DEC_S },
    { 2U, SNMTT_RADIX_DEC_S },
    { 4U, SNMTT_RADIX_DEC_S },
    { 8U, SNMTT_RADIX_DEC_S },

    /* DEC_unsigned_int */
    { 1U, SNMTT_RADIX_DEC_U },
    { 2U, SNMTT_RADIX_DEC_U },
    { 4U, SNMTT_RADIX_DEC_U },
    { 8U, SNMTT_RADIX_DEC_U },

    /* HEX... */
    { 1U, SNMTT_RADIX_HEX},
    { 2U, SNMTT_RADIX_HEX},
    { 4U, SNMTT_RADIX_HEX},
    { 8U, SNMTT_RADIX_HEX},

    /* BIN... */
    { 1U, SNMTT_RADIX_HEX },
    { 2U, SNMTT_RADIX_HEX },
    { 4U, SNMTT_RADIX_HEX },
    { 8U, SNMTT_RADIX_HEX },

    /* DEC_float... */
    { 4U, SNMTT_RADIX_DEC_S_F32 },
    { 4U, SNMTT_RADIX_DEC_S_F64 },

    /* String... */
    { 0xFF, SNMTT_RADIX_STRING }
};

static SNMTT_ArgFormat SNMTT_getArgFmt(char *argString) {
    if (!strcmp(argString, "INT8")) {
        return SNMTT_argFmtTbl_[SM_INT8];
    } else if (!strcmp(argString, "INT16")) {
        return SNMTT_argFmtTbl_[SM_INT16];
    } else if (!strcmp(argString, "INT32")) {
        return SNMTT_argFmtTbl_[SM_INT32];
    } else if (!strcmp(argString, "INT64")) {
        return SNMTT_argFmtTbl_[SM_INT64];
    } else if (!strcmp(argString, "UINT8")) {
        return SNMTT_argFmtTbl_[SM_UINT8];
    } else if (!strcmp(argString, "UINT16")) {
        return SNMTT_argFmtTbl_[SM_UINT16];
    } else if (!strcmp(argString, "UINT32")) {
        return SNMTT_argFmtTbl_[SM_UINT32];
    } else if (!strcmp(argString, "UINT64")) {
        return SNMTT_argFmtTbl_[SM_UINT64];
    } else if (!strcmp(argString, "HEX8")) {
        return SNMTT_argFmtTbl_[SM_HEX8];
    } else if (!strcmp(argString, "HEX16")) {
        return SNMTT_argFmtTbl_[SM_HEX16];
    } else if (!strcmp(argString, "HEX32")) {
        return SNMTT_argFmtTbl_[SM_HEX32];
    } else if (!strcmp(argString, "HEX64")) {
        return SNMTT_argFmtTbl_[SM_HEX64];
    } else if (!strcmp(argString, "BIN8")) {
        return SNMTT_argFmtTbl_[SM_BIN8];
    } else if (!strcmp(argString, "BIN16")) {
        return SNMTT_argFmtTbl_[SM_BIN16];
    } else if (!strcmp(argString, "BIN32")) {
        return SNMTT_argFmtTbl_[SM_BIN32];
    } else if (!strcmp(argString, "BIN64")) {
        return SNMTT_argFmtTbl_[SM_BIN64];
    } else if (!strcmp(argString, "F32")) {
        return SNMTT_argFmtTbl_[SM_F32];
    } else if (!strcmp(argString, "F64")) {
        return SNMTT_argFmtTbl_[SM_F64];
    } else if (!strcmp(argString, "STRING")) {
        return SNMTT_argFmtTbl_[SM_STRING];
    } else {  /* DEFAULT */
        return SNMTT_argFmtTbl_[SNMTT_FORMATARGS_DUMMY];
    }
}

/**
 * @defgroup How to parse payload
 *
 * @{
 */

static SNMTT_PayloadParser *SNMTT_PayloadParser_inst[SNMTT_REC_ID_MAX];

/** @} */

/**
 * @brief
 *
 * @param node
 */
static void SNMTT_pldPsrNodeDestroy(SNMTT_PayloadParser **node) {
    if ((node != (SNMTT_PayloadParser **)0)
        && (node[0] != (SNMTT_PayloadParser *)0))
    {
        /* char *format */
        if (node[0]->format != (char *)0) {
            free(node[0]->format);
            node[0]->format = (char *)0;
        }
        /* SNMTT_ArgFormat *args[SNMTT_ARGS_NUM_MAX] */
        for (int i = 0; i < SNMTT_ARGS_NUM_MAX; ++i) {
            if (node[0]->args[i]) {
                free(node[0]->args[i]);
                node[0]->args[i] = NULL;
            }
        }
        free(node[0]);
        node[0] = (SNMTT_PayloadParser *)0;
    }
}

/**
 * @brief
 *
 * @param pldPsrInst
 */
static void SNMTT_PayloadParser_destroy(SNMTT_PayloadParser **pldPsrInst) {
    /* Clear pldPsrInst */
    for (int i = 0; i < SNMTT_REC_ID_MAX; ++i) {
        SNMTT_pldPsrNodeDestroy(pldPsrInst + i);
    }
}

/**
 * @brief Only "%%" and "%s" is allowed.
 *
 * @param fmt Format string from JSON.
 *
 * @param[out] formatNum
 *
 * @return true
 *
 * @return false
 */
static bool SNMTT_formatIsLegal(const char *fmt, int *formatNum) {
    while (*fmt) {
        if (*fmt == '%') {
            ++fmt;
            if (*fmt != 's' && *fmt != '%') return false;
            if (*fmt == 's')                *formatNum = *formatNum + 1;
        }
        ++fmt;
    }
    return true;
}

/**
 * @brief
 *
 * @param[out] pldParser
 *
 * @param[in] root
 */
static void SNMTT_PayloadParser_update(SNMTT_PayloadParser **pldParser,
                                       cJSON *root)
{
    /* Clear pldParser */
    SNMTT_PayloadParser_destroy(pldParser);
    /* Update JSON to local */
    if (root != (cJSON *)0) {
        /* RX_tokens (Array) */
        cJSON *rx_tokens = cJSON_GetObjectItem(root, "RX_tokens");
        if (rx_tokens && cJSON_IsArray(rx_tokens)) {
            cJSON *rx_tokens_item = NULL;
            /* for rx_tokens_item in rx_tokens: */
            cJSON_ArrayForEach(rx_tokens_item, rx_tokens) {
                if (rx_tokens_item && cJSON_IsObject(rx_tokens_item)) {
                    /* Get RecID */
                    cJSON *RecID = cJSON_GetObjectItem(
                                                    rx_tokens_item, "RecID");
                    if (RecID && cJSON_IsNumber(RecID)) {
                        /* Check the range of RecID */
                        if (!((0 <= RecID->valueint)
                                && (RecID->valueint < SNMTT_REC_ID_MAX)))
                        {
                            /* Nothing to free currently */
                            continue;  /* Give up this */
                        }
                        /* Node allocation and into bucket */
                        SNMTT_PayloadParser *node =
                                    calloc(1, sizeof(SNMTT_PayloadParser));
                        SM_ENSURE(node != (SNMTT_PayloadParser *)0);
                        pldParser[RecID->valueint] = node;
                        /* Update format */
                        cJSON *format = cJSON_GetObjectItem(
                                                    rx_tokens_item, "format");
                        int formatNum = 0;
                        if (format && cJSON_IsString(format)) {
                            if (!SNMTT_formatIsLegal(
                                            format->valuestring, &formatNum))
                            {
                                /* Format string invalid */
                                SNMTT_pldPsrNodeDestroy(&node);
                                pldParser[RecID->valueint] =
                                                    (SNMTT_PayloadParser *)0;
                                continue;
                            }
                            node->format = strdup(format->valuestring);
                            SM_ENSURE(node->format != (char *)0);
                        }
                        /* Update args */
                        cJSON *args = cJSON_GetObjectItem(
                                                    rx_tokens_item, "args");
                        if (args && cJSON_IsArray(args)) {
                            cJSON *args_item = NULL;
                            uint8_t argFillIdx = 0;
                            bool configArgTypeInvalid = false;
                            cJSON_ArrayForEach(args_item, args) {
                                if (args_item && cJSON_IsString(args_item)) {
                                    SNMTT_ArgFormat argFmtRet =
                                                SNMTT_getArgFmt(
                                                    args_item->valuestring);
                                    /* Invalid dummy... */
                                    if (argFmtRet.argLen == 0
                                            && argFmtRet.radix ==
                                                        SNMTT_RADIX_DUMMY)
                                    {
                                        /* Give up this recID config */
                                        SNMTT_pldPsrNodeDestroy(&node);
                                        pldParser[RecID->valueint] =
                                                    (SNMTT_PayloadParser *)0;
                                        configArgTypeInvalid = true;
                                        break;
                                    }
                                    SNMTT_ArgFormat *argNode =
                                            malloc(sizeof(SNMTT_ArgFormat));
                                    argNode->argLen = argFmtRet.argLen;
                                    argNode->radix  = argFmtRet.radix;
                                    pldParser[RecID->valueint]->
                                                args[argFillIdx++] = argNode;
                                }
                            }
                            /* Next RecID */
                            if (configArgTypeInvalid) continue;
                            /* Post condition guarding */
                            if (argFillIdx != formatNum) {
                                SNMTT_pldPsrNodeDestroy(&node);
                                pldParser[RecID->valueint] =
                                            (SNMTT_PayloadParser *)0;
                            }
                        }
                    }
                }
            }
        }
        /* Recursively memory free */
        cJSON_Delete(root);
    }
}
/*..........................................................................*/
SNMTT_PayloadParser *SNMTT_RX_getRecID(int RecID) {
    return SNMTT_PayloadParser_inst[RecID];
}

/* JSON config load ========================================================*/
void SNMTT_loadJSONConfig(char const * const jsonPath) {
    cJSON *root = SNMTT_getCJSONRoot(jsonPath);
    /* RECV load */
    SNMTT_PayloadParser_update((SNMTT_PayloadParser * *)
                                    SNMTT_PayloadParser_inst,
                               root);
}

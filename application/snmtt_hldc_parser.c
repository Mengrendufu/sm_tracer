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
#include "sm_port.h"
#include "sm_assert.h"
#include "sm_hsm.h"
#include "snmtt_hldc_parser.h"

/*==========================================================================*/
// SM_DEFINE_MODULE("hdlc_parser")

/*==========================================================================*/
/**
 * @defgroup State machine event & signals
 *
 * @{
 */
enum HLDCPaserSignals {
    HLDC_PARSER_SIGNALS_DUMMY_ = 0,

    INPUT_FRAME_FLAG_SIG,
    INPUT_ESC_SIG,
    INPUT_NORMAL_SIG,

    HLDC_PARSER_SIGNALS_MAX
};

typedef struct {
    uint8_t sig;
    uint8_t currByte;
} HLDCParserEvt;
#define HLDC_EVT_SIG_(evt_) (((HLDCParserEvt *)(evt_))->sig)

/** @} */

/**
 * @defgroup HLDCParser struct
 *
 * @{
 */
typedef struct HLDCParser {
    SM_Hsm sm_hsm_;  /* State machine */
    VC_Handler init;
    VC_Handler dispatch;

    uint8_t  frmBuf[1024];
    uint16_t frmBufCnt;
    uint8_t  currDataLen;
    uint8_t  chksumIter;
} HLDCParser;

/* Instance */
HLDCParser HLDCParser_inst;

/** @} */

/*==========================================================================*/
/* UML-Top-Initial */
static SM_StatePtr HLDCParser_TOP_initial_(SM_Hsm * const me) SM_HSM_RETT;
/* State-idle */
static SM_RetState HLDCParser_idle_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT;
SM_HsmState SM_HSM_ROM HLDCParser_idle = {
    /* super   */ (SM_StatePtr)NULL,
    /* init_   */ (SM_InitHandler)NULL,
    /* entry_  */ (SM_ActionHandler)NULL,
    /* exit_   */ (SM_ActionHandler)NULL,
    /* handler */ (SM_StateHandler)&HLDCParser_idle_
};
/* State-active */
static SM_StatePtr HLDCParser_active_init_(SM_Hsm * const me) SM_HSM_RETT;
static SM_RetState HLDCParser_active_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT;
SM_HsmState SM_HSM_ROM HLDCParser_active = {
    /* super   */ (SM_StatePtr)NULL,
    /* init_   */ (SM_InitHandler)&HLDCParser_active_init_,
    /* entry_  */ (SM_ActionHandler)NULL,
    /* exit_   */ (SM_ActionHandler)NULL,
    /* handler */ (SM_StateHandler)&HLDCParser_active_
};
/* State-waitSeq */
static SM_RetState HLDCParser_waitSeq_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT;
SM_HsmState SM_HSM_ROM HLDCParser_waitSeq = {
    /* super   */ (SM_StatePtr)&HLDCParser_active,
    /* init_   */ (SM_InitHandler)NULL,
    /* entry_  */ (SM_ActionHandler)NULL,
    /* exit_   */ (SM_ActionHandler)NULL,
    /* handler */ (SM_StateHandler)&HLDCParser_waitSeq_
};
/* State-waitSeq_escMode */
static SM_RetState HLDCParser_waitSeq_escMode_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT;
SM_HsmState SM_HSM_ROM HLDCParser_waitSeq_escMode = {
    /* super   */ (SM_StatePtr)&HLDCParser_waitSeq,
    /* init_   */ (SM_InitHandler)NULL,
    /* entry_  */ (SM_ActionHandler)NULL,
    /* exit_   */ (SM_ActionHandler)NULL,
    /* handler */ (SM_StateHandler)&HLDCParser_waitSeq_escMode_
};
/* State-waitRecId */
static SM_RetState HLDCParser_waitRecId_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT;
SM_HsmState SM_HSM_ROM HLDCParser_waitRecId = {
    /* super   */ (SM_StatePtr)&HLDCParser_active,
    /* init_   */ (SM_InitHandler)NULL,
    /* entry_  */ (SM_ActionHandler)NULL,
    /* exit_   */ (SM_ActionHandler)NULL,
    /* handler */ (SM_StateHandler)&HLDCParser_waitRecId_
};
/* State-waitRecId_escMode */
static SM_RetState HLDCParser_waitRecId_escMode_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT;
SM_HsmState SM_HSM_ROM HLDCParser_waitRecId_escMode = {
    /* super   */ (SM_StatePtr)&HLDCParser_waitRecId,
    /* init_   */ (SM_InitHandler)NULL,
    /* entry_  */ (SM_ActionHandler)NULL,
    /* exit_   */ (SM_ActionHandler)NULL,
    /* handler */ (SM_StateHandler)&HLDCParser_waitRecId_escMode_
};
/* State-waitDataLen */
static SM_RetState HLDCParser_waitDataLen_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT;
SM_HsmState SM_HSM_ROM HLDCParser_waitDataLen = {
    /* super   */ (SM_StatePtr)&HLDCParser_active,
    /* init_   */ (SM_InitHandler)NULL,
    /* entry_  */ (SM_ActionHandler)NULL,
    /* exit_   */ (SM_ActionHandler)NULL,
    /* handler */ (SM_StateHandler)&HLDCParser_waitDataLen_
};
/* State-waitDataLen_escMode */
static SM_RetState HLDCParser_waitDataLen_escMode_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT;
SM_HsmState SM_HSM_ROM HLDCParser_waitDataLen_escMode = {
    /* super   */ (SM_StatePtr)&HLDCParser_waitDataLen,
    /* init_   */ (SM_InitHandler)NULL,
    /* entry_  */ (SM_ActionHandler)NULL,
    /* exit_   */ (SM_ActionHandler)NULL,
    /* handler */ (SM_StateHandler)&HLDCParser_waitDataLen_escMode_
};
/* State-waitPayload */
static SM_RetState HLDCParser_waitPayload_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT;
SM_HsmState SM_HSM_ROM HLDCParser_waitPayload = {
    /* super   */ (SM_StatePtr)&HLDCParser_active,
    /* init_   */ (SM_InitHandler)NULL,
    /* entry_  */ (SM_ActionHandler)NULL,
    /* exit_   */ (SM_ActionHandler)NULL,
    /* handler */ (SM_StateHandler)&HLDCParser_waitPayload_
};
/* State-waitPayload_escMode */
static SM_RetState HLDCParser_waitPayload_escMode_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT;
SM_HsmState SM_HSM_ROM HLDCParser_waitPayload_escMode = {
    /* super   */ (SM_StatePtr)&HLDCParser_waitPayload,
    /* init_   */ (SM_InitHandler)NULL,
    /* entry_  */ (SM_ActionHandler)NULL,
    /* exit_   */ (SM_ActionHandler)NULL,
    /* handler */ (SM_StateHandler)&HLDCParser_waitPayload_escMode_
};
/* State-waitChksum */
static SM_RetState HLDCParser_waitChksum_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT;
SM_HsmState SM_HSM_ROM HLDCParser_waitChksum = {
    /* super   */ (SM_StatePtr)&HLDCParser_active,
    /* init_   */ (SM_InitHandler)NULL,
    /* entry_  */ (SM_ActionHandler)NULL,
    /* exit_   */ (SM_ActionHandler)NULL,
    /* handler */ (SM_StateHandler)&HLDCParser_waitChksum_
};
/* State-waitChksum_escMode */
static SM_RetState HLDCParser_waitChksum_escMode_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT;
SM_HsmState SM_HSM_ROM HLDCParser_waitChksum_escMode = {
    /* super   */ (SM_StatePtr)&HLDCParser_waitChksum,
    /* init_   */ (SM_InitHandler)NULL,
    /* entry_  */ (SM_ActionHandler)NULL,
    /* exit_   */ (SM_ActionHandler)NULL,
    /* handler */ (SM_StateHandler)&HLDCParser_waitChksum_escMode_
};

/*==========================================================================*/
/* Virtual functions */
static void HLDCParser_init(HLDCParser * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    (void)e;
    /**
     * Translate *me.
     */
    SM_Hsm_init_(&me->sm_hsm_, (SM_InitHandler)HLDCParser_TOP_initial_);
}
static void HLDCParser_dispatch(HLDCParser * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    /**
     * Translate *me and *e.
     */
    SM_Hsm_dispatch_(&me->sm_hsm_, e);
}
/* Constructor */
void HLDCPaser_ctor(void) {
    HLDCParser *me = &HLDCParser_inst;
    me->init = (VC_Handler)HLDCParser_init;
    me->dispatch = (VC_Handler)HLDCParser_dispatch;

    me->frmBufCnt = 0;
    me->chksumIter = 0;

    /* Top initial tran */
    (*HLDCParser_inst.init)(&HLDCParser_inst, (void *)0);
}
/*..........................................................................*/
static bool HLDCParser_isFrameFlag(uint8_t byte_) {
    return byte_ == 0x7E;
}
static bool HLDCParser_isEsc(uint8_t byte_) {
    return byte_ == 0x7D;
}

static uint8_t HLDCParser_execEsc(uint8_t byte_) {
    return (byte_ ^ 0x20);
}

static void HLDCParser_iterChksum(HLDCParser *me, uint8_t byte_) {
    me->chksumIter += byte_;
}
static bool HLDCParser_execChksumValid(HLDCParser *me, uint8_t chksum) {
    return ((me->chksumIter + chksum) == 0xFF);
}

static void HLDCParser_cleanFrameContext(HLDCParser *me) {
    me->frmBufCnt = 0;
    me->chksumIter = 0;
}

static void HLDCParser_buildFrame(HLDCParser *me, uint8_t byte_) {
    me->frmBuf[me->frmBufCnt++] = byte_;
    HLDCParser_iterChksum(me, byte_);
}

static void HLDCParser_updateDataLen(HLDCParser *me, uint8_t byte_) {
    me->currDataLen = byte_;
}

static void HLDCParser_iterCtrPayload(HLDCParser *me) {
    --me->currDataLen;
}
static bool HLDCParser_checkPayloadDone(HLDCParser *me) {
    return (me->currDataLen == 0);
}

static void HLDCParser_frameError(HLDCParser *me) {
    (void)me;
}
static void HLDCParser_frameProcess(HLDCParser *me) {
    HLDCParser_frmPost(me->frmBuf);
}

/*==========================================================================*/
/* UML-Top-Initial */
static SM_StatePtr HLDCParser_TOP_initial_(SM_Hsm * const me) SM_HSM_RETT {
    (void)me;
    return _SM_INIT(&HLDCParser_idle);
}
/* State-idle */
static SM_RetState HLDCParser_idle_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    (void)me;
    switch (HLDC_EVT_SIG_(e)) {
        case INPUT_FRAME_FLAG_SIG: {
            return _SM_TRAN(&HLDCParser_active);
        }
        default: {
            return _SM_SUPER();
        }
    }
}
/* State-active */
static SM_StatePtr HLDCParser_active_init_(SM_Hsm * const me) SM_HSM_RETT {
    (void)me;
    HLDCParser_cleanFrameContext(containerof(me, HLDCParser, sm_hsm_));  /* Clean parser context. */
    return _SM_INIT(&HLDCParser_waitSeq);
}
static SM_RetState HLDCParser_active_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    switch (HLDC_EVT_SIG_(e)) {
        case INPUT_FRAME_FLAG_SIG: {
            return _SM_TRAN(&HLDCParser_active);  /* Self-tran */
        }
        default: {
            return _SM_SUPER();
        }
    }
}
/* State-waitSeq */
static SM_RetState HLDCParser_waitSeq_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    switch (HLDC_EVT_SIG_(e)) {
        case INPUT_ESC_SIG: {
            return _SM_TRAN(&HLDCParser_waitSeq_escMode);
        }
        case INPUT_NORMAL_SIG: {
            HLDCParser_buildFrame(containerof(me, HLDCParser, sm_hsm_),
                                  ((HLDCParserEvt *)e)->currByte);
            return _SM_TRAN(&HLDCParser_waitRecId);
        }
        default: {
            return _SM_SUPER();
        }
    }
}
/* State-waitSeq_escMode */
static SM_RetState HLDCParser_waitSeq_escMode_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    switch (HLDC_EVT_SIG_(e)) {
        case INPUT_ESC_SIG: {
            return _SM_TRAN(&HLDCParser_idle);
        }
        case INPUT_NORMAL_SIG: {
            HLDCParser_buildFrame(containerof(me, HLDCParser, sm_hsm_),
                                  HLDCParser_execEsc(
                                             ((HLDCParserEvt *)e)->currByte));
            return _SM_TRAN(&HLDCParser_waitRecId);
        }
        default: {
            return _SM_SUPER();
        }
    }
}
/* State-waitRecId */
static SM_RetState HLDCParser_waitRecId_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    switch (HLDC_EVT_SIG_(e)) {
        case INPUT_ESC_SIG: {
            return _SM_TRAN(&HLDCParser_waitRecId_escMode);
        }
        case INPUT_NORMAL_SIG: {
            HLDCParser_buildFrame(containerof(me, HLDCParser, sm_hsm_),
                                  ((HLDCParserEvt *)e)->currByte);
            return _SM_TRAN(&HLDCParser_waitDataLen);
        }
        default: {
            return _SM_SUPER();
        }
    }
}
/* State-waitRecId_escMode */
static SM_RetState HLDCParser_waitRecId_escMode_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    switch (HLDC_EVT_SIG_(e)) {
        case INPUT_ESC_SIG: {
            return _SM_TRAN(&HLDCParser_idle);
        }
        case INPUT_NORMAL_SIG: {
            HLDCParser_buildFrame(containerof(me, HLDCParser, sm_hsm_),
                                  HLDCParser_execEsc(
                                             ((HLDCParserEvt *)e)->currByte));
            return _SM_TRAN(&HLDCParser_waitDataLen);
        }
        default: {
            return _SM_SUPER();
        }
    }
}
/* State-waitDataLen */
static SM_RetState HLDCParser_waitDataLen_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    switch (HLDC_EVT_SIG_(e)) {
        case INPUT_ESC_SIG: {
            return _SM_TRAN(&HLDCParser_waitDataLen_escMode);
        }
        case INPUT_NORMAL_SIG: {
            HLDCParser_buildFrame(containerof(me, HLDCParser, sm_hsm_),
                                  ((HLDCParserEvt *)e)->currByte);
            HLDCParser_updateDataLen(containerof(me, HLDCParser, sm_hsm_),
                                     ((HLDCParserEvt *)e)->currByte);
            if (HLDCParser_checkPayloadDone(containerof(me, HLDCParser, sm_hsm_))) {
                return _SM_TRAN(&HLDCParser_waitChksum);
            } else {
                return _SM_TRAN(&HLDCParser_waitPayload);
            }
        }
        default: {
            return _SM_SUPER();
        }
    }
}
/* State-waitDataLen_escMode */
static SM_RetState HLDCParser_waitDataLen_escMode_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    switch (HLDC_EVT_SIG_(e)) {
        case INPUT_ESC_SIG: {
            return _SM_TRAN(&HLDCParser_idle);
        }
        case INPUT_NORMAL_SIG: {
            HLDCParser_buildFrame(containerof(me, HLDCParser, sm_hsm_),
                                  ((HLDCParserEvt *)e)->currByte);
            HLDCParser_updateDataLen(containerof(me, HLDCParser, sm_hsm_),
                                     HLDCParser_execEsc(
                                             ((HLDCParserEvt *)e)->currByte));
            if (HLDCParser_checkPayloadDone(containerof(me, HLDCParser, sm_hsm_))) {
                return _SM_TRAN(&HLDCParser_waitChksum);
            } else {
                return _SM_TRAN(&HLDCParser_waitPayload);
            }
        }
        default: {
            return _SM_SUPER();
        }
    }
}
/* State-waitPayload */
static SM_RetState HLDCParser_waitPayload_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    switch (HLDC_EVT_SIG_(e)) {
        case INPUT_ESC_SIG: {
            return _SM_TRAN(&HLDCParser_waitPayload_escMode);
        }
        case INPUT_NORMAL_SIG: {
            HLDCParser_buildFrame(containerof(me, HLDCParser, sm_hsm_),
                                  ((HLDCParserEvt *)e)->currByte);
            HLDCParser_iterCtrPayload(containerof(me, HLDCParser, sm_hsm_));
            if (HLDCParser_checkPayloadDone(containerof(me, HLDCParser, sm_hsm_))) {
                return _SM_TRAN(&HLDCParser_waitChksum);
            } else {
                return _SM_HANDLED();
            }
        }
        default: {
            return _SM_SUPER();
        }
    }
}
/* State-waitPayload_escMode */
static SM_RetState HLDCParser_waitPayload_escMode_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    switch (HLDC_EVT_SIG_(e)) {
        case INPUT_ESC_SIG: {
            return _SM_TRAN(&HLDCParser_idle);
        }
        case INPUT_NORMAL_SIG: {
            HLDCParser_buildFrame(containerof(me, HLDCParser, sm_hsm_),
                                  HLDCParser_execEsc(
                                             ((HLDCParserEvt *)e)->currByte));
            HLDCParser_iterCtrPayload(containerof(me, HLDCParser, sm_hsm_));
            if (HLDCParser_checkPayloadDone(containerof(me, HLDCParser, sm_hsm_))) {
                return _SM_TRAN(&HLDCParser_waitChksum);
            } else {
                return _SM_TRAN(&HLDCParser_waitPayload); /* Back to parent */
            }
        }
        default: {
            return _SM_SUPER();
        }
    }
}
/* State-waitChksum */
static SM_RetState HLDCParser_waitChksum_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    switch (HLDC_EVT_SIG_(e)) {
        case INPUT_ESC_SIG: {
            return _SM_TRAN(&HLDCParser_waitChksum_escMode);
        }
        case INPUT_NORMAL_SIG: {
            /**
             * Valid checksum, local iterated VS received
             */
            if (HLDCParser_execChksumValid(containerof(me, HLDCParser, sm_hsm_),
                                           ((HLDCParserEvt *)e)->currByte))
            {
                HLDCParser_frameProcess(containerof(me, HLDCParser, sm_hsm_));
            } else {
                HLDCParser_frameError(containerof(me, HLDCParser, sm_hsm_));
            }
            return _SM_TRAN(&HLDCParser_active);
        }
        default: {
            return _SM_SUPER();
        }
    }
}
/* State-waitChksum_escMode */
static SM_RetState HLDCParser_waitChksum_escMode_(SM_Hsm * const me, HLDCParserEvt const * const e) SM_HSM_RETT {
    switch (HLDC_EVT_SIG_(e)) {
        case INPUT_ESC_SIG: {
            return _SM_TRAN(&HLDCParser_idle);
        }
        case INPUT_NORMAL_SIG: {
            /**
             * Valid checksum, local iterated VS received
             */
            if (
                HLDCParser_execChksumValid(containerof(me, HLDCParser, sm_hsm_),
                                           HLDCParser_execEsc(
                                             ((HLDCParserEvt *)e)->currByte))
            ) {
                HLDCParser_frameProcess(containerof(me, HLDCParser, sm_hsm_));
            } else {
                HLDCParser_frameError(containerof(me, HLDCParser, sm_hsm_));
            }
            return _SM_TRAN(&HLDCParser_active);
        }
        default: {
            return _SM_SUPER();
        }
    }
}

/*==========================================================================*/
void HLDCParser_trigger(uint8_t byte_) {
    /* Event build... */
    HLDCParserEvt evt;
    if (HLDCParser_isFrameFlag(byte_)) {
        evt.sig = INPUT_FRAME_FLAG_SIG;
    } else if (HLDCParser_isEsc(byte_)) {
        evt.sig = INPUT_ESC_SIG;
    } else {
        evt.sig = INPUT_NORMAL_SIG;
    }
    evt.currByte = byte_;

    /* Event dispatch. */
    (*HLDCParser_inst.dispatch)(&HLDCParser_inst, (void *)(&evt));
}

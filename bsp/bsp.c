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
SM_DEFINE_MODULE("bsp")

/*==========================================================================*/
void BSP_init(void) {
    /* Initialize event pools ..............................................*/
    /* Small (blockSize: 16B) */
    static QF_MPOOL_EL(QEvt) smlPoolSto[512];
    QF_poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    /* Medium (blockSize: 20B) */
    static QF_MPOOL_EL(SpMngrEvt) midPoolSto[1024];
    QF_poolInit(midPoolSto, sizeof(midPoolSto), sizeof(midPoolSto[0]));
}
/*..........................................................................*/
void BSP_ledOff(void) {
}
/*..........................................................................*/
void BSP_ledOn(void) {
}

/*==========================================================================*/
void QF_onStartup(void) {
}
/*..........................................................................*/
void QF_onCleanup(void) {
    /* Nothing to do — SDL handles cleanup. */
}
/*..........................................................................*/
void QF_onClockTick(void) {
    QTIMEEVT_TICK_X(0, (void *)0);
}

/*==========================================================================*/
void Q_onAssert(char const * const module, int_t const loc) {
    fprintf(stderr, "ASSERT: %s:%d\n", module, (int)loc);
    exit(-1);
}
/*..........................................................................*/
#ifndef SM_DBC_DISABLE
SM_NORETURN SM_onAssert(char const *module, int label) {
    Q_onAssert(module, label);
}
#endif  /* SM_DBC_DISABLE */

/*==========================================================================*/
void HLDCParser_frmPost(unsigned char *frmOut) {
    int frmSize = 3 + frmOut[2];
    unsigned char *frmPkg = malloc(frmSize);
    SM_ENSURE(frmPkg != (unsigned char *)0);

    memcpy(frmPkg, frmOut, frmSize);
    SpMngrEvt *evt = Q_NEW(SpMngrEvt, SP_MNGR_RECV_MSG_SIG);
    evt->pld.msgFrm = frmPkg;
    evt->pldSize.msgFrmSize = frmSize;
    QACTIVE_POST(AO_SpMngr, (QEvt*)evt, (void *)0);
}

//============================================================================
// QP/C Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#define QP_IMPL           // this is QP implementation
#include "qp_port.h"      // QP port
#include "qp_pkg.h"       // QP package-scope interface
#include "qsafe.h"        // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // QS port
    #include "qs_pkg.h"   // QS package-scope internal interface
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

#include <limits.h>       // limits of dynamic range for integers

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects =============================================================
SDL_mutex *l_sdlCritMutex;
static int_t l_critSectNest;

static SDL_mutex *l_startupMutex;
static SDL_cond  *l_startupCond;
static bool       l_startupDone;
static uint32_t   l_tickMsec = 10U;
static int        l_tickPrio = 50;
static bool       l_isRunning;

static int SDLCALL ao_thread(void *arg);

//============================================================================
// QF functions

//............................................................................
void QF_enterCriticalSection_(void) {
    SDL_LockMutex(l_sdlCritMutex);
    Q_ASSERT_INCRIT(100, l_critSectNest == 0);
    ++l_critSectNest;
}
//............................................................................
void QF_leaveCriticalSection_(void) {
    Q_ASSERT_INCRIT(200, l_critSectNest == 1);
    if ((--l_critSectNest) == 0) {
        SDL_UnlockMutex(l_sdlCritMutex);
    }
}

//............................................................................
void QF_init(void) {
    l_sdlCritMutex = SDL_CreateMutex();
    l_startupMutex = SDL_CreateMutex();
    l_startupCond  = SDL_CreateCond();
    l_startupDone  = false;

    QTimeEvt_init();
}

//............................................................................
int QF_run(void) {
    QF_onStartup();

    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()

    SDL_LockMutex(l_startupMutex);
    l_startupDone = true;
    SDL_CondBroadcast(l_startupCond);
    SDL_UnlockMutex(l_startupMutex);

    l_isRunning = true;

    QF_setTickRate(100U, 100);
    SDL_ThreadPriority sdlPrio = SDL_THREAD_PRIORITY_NORMAL;
    if (l_tickPrio < 33) {
        sdlPrio = SDL_THREAD_PRIORITY_LOW;
    }
    else if (l_tickPrio > 66) {
        sdlPrio = SDL_THREAD_PRIORITY_HIGH;
    }
    SDL_SetThreadPriority(sdlPrio);

    while (l_isRunning) {
        SDL_Delay(l_tickMsec);
        QF_onClockTick();
    }

    QF_onCleanup();
    QS_EXIT();

    return 0;
}
//............................................................................
void QF_stop(void) {
    l_isRunning = false; // terminate the main (ticker) thread
}
//............................................................................
void QF_setTickRate(uint32_t ticksPerSec, int tickPrio) {
    Q_REQUIRE_ID(600, ticksPerSec != 0U);
    l_tickMsec = 1000UL / ticksPerSec;
    l_tickPrio = tickPrio;
}

// QActive functions =========================================================
void QActive_start(QActive * const me,
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto, uint_fast16_t const qLen,
    void * const stkSto, uint_fast16_t const stkSize,
    void const * const par)
{
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    // no external AO-stack storage needed for this port
    Q_REQUIRE_ID(800, stkSto == (void *)0);

    me->prio  = (uint8_t)(prioSpec & 0xFFU);
    me->pthre = 0U;
    QActive_register_(me);

    me->osObject = (void *)SDL_CreateCond();
    QEQueue_init(&me->eQueue, qSto, qLen);

    // the top-most initial tran. (virtual)
    QASM_INIT(&me->super, par, me->prio);
    QS_FLUSH();

    me->thread = (void *)SDL_CreateThread(ao_thread, "AO_thread", me);
    Q_ENSURE_ID(830, me->thread != NULL);
    me->pthre = (uint8_t)((prioSpec >> 8U) & 0xFFU);
}
//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive_stop(QActive * const me) {
    if (QActive_subscrList_ != (QSubscrList *)0) {
        QActive_unsubscribeAll(me); // unsubscribe from all events
    }
    me->thread = (void *)0; // stop the thread loop (see ao_thread())
}
#endif
//............................................................................
void QActive_setAttr(QActive *const me, uint32_t attr1, void const *attr2) {
    Q_UNUSED_PAR(me);
    Q_UNUSED_PAR(attr1);
    Q_UNUSED_PAR(attr2);
    Q_ERROR_INCRIT(900); // should not be called in this QP port
}

//============================================================================
static int SDLCALL ao_thread(void *arg) {
    QActive *act = (QActive *)arg;

    switch (act->pthre) {
        case 1U:
            SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);
            break;
        case 3U:
            SDL_SetThreadPriority(SDL_THREAD_PRIORITY_TIME_CRITICAL);
            break;
        default:
            SDL_SetThreadPriority(SDL_THREAD_PRIORITY_NORMAL);
            break;
    }

    SDL_LockMutex(l_startupMutex);
    while (!l_startupDone) {
        SDL_CondWait(l_startupCond, l_startupMutex);
    }
    SDL_UnlockMutex(l_startupMutex);

#ifdef QACTIVE_CAN_STOP
    while (act->thread)
#else
    for (;;) // for-ever
#endif
    {
        QEvt const *e = QActive_get_(act); // wait for event
        QASM_DISPATCH(&act->super, e, act->prio); // dispatch to the SM
        QF_gc(e); // check if the event is garbage, and collect it if so
    }
#ifdef QACTIVE_CAN_STOP
    QActive_unregister_(act); // un-register this active object
#endif
    return 0;
}

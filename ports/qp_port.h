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
#ifndef QP_PORT_H_
#define QP_PORT_H_

#include <stdint.h>     // Exact-width types. WG14/N843 C99 Standard
#include <stdbool.h>    // Boolean type.      WG14/N843 C99 Standard
#include "qp_config.h"  // QP configuration from the application

#ifdef __GNUC__

    // no-return function specifier (GCC compiler)
    #define Q_NORETURN   __attribute__ ((noreturn)) void

#elif (defined _MSC_VER)
    #ifdef __cplusplus
        // no-return function specifier (Microsoft Visual Studio C++ compiler)
        #define Q_NORETURN   [[ noreturn ]] void
    #else
        // no-return function specifier C11
        #define Q_NORETURN   _Noreturn void
    #endif

    // This is the case where QP is compiled by the Microsoft Visual C++
    // compiler in the C++ mode, which can happen when qep_port.h is included
    // in a C++ module, or the compilation is forced to C++ by the option /TP.
    //
    // The following pragma suppresses the level-4 C++ warnings C4510, C4512,
    // and C4610, which warn that default constructors and assignment operators
    // could not be generated for structures QMState and QMTranActTable.
    //
    // The QP source code cannot be changed to avoid these C++ warnings
    // because the structures QMState and QMTranActTable must remain PODs
    // (Plain Old Datatypes) to be initializable statically with constant
    // initializers.
    //
    #pragma warning (disable: 4510 4512 4610)

#endif

// static assertion (C11 Standard)
#define Q_ASSERT_STATIC(expr_)  _Static_assert((expr_), "QP static assert")

// QActive event queue, os-type, and thread types
#define QACTIVE_EQUEUE_TYPE  QEQueue
#define QACTIVE_OS_OBJ_TYPE  void*
#define QACTIVE_THREAD_TYPE  void*

// QF critical section for Win32, see NOTE1
#define QF_CRIT_STAT
#define QF_CRIT_ENTRY()      QF_enterCriticalSection_()
#define QF_CRIT_EXIT()       QF_leaveCriticalSection_()
#define QF_CRIT_EST()        QF_enterCriticalSection_()

// QF_LOG2 not defined -- use the internal LOG2() implementation

// internal functions for critical section management
void QF_enterCriticalSection_(void);
void QF_leaveCriticalSection_(void);

// set clock tick rate and priority
void QF_setTickRate(uint32_t ticksPerSec, int tickPrio);

// clock tick callback
void QF_onClockTick(void);

// special adaptations for QWIN GUI applications
#ifdef QWIN_GUI
    // replace main() with main_gui() as the entry point to a GUI app.
    #define main() main_gui()
    int main_gui(void); // prototype of the GUI application entry point
#elif defined QF_CONSOLE
    // abstractions for console access...
    void QF_consoleSetup(void);
    void QF_consoleCleanup(void);
    int QF_consoleGetKey(void);
    int QF_consoleWaitForKey(void);
#endif

// include files -------------------------------------------------------------
#include "qequeue.h"   // Win32 port needs the native event-queue
#include "qmpool.h"    // Win32 port needs the native memory-pool
#include "qp.h"        // QP platform-independent public interface

#ifdef _MSC_VER
    #pragma warning (default: 4510 4512 4610)
#endif

//============================================================================
// interface used only inside QF implementation, but not in applications

#ifdef QP_IMPL

// QF scheduler locking (not used at this point, see NOTE2)
#define QF_SCHED_STAT_
#define QF_SCHED_LOCK_(dummy) ((void)0)
#define QF_SCHED_UNLOCK_()    ((void)0)

#include <SDL.h>  // SDL2 types needed for macros below

// global critical-section mutex defined in qf_port.c
extern SDL_mutex *l_sdlCritMutex;

// QF event queue customization for SDL2
#define QACTIVE_EQUEUE_WAIT_(me_) \
    while ((me_)->eQueue.frontEvt == (QEvt *)0) { \
        QF_CRIT_EXIT(); \
        SDL_CondWait((SDL_cond *)(me_)->osObject, l_sdlCritMutex); \
        QF_CRIT_ENTRY(); \
    }

#define QACTIVE_EQUEUE_SIGNAL_(me_) \
    SDL_CondSignal((SDL_cond *)(me_)->osObject)

// QMPool operations
#define QF_EPOOL_TYPE_  QMPool
#define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
            (QMPool_init(&(p_), (poolSto_), (poolSize_), (evtSize_)))
#define QF_EPOOL_EVENT_SIZE_(p_)  ((uint16_t)(p_).blockSize)
#define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
            ((e_) = (QEvt *)QMPool_get(&(p_), (m_), (qsId_)))
#define QF_EPOOL_PUT_(p_, e_, qsId_) (QMPool_put(&(p_), (e_), (qsId_)))
#define QF_EPOOL_USE_(ePool_)   (QMPool_getUse(ePool_))
#define QF_EPOOL_FREE_(ePool_)  ((uint16_t)(ePool_)->nFree)
#define QF_EPOOL_MIN_(ePool_)   ((uint16_t)(ePool_)->nMin)

#endif // QP_IMPL

//============================================================================
// NOTE1:
// QP, like all real-time frameworks, needs to execute certain sections of
// code exclusively — only one thread at a time. Such sections are called
// "critical sections".
//
// This SDL2 port uses a pair of functions QF_enterCriticalSection_() /
// QF_leaveCriticalSection_() implemented in qf_port.c. They protect all
// critical sections using a single SDL_mutex (l_sdlCritMutex). The mutex
// is non-recursive; re-entry is explicitly forbidden and caught by an
// assertion (Q_ASSERT_INCRIT). Using one mutex for all critical sections
// guarantees that only one thread at a time is inside a critical section,
// preventing race conditions and data corruption.
//
// NOTE2:
// Scheduler locking (used inside QActive_publish_()) is NOT implemented
// in this port. This means that event multicasting is NOT atomic, so thread
// preemption CAN happen during that time, especially when a low-priority
// thread publishes events to higher-priority threads. This can lead to
// (occasionally) unexpected event sequences.
//

#endif // QP_PORT_H_

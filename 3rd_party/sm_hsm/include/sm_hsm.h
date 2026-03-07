//============================================================================
// Copyright (C) 2026 Sunny Matato
//
// This program is free software. It comes without any warranty, to
// the extent permitted by applicable law. You can redistribute it
// and/or modify it under the terms of the Do What The Fuck You Want
// To Public License, Version 2, as published by Sam Hocevar.
// See http://www.wtfpl.net/ for more details.
//============================================================================
#ifndef SM_HSM_H_
#define SM_HSM_H_

//============================================================================
//! @brief Typically, if you use 8051, you might do this:
//                        #define SM_HSM_ROM const code
//                        #define SM_HSM_RETT reentrant
//                    if you use Cortex-M3, you might do this:
//                        #define SM_HSM_ROM const
//                        #define SM_HSM_RETT
//! @cond INTERNAL

#ifndef SM_HSM_ROM
    #define SM_HSM_ROM const
#endif // SM_HSM_ROM

#ifndef SM_HSM_RETT
    #define SM_HSM_RETT
#endif // SM_HSM_RETT

//! @endcond

//============================================================================
//! @brief Event processing return.
//! @cond INTERNAL

typedef unsigned char SM_RetState;
#define SM_RET_HANDLED 0
#define SM_RET_TRAN    1
#define SM_RET_SUPER   2

//! @endcond

//============================================================================
//! @brief UML Top-Initial handler, Entry/Exit handler, nomal state handler.
//! @cond INTERNAL

typedef void        (*SM_ActionHandler)(void * const me) SM_HSM_RETT;
typedef SM_RetState (*SM_StateHandler) (void * const me,
                                        void const * const e) SM_HSM_RETT;

//! @endcond

//............................................................................
//! @class SM_HsmState
struct SM_HsmState {
    struct SM_HsmState SM_HSM_ROM * super;
    struct SM_HsmState SM_HSM_ROM * (*init_)(void * const me) SM_HSM_RETT;
    SM_ActionHandler entry_;
    SM_ActionHandler exit_;
    SM_StateHandler handler_;
};

//............................................................................
//! @brief Struct type package.
//! @cond INTERNAL

typedef struct SM_HsmState SM_HSM_ROM SM_HsmState;
typedef struct SM_HsmState SM_HSM_ROM * SM_StatePtr;
typedef SM_StatePtr (*SM_InitHandler)(void * const me) SM_HSM_RETT;

//! @endcond

//============================================================================
//! @class SM_Hsm
//! @extends SM_HsmState
typedef struct SM_Hsm {
    SM_StatePtr curr; //!< @private @memberof SM_Hsm
    SM_StatePtr next; //!< @private @memberof SM_Hsm
} SM_Hsm;

//............................................................................
#define _SM_HANDLED()     (SM_RET_HANDLED)
#define _SM_SUPER()       (SM_RET_SUPER)
#define _SM_TRAN(target_) ((((SM_Hsm *)(me))->next) = (target_), SM_RET_TRAN)
#define _SM_INIT(target_) (target_) // TOP-INIT && STATE-INIT

//============================================================================
// Deepest nest level for HSM, TOP state exclude
#ifndef SM_MAX_NEST_DEPTH_
    #define SM_MAX_NEST_DEPTH_ 5
#endif // SM_MAX_NEST_DEPTH_

//............................................................................
//! @brief Virtual pointer of SM_Hsm_init_ and SM_Hsm_dispatch_.
//! @cond INTERNAL

typedef void (*VC_Handler)(void * const me, void const * const e) SM_HSM_RETT;

//! @endcond

//============================================================================
//! @static @public @memberof SM_Hsm
void SM_Hsm_init_(SM_Hsm * const me, SM_InitHandler initial_) SM_HSM_RETT;

//! @static @public @memberof SM_Hsm
void SM_Hsm_dispatch_(SM_Hsm * const me,
                      void const * const e) SM_HSM_RETT;

//! @static @public @memberof SM_Hsm
void SM_Hsm_transition_(SM_Hsm * const me,
                        SM_StatePtr source,
                        SM_StatePtr target) SM_HSM_RETT;

#endif // SM_HSM_H_

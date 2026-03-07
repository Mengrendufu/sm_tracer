//============================================================================
// Copyright (C) 2026 Sunny Matato
//
// This program is free software. It comes without any warranty, to
// the extent permitted by applicable law. You can redistribute it
// and/or modify it under the terms of the Do What The Fuck You Want
// To Public License, Version 2, as published by Sam Hocevar.
// See http://www.wtfpl.net/ for more details.
//============================================================================
#include "sm_port.h"
#include "sm_assert.h"
#include "sm_hsm.h"

//============================================================================
SM_DEFINE_MODULE("sm_hsm")

//============================================================================
//! @private @memberof SM_Hsm
void SM_Hsm_init_(SM_Hsm * const me, SM_InitHandler initial_) SM_HSM_RETT {
    SM_StatePtr path[SM_MAX_NEST_DEPTH_];
    SM_StatePtr s;
    SM_StatePtr target;
    signed char ip;
    signed char i;

    SM_REQUIRE(me != (SM_Hsm *)0);
    SM_REQUIRE(initial_ != (SM_InitHandler)0);

    target = (*((SM_InitHandler)(initial_)))(me);
    SM_ENSURE(target != (SM_StatePtr)0);

    ip = -1;
    s = target;
    while (s != (SM_StatePtr)0) {
        SM_REQUIRE(ip < (SM_MAX_NEST_DEPTH_ - 1));
        path[++ip] = s;
        s = ((SM_StatePtr)s)->super;
    }

    while (ip >= 0) {
        s = (SM_StatePtr)(path[ip]);
        if (s->entry_) (*s->entry_)(me);
        --ip;
    }

    me->curr = target;
    me->next = (SM_StatePtr)0;

    while (((SM_StatePtr)(me->curr))->init_ != (SM_InitHandler)0) {
        target = (*(((SM_StatePtr)(me->curr))->init_))(me);
        SM_ENSURE(target != (SM_StatePtr)0);
        ip = -1;
        s = target;
        while (s != (SM_StatePtr)0 && s != me->curr) {
            SM_REQUIRE(ip < (SM_MAX_NEST_DEPTH_ - 1));
            path[++ip] = s;
            s = ((SM_StatePtr)s)->super;
        }
        for (i = ip; i >= 0; --i) {
            s = (SM_StatePtr)(path[i]);
            me->curr = s;
            if (s->entry_) (*s->entry_)(me);
        }
    }
}

//............................................................................
//! @private @memberof SM_Hsm
void SM_Hsm_dispatch_(SM_Hsm * const me,
                      void const * const e) SM_HSM_RETT
{
    SM_StatePtr s;
    SM_RetState ret;

    SM_REQUIRE(me != (SM_Hsm *)0);
    SM_REQUIRE(e != (void *)0);

    s = me->curr;

    SM_REQUIRE(s != (SM_StatePtr)0);
    while (s != (SM_StatePtr)0) {
        ret = (*(((SM_StatePtr)(s))->handler_))(me, e);
        switch (ret) {
            case SM_RET_HANDLED: return;
            case SM_RET_SUPER:   s = ((SM_StatePtr)(s))->super; break;
            case SM_RET_TRAN:    SM_Hsm_transition_(me,
                                                    s, me->next);
                                 return;
            default:             SM_ERROR("Whip."); break;
        }
    }
}

//..........................................................................
//! @private @memberof SM_Hsm
void SM_Hsm_transition_(SM_Hsm * const me,
                        SM_StatePtr source,
                        SM_StatePtr target) SM_HSM_RETT
{
    SM_StatePtr path[SM_MAX_NEST_DEPTH_];
    SM_StatePtr s;
    signed char ip;
    signed char i;
    bool bReachedSource;
    bool LCAFound;

    SM_REQUIRE(me != (SM_Hsm *)0);
    SM_REQUIRE((SM_StatePtr)(source) != (SM_StatePtr)0);
    SM_REQUIRE((SM_StatePtr)(target) != (SM_StatePtr)0);

    // ENTRY path.
    ip = -1;
    s = target;
    while (s != (SM_StatePtr)0) {
        SM_REQUIRE(ip < (SM_MAX_NEST_DEPTH_ - 1));
        path[++ip] = s;
        s = ((SM_StatePtr)(s))->super;
    }

    i = 0;
    bReachedSource = false;
    LCAFound = false;
    s = me->curr;
    while (s != (SM_StatePtr)0) {
        if (s == source) bReachedSource = true;
        if (bReachedSource) {
            if (!((s == source) && (target == source))) { // Not self-tran.
                for (i = 0; i <= ip; ++i) {
                    if (s == path[i]) { // Found LCA.
                        LCAFound = true; break;
                    }
                }
            }
        }
        if (LCAFound) break;
        if (((SM_StatePtr)(s))->exit_) {
            (*(((SM_StatePtr)(s))->exit_))(me); // EXIT until source.
        }
        s = ((SM_StatePtr)s)->super;
    }

    if (LCAFound) {
    } else {
        if (bReachedSource) {
            // source->super == NULL && target->super == NULL
            i = ++ip;
        } else {
            // Should never arrive.
            SM_ERROR("Punishment.");
        }
    }

    while (--i >= 0) { // ENTRY except LCA.
        s = ((SM_StatePtr)(path[i]));
        if (s->entry_) (*s->entry_)(me);
    }

    me->curr = target;

    while (((SM_StatePtr)(me->curr))->init_ != (SM_InitHandler)0) {
        target = (*(((SM_StatePtr)(me->curr))->init_))(me);
        SM_ENSURE(target != (SM_StatePtr)0);
        ip = -1;
        s = target;
        while (s != (SM_StatePtr)0 && s != me->curr) {
            SM_REQUIRE(ip < (SM_MAX_NEST_DEPTH_ - 1));
            path[++ip] = s;
            s = ((SM_StatePtr)s)->super;
        }
        for (i = ip; i >= 0; --i) {
            s = (SM_StatePtr)(path[i]);
            me->curr = s;
            if (s->entry_) (*s->entry_)(me);
        }
    }
}

//============================================================================
// Copyright (C) 2026 Sunny Matato
//
// This program is free software. It comes without any warranty, to
// the extent permitted by applicable law. You can redistribute it
// and/or modify it under the terms of the Do What The Fuck You Want
// To Public License, Version 2, as published by Sam Hocevar.
// See http://www.wtfpl.net/ for more details.
//============================================================================
#ifndef SM_ASSERT_H_
#define SM_ASSERT_H_

#ifdef __cplusplus
extern "C" {
#endif

// SM_Assert enable ==========================================================
#ifndef SM_DBC_DISABLE

    #define SM_DEFINE_MODULE(name_)                                          \
                             static char const SM_module_name_[] = name_;

    #define SM_ERROR(dummy_) (SM_onAssert(SM_module_name_, __LINE__))
    #define SM_ASSERT(cond_) ((cond_) ? (void)0 : SM_ERROR("Error."))
    #define SM_ALLEGE(cond_) SM_ASSERT(cond_)

    //! @brief Assertion callback.
    //! @cond INTERNAL

    #ifndef SM_NORETURN
        #define SM_NORETURN void
    #endif // SM_NORETURN

    #ifndef SM_RETT
        #define SM_RETT
    #endif // SM_RETT

    SM_NORETURN SM_onAssert(char const *module, int label) SM_RETT;

    //! @endcond

// SM_Assert disable =========================================================
#else

    #define SM_DEFINE_MODULE(name_)
    #define SM_ERROR(dummy_)
    #define SM_ASSERT(cond_) ((void)0)
    #define SM_ALLEGE(cond_) ((void)(cond_))

#endif // SM_DBC_DISABLE

//============================================================================
#define SM_REQUIRE(cond_) SM_ASSERT(cond_)
#define SM_ENSURE(cond_)  SM_ASSERT(cond_)

#ifdef __cplusplus
}
#endif

#endif // SM_ASSERT_H_

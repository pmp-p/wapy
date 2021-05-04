#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "py/mphal.h"
#include "py/runtime.h"
#include "extmod/misc.h"

#include <stdarg.h>

#include <time.h>
#include <sys/time.h>  // for struct timezone


// fix linking


#include "py/objint.h"

const mpz_dig_t maxsize_dig[] = {
    #define NUM_DIG 1
    (MP_SSIZE_MAX >> MPZ_DIG_SIZE * 0) & DIG_MASK,
    #if (MP_SSIZE_MAX >> MPZ_DIG_SIZE * 0) > DIG_MASK
     #undef NUM_DIG
     #define NUM_DIG 2
     (MP_SSIZE_MAX >> MPZ_DIG_SIZE * 1) & DIG_MASK,
     #if (MP_SSIZE_MAX >> MPZ_DIG_SIZE * 1) > DIG_MASK
      #undef NUM_DIG
      #define NUM_DIG 3
      (MP_SSIZE_MAX >> MPZ_DIG_SIZE * 2) & DIG_MASK,
      #if (MP_SSIZE_MAX >> MPZ_DIG_SIZE * 2) > DIG_MASK
       #undef NUM_DIG
       #define NUM_DIG 4
       (MP_SSIZE_MAX >> MPZ_DIG_SIZE * 3) & DIG_MASK,
       #if (MP_SSIZE_MAX >> MPZ_DIG_SIZE * 3) > DIG_MASK
        #error cannot encode MP_SSIZE_MAX as mpz
       #endif
      #endif
     #endif
    #endif
};

const mp_obj_int_t mp_sys_maxsize_obj = {
    {&mp_type_int},
    {.fixed_dig = 1, .len = NUM_DIG, .alloc = NUM_DIG, .dig = (mpz_dig_t *)maxsize_dig}
};



// logging


FILE *fd_logger;


// hal


#if __EMSCRIPTEN__
    #pragma message " ------------------------ EMSCRIPTEN enabled ----------------------"
#else

const struct _mp_obj_module_t mp_module_ffi ;

static int emscripten_loop_run = 1;

void emscripten_cancel_main_loop(){
    emscripten_loop_run = 0;
};

#endif

/*
mp_import_stat_t
wasm_find_module(const char *modname) {
    cdbg("404:wasm_find_module '%s'\n", modname);
    return MP_IMPORT_STAT_NO_EXIST;
}


uint
mp_import_stat(const char *path) {
    cdbg("404:mp_import_stat '%s'\n", path);
    return MP_IMPORT_STAT_NO_EXIST;
}


*/


#ifndef __CPP__

    // WASI and non POSIX support

    #include <time.h>
    #include <sys/time.h>

    struct timespec t_timespec;
    struct timeval t_timeval;
    struct timeval wa_tv;
    unsigned int wa_ts_nsec;

    #if defined(HAL_EMSDK)
        #include "../wapy-hal/wasm.c"
    #else
        #ifndef __EMSCRIPTEN__
        #include "../wapy/stubs/emscripten.h"
        #endif

        #if defined(__WASI__) || defined(__ARDUINO__)
            #if defined(__ARDUINO__)
                extern void sched_yield();
            #else
                #include <sched.h>
            #endif

            int wasx_clock_gettime(clockid_t clockid, struct timespec *ts) {
                sched_yield();
                ts->tv_sec = wa_tv.tv_sec;
                ts->tv_nsec = wa_ts_nsec;
                return 0;
            }

            int wasx_gettimeofday(struct timeval *tv, const struct timezone *tz) {
                sched_yield();
                tv->tv_sec = wa_tv.tv_sec;
                tv->tv_usec = wa_tv.tv_usec;
                return 0;
            }

        #else
            // posix native
            // or
            // emscripten can handle 64bits return values directly via its javascript polyfill.
            int wasx_clock_gettime(clockid_t clockid, struct timespec *ts) {
                int rv = clock_gettime(CLOCK_MONOTONIC, ts);
                return rv;
            }

            int wasx_gettimeofday(struct timeval *tv, const struct timezone *tz) {
                int rv = gettimeofday(&wa_tv, NULL);
                tv->tv_sec = wa_tv.tv_sec;
                tv->tv_usec = wa_tv.tv_usec;
                return rv;
            }

        #endif

        #ifndef HAL
            #define HAL "../wapy-hal/stub.c"
        #endif

        #if defined(__ANDROID__)
            #include "../wapy-hal/aosp.c"
        #elif defined(__EMSCRIPTEN__)
            #include "../wapy-hal/emsdk.c"
        #elif defined(__ARDUINO__) || defined(__WASI__)
            #include HAL
        #else
            #pragma message " ---------- NO HAL specified using stubs -------------"
            #include HAL
        #endif
    #endif
#endif

#include <time.h>
#include <sys/time.h>

FILE *fd_logger;

struct timespec t_timespec;
struct timeval t_timeval;
struct timeval wa_tv;
unsigned int wa_ts_nsec;
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


uint mp_import_stat(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return MP_IMPORT_STAT_DIR;
        } else if (S_ISREG(st.st_mode)) {
            return MP_IMPORT_STAT_FILE;
        }
    }
    return MP_IMPORT_STAT_NO_EXIST;
}


// *FORMAT-OFF*

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



#ifdef NUM_DIG
const mp_obj_int_t mp_sys_maxsize_obj = {
    {&mp_type_int},
    {.fixed_dig = 1, .len = NUM_DIG, .alloc = NUM_DIG, .dig = (mpz_dig_t *)maxsize_dig}
};


#else // ?? objint_longlong definition
    const mp_obj_int_t mp_sys_maxsize_obj = {{&mp_type_int}, MP_SSIZE_MAX};
#endif

// *FORMAT-ON*


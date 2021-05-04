// wapy-hal/emsdk.c
#pragma message "POSIX hal?"

#include <time.h>
#include <sys/time.h>

extern int sched_yield(void);

extern int wasx_clock_gettime(clockid_t clockid, struct timespec *ts);
extern int wasx_gettimeofday(struct timeval *tv, const struct timezone *tz);
/*
 {
    int rv = clock_gettime(CLOCK_MONOTONIC, ts);
    return rv;
}

int wasx_gettimeofday(struct timeval *tv, const struct timezone *tz) {
    int rv = gettimeofday(&wa_tv, NULL);
    tv->tv_sec = wa_tv.tv_sec;
    tv->tv_usec = wa_tv.tv_usec;
    return rv;
}

*/
void mp_hal_delay_us(mp_uint_t us) {
    cdbg("mp_hal_delay_us(%u)", us );
}

void mp_hal_delay_ms(mp_uint_t ms) {
    mp_hal_delay_us(ms*1000);
}

uint64_t mp_hal_time_ns(void) {
    time_t now = time(NULL);
    return (uint64_t)now * 1000000000ULL;
}


void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    for(int i=0;i<len;i++) {
        printf("%c", str[i] );
    }
}


// Receive single character
int mp_hal_stdin_rx_chr(void) {
    unsigned char c = fgetc(stdin);
    return c;
}



#define EPOCH_US 0
#if EPOCH_US
static unsigned long epoch_us = 0;
#endif


mp_uint_t mp_hal_ticks_ms(void) {
    #if (defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0) && defined(_POSIX_MONOTONIC_CLOCK)
    wasx_clock_gettime(CLOCK_MONOTONIC, &t_timespec);
    return t_timespec.tv_sec * 1000 + t_timespec.tv_nsec / 1000000;
    #else
    wa_gettimeofday(&t_timeval, NULL);
    return t_timeval.tv_sec * 1000 + t_timeval.tv_usec / 1000;
    #endif
}


mp_uint_t mp_hal_ticks_us(void) {
    wasx_clock_gettime(CLOCK_MONOTONIC, &t_timespec);
    unsigned long now_us = t_timespec.tv_sec * 1000000 + t_timespec.tv_nsec / 1000 ;
    #if EPOCH_US
        if (!epoch_us)
            epoch_us = now_us-1;
        return (mp_uint_t)(now_us - epoch_us);
    #else
        return  (mp_uint_t)now_us;
    #endif
}


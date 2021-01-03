#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "py/mphal.h"
#include "py/runtime.h"
#include "extmod/misc.h"

#include <stdarg.h>

#if defined(__EMSCRIPTEN__) || defined(__WASI__)
    #include "emscripten.h"
#elif __CPP__
    #define EMSCRIPTEN_KEEPALIVE
#endif

#include "../wapy/upython.h"

#include <time.h>

#if defined(__EMSCRIPTEN__)
#define wa_clock_gettime(clockid, timespec) clock_gettime(clockid, timespec)
#include <sys/time.h>
#define wa_gettimeofday(timeval, tmz) gettimeofday(timeval, tmz)
#endif


void mp_hal_delay_us(mp_uint_t us) {
    clog("mp_hal_delay_us(%u)", us );
}

void mp_hal_delay_ms(mp_uint_t ms) {
    mp_hal_delay_us(ms*1000);
}


extern struct timespec t_timespec;
extern struct timeval t_timeval;


#define EPOCH_US 0
#if EPOCH_US
static unsigned long epoch_us = 0;
#endif


uint64_t mp_hal_time_ns(void) {
   // wa_clock_gettime(CLOCK_MONOTONIC, &t_timespec);
   // return (uint64_t)( t_timespec.tv_sec + t_timespec.tv_sec );

    wa_gettimeofday(&t_timeval, NULL);
    return (uint64_t)t_timeval.tv_sec * 1000000000ULL + (uint64_t)t_timeval.tv_usec * 1000ULL;
}


mp_uint_t mp_hal_ticks_ms(void) {
    #if (defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0) && defined(_POSIX_MONOTONIC_CLOCK)
    wa_clock_gettime(CLOCK_MONOTONIC, &t_timespec);
    return t_timespec.tv_sec * 1000 + t_timespec.tv_nsec / 1000000;
    #else
    wa_gettimeofday(&t_timeval, NULL);
    return t_timeval.tv_sec * 1000 + t_timeval.tv_usec / 1000;
    #endif
}


mp_uint_t mp_hal_ticks_us(void) {
    wa_clock_gettime(CLOCK_MONOTONIC, &t_timespec);
    unsigned long now_us = t_timespec.tv_sec * 1000000 + t_timespec.tv_nsec / 1000 ;
    #if EPOCH_US
        if (!epoch_us)
            epoch_us = now_us-1;
        return (mp_uint_t)(now_us - epoch_us);
    #else
        return  (mp_uint_t)now_us;
    #endif
}


// Receive single character
int mp_hal_stdin_rx_chr(void) {

    cdbg("mp_hal_stdin_rx_chr");
    unsigned char c = fgetc(stdin);
    return c;
}

static unsigned char last = 0;

extern rbb_t out_rbb;

unsigned char v2a(int c)
{
    const unsigned char hex[] = "0123456789abcdef";
    return hex[c];
}

unsigned char hex_hi(unsigned char b) {
    return v2a((b >> 4) & 0x0F);
}
unsigned char hex_lo(unsigned char b) {
    return v2a((b) & 0x0F);
}

unsigned char out_push(unsigned char c) {
    if (last>127) {
        //if (c>127)
          //  fprintf(stderr," -- utf-8(2/2) %u --\n", c );
    } else {
        //if (c>127)
          //  fprintf(stderr," -- utf-8(1/2) %u --\n", c );
    }
    rbb_append(&out_rbb, hex_hi(c));
    rbb_append(&out_rbb, hex_lo(c));
    return (unsigned char)c;
}



//FIXME: libc print with valid json are likely to pass and get interpreted by pts
//TODO: buffer all until render tick
extern int io_encode_hex;
//this one (over)cooks like _cooked
void mp_hal_stdout_tx_strn(const char *str, size_t len) {
#if defined(__EMSCRIPTEN__)
#else // WASI/node
    if (!io_encode_hex) {
        printf("%s", str);
        return;
    }
#endif
    for(int i=0;i<len;i++) {
        if ( (str[i] == 0x0a) && (last != 0x0d) ) {
            out_push( 0x0d );
        }
        last = out_push( (unsigned char)str[i] );
    }
}


#if defined(__EMSCRIPTEN__)

EMSCRIPTEN_KEEPALIVE static PyObject *
embed_run_script(PyObject *self, PyObject *argv) {
    char *runstr = NULL;
    if (!PyArg_ParseTuple(argv, "s", &runstr)) {
        return NULL;
    }
    emscripten_run_script( runstr );
    Py_RETURN_NONE;
}


#endif



#if MICROPY_USE_READLINE

#else

char *prompt(char *p) {
    cdbg("165: simple read string\n");
    static char buf[256];
    fputs(p, stderr);
    char *s = fgets(buf, sizeof(buf), stdin);
    if (!s) {
        return NULL;
    }
    int l = strlen(buf);
    if (buf[l - 1] == '\n') {
        buf[l - 1] = 0;
    } else {
        l++;
    }
    char *line = malloc(l);
    memcpy(line, buf, l);
    return line;
}

#endif

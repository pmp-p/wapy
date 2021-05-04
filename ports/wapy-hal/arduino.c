#pragma message "arduino hal?"

#include <time.h>
#include <sys/time.h>

void mp_reader_new_file(mp_reader_t *reader, const char *filename) {
}

mp_import_stat_t mp_import_stat(const char *path) {
    cdbg("25:stat(%s) : ", path);
    return wasm_find_module(path);
}

mp_lexer_t *
mp_lexer_new_from_file(const char *filename) {
    FILE *file = fopen(filename,"r");
    if (!file) {
        //printf("404: fopen(%s)\n", filename);
        clog("404: fopen(%s)\n", filename);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long long size_of_file = ftell(file);
    //cdbg("mp_lexer_new_from_file(%s size=%lld)\n", filename, (long long)size_of_file );
    fseek(file, 0, SEEK_SET);

    char * cbuf = malloc(size_of_file+1);
    fread(cbuf, size_of_file, 1, file);
    cbuf[size_of_file]=0;

    if (cbuf == NULL) {
        cdbg("READ ERROR: mp_lexer_new_from_file(%s size=%lld)\n", filename, (long long)size_of_file );
        return NULL;
    }
    mp_lexer_t* lex = mp_lexer_new_from_str_len(qstr_from_str(filename), cbuf, strlen(cbuf), 0);
    free(cbuf); // <- remove that and emcc -shared will break
    return lex;
}

#ifndef __FINAL__
    #include "wapy/core/file.wapy.c"
    #include "wapy/core/ringbuf_b.c"
    #include "../py/frozenmod.c"
#endif

extern struct timeval wa_tv;
extern unsigned int wa_ts_nsec;

extern int sched_yield(void);
//static
 int wa_clock_gettime(clockid_t clockid, struct timespec *ts) {
    sched_yield();
    ts->tv_sec = wa_tv.tv_sec;
    ts->tv_nsec = wa_ts_nsec;
    return 0;
}


//static
 int wa_gettimeofday(struct timeval *tv, const struct timezone *tz) {
    sched_yield();
    tv->tv_sec = wa_tv.tv_sec;
    tv->tv_usec = wa_tv.tv_usec;
    return 0;
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
        if (c>127)
            fprintf(stderr," -- utf-8(2/2) %u --\n", c );
    } else {
	if (c>127)
            fprintf(stderr," -- utf-8(1/2) %u --\n", c );
    }
    rbb_append(&out_rbb, hex_hi(c));
    rbb_append(&out_rbb, hex_lo(c));
    return (unsigned char)c;
}


void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    for(int i=0;i<len;i++) {
        if ( (str[i] == 0x0a) && (last != 0x0d) ) {
            out_push( 0x0d );
        }
        last = out_push( (unsigned char)str[i] );
    }
}


// Receive single character
int mp_hal_stdin_rx_chr(void) {

    fprintf(stderr,"mp_hal_stdin_rx_chr");
    unsigned char c = fgetc(stdin);
    return c;
}



#define EPOCH_US 0
#if EPOCH_US
static unsigned long epoch_us = 0;
#endif


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





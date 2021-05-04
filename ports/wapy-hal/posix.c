#pragma message "POSIX hal?"

#include <time.h>
#include <sys/time.h>

void mp_reader_new_file(mp_reader_t *reader, const char *filename) {
}

const struct _mp_obj_module_t mp_module_ffi ;

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
    #include "../wapy/core/file.wapy.c"
    #include "../wapy/core/ringbuf_b.c"
    #include "../py/frozenmod.c"
#endif

extern struct timeval wa_tv;
extern unsigned int wa_ts_nsec;
extern int sched_yield(void);

void mp_hal_delay_us(mp_uint_t us) {
    clog("mp_hal_delay_us(%u)", us );
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



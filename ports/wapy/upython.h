#ifndef UPYTHON_H

#include <stdarg.h>

#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "lib/utils/pyexec.h"
#include "py/mphal.h"

#define IS_FILE 1
#define IS_STR 0

#ifdef __EMSCRIPTEN__
    #include "emscripten.h"
#else
    #define EMSCRIPTEN_KEEPALIVE
#endif
extern size_t bsd_strlen(const char *str);

//EMSCRIPTEN_KEEPALIVE void Py_InitializeEx(int param);
//EMSCRIPTEN_KEEPALIVE void PyRun_SimpleString(const char * code);

#if WASM_FILE_API
EMSCRIPTEN_KEEPALIVE void PyRun_SimpleFile(FILE *fp, const char *filename);
EMSCRIPTEN_KEEPALIVE void PyRun_VerySimpleFile(const char *filename);
#endif

int do_code(const char *src,  int is_file);

#define PATHLIST_SEP_CHAR ':'

#if WAPY

    #if MICROPY_ENABLE_GC
        static char heap[32*1024*1024];
    #endif
static int repl_started = -100;

#endif

#define REPL_INPUT_SIZE 16384
#define REPL_INPUT_MAX REPL_INPUT_SIZE-1


#define nullptr NULL
#define Py_RETURN_NONE return nullptr;
#define PyObject mp_obj_t


#include "core/ringbuf_b.h"
#include "core/ringbuf_o.h"


int PyArg_ParseTuple(PyObject *argv, const char *fmt, ...);

extern int VMFLAGS_IF;
extern int SHOW_OS_LOOP;
extern int show_os_loop(int state);

struct
wPyInterpreterState {
    char *shm_stdio ;
    char *shm_input_event_0;
};


struct
wPyThreadState {
    struct wPyInterpreterState *interp;
};

extern struct wPyInterpreterState i_main ;
extern struct wPyThreadState i_state ;

#define cdbg(...) if (1){ fprintf(stderr, __VA_ARGS__ );fprintf(stderr, "\n"); }
#define clog(...) if (show_os_loop(-1)){ fprintf(stderr, __VA_ARGS__ );fprintf(stderr, "\n"); }


#define UPYTHON_H 1

#endif //UPYTHON_H


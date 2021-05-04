#ifndef UPYTHON_H

// still need global interrupt state marker ( @syscall / @awaited / aio_suspend  )
// to choose which VM to enter at top level
// this is different from interrupts context ( if used )


#define HEAP_SIZE 64 * 1024 * 1024

// TODO: use a circular buffer for everything io related
#define MP_IO_SHM_SIZE 65535

#define MP_IO_SIZE 512

#define IO_KBD ( MP_IO_SHM_SIZE - (1 * MP_IO_SIZE) )



#include <stdarg.h>

#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "lib/utils/pyexec.h"
#include "py/mphal.h"

#include "py/compile.h"
#include "py/emitglue.h"
#include "py/objtype.h"
#include "py/runtime.h"
#include "py/parse.h"
#include "py/bc0.h"
// overwrite the "math" in bytecode value with plain integers
// #include "bc_as_integers.h"
#include "py/bc.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "lib/utils/pyexec.h"


// for mp_call_function_0
#include "py/parsenum.h"
#include "py/compile.h"
#include "py/objstr.h"
#include "py/objtuple.h"
#include "py/objlist.h"
#include "py/objmodule.h"  // <= function defined in
#include "py/objgenerator.h"
#include "py/smallint.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/stackctrl.h"
#include "py/gc.h"

//#include "../wapy/core/fdfile.h"
#include "../wapy/repl.h"
#include "wapy/debug.h"


#if WAPY

    #if MICROPY_ENABLE_GC
        static char heap[32*1024*1024];
    #endif

#endif

#define REPL_INPUT_SIZE 16384
#define REPL_INPUT_MAX REPL_INPUT_SIZE-1



#if __EMSCRIPTEN__
#define __MAIN__ (1)


#undef __MAIN__
#endif

#define IS_FILE 1
#define IS_STR 0

#define PATHLIST_SEP_CHAR ':'


#define nullptr NULL
#define Py_RETURN_NONE return nullptr;
#define PyObject mp_obj_t


int VMFLAGS_IF = 0;
int SHOW_OS_LOOP=0;

//struct timespec t_timespec;
//struct timeval t_timeval;


extern int emscripten_GetProcAddress(const char * name);

int uncaught_exception_handler(void);

void dump_args2(const mp_obj_t *a, size_t sz);

int noint_aio_fsync();

int endswith(const char * str, const char * suffix);

size_t has_io();

int PyArg_ParseTuple(PyObject *argv, const char *fmt, ...) {
    va_list argptr;
    va_start (argptr, fmt );
    vfprintf(stdout,fmt,argptr);
    va_end (argptr);
    return 0;
}

#if WASM_FILE_API
EMSCRIPTEN_KEEPALIVE void PyRun_SimpleFile(FILE *fp, const char *filename);
EMSCRIPTEN_KEEPALIVE void PyRun_VerySimpleFile(const char *filename);
#endif

int do_code(const char *src,  int is_file);


int PyArg_ParseTuple(PyObject *argv, const char *fmt, ...);

extern int VMFLAGS_IF;
extern int SHOW_OS_LOOP;
extern int show_os_loop(int state);

/*
struct
wPyInterpreterState {
    char *shm_stdio ;
    char *shm_input_event_0;
};
*/

struct
wPyThreadState {
    struct wPyInterpreterState *interp;
};

extern struct wPyInterpreterState i_main ;
extern struct wPyThreadState i_state ;

extern char * wPy_NewInterpreter();
extern void wPy_Initialize();

extern char * shm_ptr();

#define UPYTHON_H 1

#endif //UPYTHON_H


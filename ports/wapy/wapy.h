
//need global interrupt state marker ( @syscall / @awaited / aio_suspend  )
// to choose which VM to enter at top level
// this is different from ctx interrupts ctx_if ( if used )

int VMFLAGS_IF = 0;
int SHOW_OS_LOOP=0;
struct timespec t_timespec;
struct timeval t_timeval;


#include "../wapy/core/fdfile.h"

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

#include "upython.h"

#define __MAIN__ (1)
#include "emscripten.h"
#undef __MAIN__

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



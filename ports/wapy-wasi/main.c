#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

#define __MAIN__ (1)
#include "emscripten.h"
#undef __MAIN__

#define DBG 0
#define DLOPEN 0

#if !MICROPY_ENABLE_PYSTACK
#error "need MICROPY_ENABLE_PYSTACK (1)
#endif

static int SHOW_OS_LOOP=0;

static int g_argc;
static char **g_argv; //[];

size_t
bsd_strlen(const char *str) {
        const char *s;
        for (s = str; *s; ++s);
        return (s - str);
}


int
endswith(const char * str, const char * suffix) {
  int str_len = strlen(str);
  int suffix_len = strlen(suffix);

  return
    (str_len >= suffix_len) && (0 == strcmp(str + (str_len-suffix_len), suffix));
}

#include "api/wasm_file_api.c"
#include "api/wasm_import_api.c"

static int KPANIC = 1;


extern char* shm_ptr();

char **
copy_argv(int argc, char *argv[]) {
    // calculate the contiguous argv buffer size
    int length=0;

    size_t ptr_args = argc + 1;
    for (int i = 0; i < argc; i++) {
        length += (bsd_strlen(argv[i]) + 1);
    }
    char** new_argv = (char**)malloc((ptr_args) * sizeof(char*) + length);
    // copy argv into the contiguous buffer
    length = 0;
    for (int i = 0; i < argc; i++) {
        new_argv[i] = &(((char*)new_argv)[(ptr_args * sizeof(char*)) + length]);
        strcpy(new_argv[i], argv[i]);
        fprintf(stderr,"argv[%d] = %s\n", i, argv[i]);
        length += (bsd_strlen(argv[i]) + 1);
    }
    // insert NULL terminating ptr at the end of the ptr array
    new_argv[ptr_args-1] = NULL;
    return (new_argv);
}



#include "upython.c"


#include "vmsl/vmreg.h"

#include "vmsl/vmreg.c"

extern mp_obj_t fun_bc_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args);
extern mp_vm_return_kind_t mp_execute_bytecode(mp_code_state_t *code_state, volatile mp_obj_t inject_exc);
extern mp_obj_t gen_instance_iternext(mp_obj_t self_in);
extern int mp_call_prepare_args_n_kw_var(bool have_self, size_t n_args_n_kw, const mp_obj_t *args, mp_call_args_t *out_args);

//extern mp_obj_t fun_bc_call_pre(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args);
//extern mp_obj_t fun_bc_call_past(mp_code_state_t *code_state, mp_vm_return_kind_t vm_return_kind, mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args);


#include <search.h>

#if MICROPY_PY_SYS_SETTRACE
#error "248:duplicate symbol: mp_type_code"
/*
wasm-ld: error: duplicate symbol: mp_type_code
>>> defined in libmicropython.a(profile.o)
>>> defined in libmicropython.a(builtinevex.o)
*/
#include "py/profile.h"
#endif

//extern
int pyexec_friendly_repl_process_char(int c);
int pyexec_repl_repl_restart(int ret);
int handle_uncaught_exception(void);

// entry point for implementing core vm parts in python, set via "embed" module
extern void pyv(mp_obj_t value);
extern mp_obj_t pycore(const char *fn);

// entry point for line tracing
#if 0 //__WASM__
    #pragma message "no embed"
    qstr trace_prev_file = 1/*MP_QSTR_*/;
    qstr trace_file = 1/*MP_QSTR_*/;

    size_t trace_prev_line;
    size_t trace_line;

    int trace_on;
#else
extern qstr trace_prev_file ;
extern size_t trace_prev_line;

extern qstr trace_file ;
extern size_t trace_line;

extern int trace_on;
#endif

#define io_stdin i_main.shm_stdio

FILE *cc;
FILE *kv;
FILE *sc;

// to use kb buffer space for scripts
// if (strlen(io_stdin)>=IO_KBD){ io_stdin[IO_KBD]  = 0;
#define IO_CODE_DONE { io_stdin[0] = 0; }

#if MICROPY_PY_THREAD_GIL && MICROPY_PY_THREAD_GIL_VM_DIVISOR
// This needs to be volatile and outside the VM loop so it persists across handling
// of any exceptions.  Otherwise it's possible that the VM never gives up the GIL.
volatile int gil_divisor = MICROPY_PY_THREAD_GIL_VM_DIVISOR;
#endif



// wasi support

//static
struct timespec t_timespec;
//static
struct timeval t_timeval;

//static
struct timeval wa_tv;
//static
unsigned int wa_ts_nsec;

/*
#include <time.h>
#include <sched.h>

static struct timeval wa_tv;
static unsigned int wa_ts_nsec;

int wa_clock_gettime(clockid_t clockid, struct timespec *ts) {
    sched_yield();
    ts->tv_sec = wa_tv.tv_sec;
    ts->tv_nsec = wa_ts_nsec;
    fprintf(sc,"[\"clock_gettime\", %d, %d, %lu]\n", (int)clockid, (int)&ts, ts->tv_nsec);
    return 0;
}


int wa_gettimeofday(struct timeval *tv, struct timezone *tz) {

    sched_yield();
    memcpy( &tv, &wa_tv, sizeof(tv));

    //tv->tv_sec = 9223372036854775807 ;//

    fprintf(sc,"[\"gettimeofday\",%d,%d, %lld, %lld ]\n", (int)&tv, (int)&tz, tv->tv_sec, tv->tv_usec);
    return 0;
}
*/

void wa_setenv(const char *key, int value) {
    fprintf(kv,"{\"%s\":%d}\n", key, value);
}

void wa_syscall(const char *code) {
    fprintf(sc," %s\n", code);
    fflush(sc);
}

void __wa_env(const char *key, unsigned int sign, unsigned int bitwidth, void* addr ) {
    const char *csign[2] = { "i", "u" };
    fprintf(kv,"{\"%s\": [\"%s%u\", %u]}\n", key, csign[sign], bitwidth, (unsigned int)addr);
}

#define wa_env(key,x) __wa_env(key, ( x >= 0 && ~x >= 0 ), sizeof(x)*8 , &x )

void Py_Init() {
    cc = fdopen(3, "r+");
    kv = fdopen(4, "r+");
    sc = fdopen(5, "r+");

// do not call cpython names directly
    wPy_Initialize();
    wPy_NewInterpreter();

    wa_setenv("shm", (int)shm_ptr());

// js<->wa adapters tests
/*
    {
        unsigned char uc;  // beh
        wa_env("uc", uc ); // beh

        unsigned int ui;
        wa_env("ui", ui );

        unsigned long long ul;
        wa_env("ul", ul );
    }

    {
        signed char sc;
        wa_env("sc", sc );

        signed int si;
        wa_env("si", si );

        signed long long sl;
        wa_env("sl", sl );
    }
*/

    wa_env("tsec", wa_tv.tv_sec);
    wa_env("tusec", wa_tv.tv_usec );

    wa_env("tnsec", wa_ts_nsec);





    wa_setenv("PyRun_SimpleString_MAXSIZE", IO_KBD);
    wa_setenv("io_port_kbd", (int)&io_stdin[IO_KBD]);
    wa_setenv("MP_IO_SIZE", MP_IO_SIZE);
    wa_syscall("window.setTimeout( init_repl_begin , 1000 )");

    IO_CODE_DONE;

}

static bool def_PyRun_SimpleString_is_repl = false ;
static int async_loop = 1;
static int async_state;


extern int emscripten_GetProcAddress(const char * name);


#if MICROPY_OPT_CACHE_MAP_LOOKUP_IN_BYTECODE
static inline mp_map_elem_t *mp_map_cached_lookup(mp_map_t *map, qstr qst, uint8_t *idx_cache) {
    size_t idx = *idx_cache;
    mp_obj_t key = MP_OBJ_NEW_QSTR(qst);
    mp_map_elem_t *elem = NULL;
    if (idx < map->alloc && map->table[idx].key == key) {
        elem = &map->table[idx];
    } else {
        elem = mp_map_lookup(map, key, MP_MAP_LOOKUP);
        if (elem != NULL) {
            *idx_cache = (elem - &map->table[0]) & 0xff;
        }
    }
    return elem;
}
#endif


STATIC void stderr_print_strn2(void *env, const char *str, size_t len) {
    (void)env;
    mp_hal_stdout_tx_strn(str,len);
}

const mp_print_t mp_stderr_print2 = {NULL, stderr_print_strn2};

int uncaught_exception_handler(void) {
    mp_obj_base_t *exc = MP_STATE_THREAD(active_exception);
    // check for SystemExit
    if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(exc->type), MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
        // None is an exit value of 0; an int is its value; anything else is 1
        /*
        mp_obj_t exit_val = mp_obj_exception_get_value(MP_OBJ_FROM_PTR(exc));
        mp_int_t val = 0;
        if (exit_val != mp_const_none && !mp_obj_get_int_maybe(exit_val, &val)) {
            val = 1;
        }
        return FORCED_EXIT | (val & 255);
        */
        #if __EMSCRIPTEN__
                EM_ASM({console.log("91:SystemExit");});
        #endif

        return 1;
    }
    MP_STATE_THREAD(active_exception) = NULL;
    // Report all other exceptions
    cdbg("mp_stderr_print2=%p",exc)
    cdbg("mp_obj=%p", MP_OBJ_FROM_PTR(exc) );
    mp_obj_print_exception(&mp_stderr_print2, MP_OBJ_FROM_PTR(exc));
    return 0;
}

void
dump_args2(const mp_obj_t *a, size_t sz) {
    fprintf(stderr,"%p: ", a);
    for (size_t i = 0; i < sz; i++) {
        fprintf(stderr,"%p ", a[i]);
    }
    fprintf(stderr,"\n");
}


// this is reserved to max speed asynchronous code

void
noint_aio_fsync() {

    if (!io_stdin[0])
        return;

    if (!endswith(io_stdin, "#aio.step\n"))
        return;

    int ex=-1;
    async_state = VMFLAGS_IF;
    // CLI
    VMFLAGS_IF = 0;

    //TODO: maybe somehow consumme kbd data for async inputs ?
    //expect script to be properly async programmed and run them full speed via C stack ?

    if (async_loop) {

        if ( (async_loop = pyeval(i_main.shm_stdio, MP_PARSE_FILE_INPUT))  ) {
            ex=0;
        } else {
            fprintf(stdout, "ERROR[%s]\n", io_stdin);
            // ex check
            ex=1;
        }

    }

// TODO:    here we may able to tranform toplevel sync code to async and re eval
// WARNING: it may have side effects because could have run until async exception is caught
    if (ex>=0) {
        if (MP_STATE_THREAD(active_exception) != NULL) {
            clog("646: uncaught exception")
            //mp_hal_set_interrupt_char(-1);
            mp_handle_pending(false);
            if (uncaught_exception_handler()) {
                clog("651:SystemExit");
            } else {
                clog("653: exception done");
            }
            async_loop = 0;
        }
    }
    IO_CODE_DONE;
    // STI
    VMFLAGS_IF = async_state;
}



size_t
has_io() {
    size_t check = strlen(io_stdin);
  if (io_stdin[0] && (check != 38))
      return check;
  return 0;
}




//void
//main_loop_or_step(void) {



//***************************************************************************************



int PyArg_ParseTuple(PyObject *argv, const char *fmt, ...) {
    va_list argptr;
    va_start (argptr, fmt );
    vfprintf(stdout,fmt,argptr);
    va_end (argptr);
    return 0;
}



static int loops = 0;

//unsigned long long TV_NS = 16000;

int
main(int argc, char *argv[]) {
    //TV_NS += 16000;

    if (KPANIC!=0) {

        if (KPANIC>0) {

            g_argc = argc;
            g_argv = copy_argv(argc, argv);
            KPANIC = 0;
            // init
            crash_point = &&VM_stackmess;

            fprintf(stderr, "argv[0..%d] env=%s memory=%p\n", argc, getenv("WAPY"), shm_ptr() );

            if (argc) {
                printf("\nnode.js detected, running repl in infinite loop ...\n");
                // do not multiplex output
                cc = stdout;
            }

            #include "vmsl/vmwarmup.c"

            if (!argc) {
                // WASI syscall
                return 0;
            }

        } else {
            printf("\nno guru meditation [%d,%d]", getchar(), fgetc(cc) );
    // FIXME: exitcode
            return 1;
        }

    }

while (!KPANIC) {

    #include "../wapy/vmsl/vm_loop.c"

// node.js
    if(argc) {
        fflush(stdout);
        fgets( io_stdin, MP_IO_SHM_SIZE, stdin );
        continue;
    }

// wasi
    break;
}

    return 0;
} // main_loop_or_step




//



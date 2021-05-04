//C

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

#include "py/mpstate.h"

#include "../wapy/wapy.h"


#define DBG 0
#define DLOPEN 0

#if !MICROPY_ENABLE_PYSTACK
    #error "need MICROPY_ENABLE_PYSTACK (1)
#endif




extern char* shm_ptr();

#ifdef __EMSCRIPTEN__
    #include "emscripten.h"
#else
    #define EMSCRIPTEN_KEEPALIVE
#endif



extern mp_obj_t fun_bc_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args);
extern mp_vm_return_kind_t mp_execute_bytecode(mp_code_state_t *code_state, volatile mp_obj_t inject_exc);
extern mp_obj_t gen_instance_iternext(mp_obj_t self_in);
extern int mp_call_prepare_args_n_kw_var(bool have_self, size_t n_args_n_kw, const mp_obj_t *args, mp_call_args_t *out_args);

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
extern qstr trace_prev_file ;
extern size_t trace_prev_line;

extern qstr trace_file ;
extern size_t trace_line;

extern int trace_on;


#define io_stdin i_main.shm_stdio
FILE *cc;
FILE *fd_keyvalue;
FILE *fd_syscall;
extern FILE *fd_logger;


#include "../wapy/upython.c"




// to use kb buffer space for scripts
// if (strlen(io_stdin)>=IO_KBD){ io_stdin[IO_KBD]  = 0;
#define IO_CODE_DONE { io_stdin[0] = 0; }

#if MICROPY_PY_THREAD_GIL && MICROPY_PY_THREAD_GIL_VM_DIVISOR
// This needs to be volatile and outside the VM loop so it persists across handling
// of any exceptions.  Otherwise it's possible that the VM never gives up the GIL.
volatile int gil_divisor = MICROPY_PY_THREAD_GIL_VM_DIVISOR;
#endif



// =================== wasi support ===================================

/*
struct timespec t_timespec;
struct timeval t_timeval;

*/
extern unsigned int wa_ts_nsec;
extern struct timeval wa_tv;


void
wa_setenv(const char *key, int value) {
    fprintf(fd_keyvalue,"{\"%s\":%d}\n", key, value);
}

void
wa_syscall(const char *code) {
    fprintf(fd_syscall," %s\n", code);
}

void
__wa_env(const char *key, unsigned int sign, unsigned int bitwidth, void* addr ) {
    const char *csign[2] = { "i", "u" };
    fprintf(fd_keyvalue,"{\"%s\": [\"%s%u\", %u]}\n", key, csign[sign], bitwidth, (unsigned int)addr);
}

#define wa_env(key,x) __wa_env(key, ( x >= 0 && ~x >= 0 ), sizeof(x)*8 , &x )

//===========================================================================




#if __EMSCRIPTEN__
    #include "../wapy/debug.c"

    #pragma message "get rid of that EM_ASM emscripten-way"
    // can't be in loop body because of EM_ASM
    void Py_Init() {
        fd_logger =  fdopen(2, "w"); // no we won't close, it's a webpage ...
        wPy_Initialize();
        wPy_NewInterpreter();

        EM_ASM( {
            vm.aio.plink.MAXSIZE = $0;
            vm.aio.plink.shm = $1;
            vm.aio.plink.io_port_kbd = $2;
            vm.aio.plink.MP_IO_SIZE = $3;
            console.log("aio.plink.shm=" + vm.aio.plink.shm+" +" + vm.aio.plink.MAXSIZE);
            console.log("aio.plink.io_port_kbd=" + vm.aio.plink.io_port_kbd+" +"+ vm.aio.plink.MP_IO_SIZE);
            window.setTimeout( vm.script.init_repl, 1000 );
        }, IO_KBD, shm_ptr(), &io_stdin[IO_KBD], MP_IO_SIZE);

        IO_CODE_DONE;

    }
#else

void Py_Init() {
#if defined(__WASI__)
    cc = fdopen(3, "r+");
    fd_keyvalue = fdopen(4, "r+");
    fd_syscall = fdopen(5, "r+");
    fd_logger =  fdopen(6, "r+");
#else
    cc = fdopen(2, "r+");
    fd_keyvalue = cc;
    fd_syscall = cc;
    fd_logger =  cc;
#endif

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

#endif


static bool def_PyRun_SimpleString_is_repl = false ;
static int async_loop = 1;
static int async_state;




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


mp_state_ctx_t mp_state_ctx;

STATIC void stderr_print_strn2(void *env, const char *str, size_t len) {
    (void)env;
    mp_hal_stdout_tx_strn(str,len);
}

const mp_print_t mp_stderr_print2 = {NULL, stderr_print_strn2};

#define PYV(v) pyv((mp_obj_t)v)

// user defined exception handler
int uncaught_exception_handler(void) {
    mp_obj_base_t *ex = MP_STATE_THREAD(active_exception);
    // check for SystemExit
    if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(ex->type), MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
        // None is an exit value of 0; an int is its value; anything else is 1
        /*
        mp_obj_t exit_val = mp_obj_exception_get_value(MP_OBJ_FROM_PTR(exc));
        mp_int_t val = 0;
        if (exit_val != mp_const_none && !mp_obj_get_int_maybe(exit_val, &val)) {
            val = 1;
        }
        return FORCED_EXIT | (val & 255);
        */
        cdbg("499:SystemExit");
        return 1;
    }
    MP_STATE_THREAD(active_exception) = NULL;

    // fake sys.excepthook via pythons.__init__
    cdbg("57: *** EXCEPTION ***\n");
    PYV( mp_obj_get_type(ex) );
    PYV( MP_OBJ_FROM_PTR(ex) );
    PYV( MP_ROM_NONE );
    pycore("pyc_excepthook");
    return 0;
}


void
dump_args2(const mp_obj_t *a, size_t sz) {
    cdbg("331: %p: ", a);
    for (size_t i = 0; i < sz; i++) {
        fprintf(fd_logger,"%p ", a[i]);
    }
    fprintf(fd_logger,"\n");
}



// this is reserved to max speed asynchronous code

int
noint_aio_fsync() {

    if (!io_stdin[0])
        return -1;

    if (!endswith(io_stdin, "#aio.step\n"))
        return -1;

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

    // STI
    VMFLAGS_IF = async_state;
    return ex;
}



int
endswith(const char * str, const char * suffix) {
  int str_len = strlen(str);
  int suffix_len = strlen(suffix);

  return
    (str_len >= suffix_len) && (0 == strcmp(str + (str_len-suffix_len), suffix));
}



// do not run lines starting with #
size_t
has_io() {
    size_t check = strlen(io_stdin);
    if (io_stdin[0] && (check != 38))
        return check;
    return 0;
}










//

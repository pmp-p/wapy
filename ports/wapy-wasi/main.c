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


#include "../wapy/wapy.h"

#define DBG 0
#define DLOPEN 0

#if !MICROPY_ENABLE_PYSTACK
#error "need MICROPY_ENABLE_PYSTACK (1)
#endif


static int g_argc;
static char **g_argv; //[];



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
        length += (strlen(argv[i]) + 1);
    }
    char** new_argv = (char**)malloc((ptr_args) * sizeof(char*) + length);
    // copy argv into the contiguous buffer
    length = 0;
    for (int i = 0; i < argc; i++) {
        new_argv[i] = &(((char*)new_argv)[(ptr_args * sizeof(char*)) + length]);
        strcpy(new_argv[i], argv[i]);
        cdbg("52: argv[%d] = %s\n", i, argv[i]);
        length += (strlen(argv[i]) + 1);
    }
    // insert NULL terminating ptr at the end of the ptr array
    new_argv[ptr_args-1] = NULL;
    return (new_argv);
}


#include "../wapy/upython.c"


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
FILE *fd_keyvalue;
FILE *fd_syscall;
FILE *fd_logger;

// to use kb buffer space for scripts
// if (strlen(io_stdin)>=IO_KBD){ io_stdin[IO_KBD]  = 0;
#define IO_CODE_DONE { io_stdin[0] = 0; }

#if MICROPY_PY_THREAD_GIL && MICROPY_PY_THREAD_GIL_VM_DIVISOR
// This needs to be volatile and outside the VM loop so it persists across handling
// of any exceptions.  Otherwise it's possible that the VM never gives up the GIL.
volatile int gil_divisor = MICROPY_PY_THREAD_GIL_VM_DIVISOR;
#endif



// =================== wasi support ===================================
struct timespec t_timespec;
struct timeval t_timeval;
struct timeval wa_tv;
unsigned int wa_ts_nsec;

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






void Py_Init() {
    cc = fdopen(3, "r+");
    fd_keyvalue = fdopen(4, "r+");
    fd_syscall = fdopen(5, "r+");
    fd_logger =  fdopen(6, "r+");

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

#include "../wapy/wapy.c"



//***************************************************************************************

int io_encode_hex = 1;
//static int loops = 0;


#ifdef __ARDUINO__
    #ifndef __CPP__
        #include ARDUINO_HAL
    #endif
#endif






int
main(int argc, char *argv[]) {

    if (KPANIC!=0) {

        if (KPANIC>0) {

            g_argc = argc;
            g_argv = copy_argv(argc, argv);

            if (argc)
                io_encode_hex = !argc;

            KPANIC = 0;
            // init
            crash_point = &&VM_stackmess;

            cdbg("270: argv[0..%d] env=%s memory=%p\n", argc, getenv("WAPY"), shm_ptr() );

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
} // main_iteration




//



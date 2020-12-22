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



/*
LOGO:
    https://hackernoon.com/screamin-speed-with-webassembly-b30fac90cd92
    https://www.gatherdigital.co.uk/news/2016/08/an-early-look-at-webassembly

Python and web:
    https://twitter.com/sfermigier/status/1193457847008403456
    https://www.mail-archive.com/web-sig@python.org/msg04347.html
    https://bugs.python.org/issue40280
    https://discuss.python.org/t/python-in-the-browser/4248/6

OS ? :
    https://github.com/S2E/PyKVM
    https://github.com/plasma-umass/browsix

NOGIL:
    https://github.com/plasma-umass/snakefish

THE USER POINT OF VIEW:
    https://danluu.com/input-lag/


BOOKMARKS:

https://faster-cpython.readthedocs.io/implementations.html

https://www.aosabook.org/en/500L/a-python-interpreter-written-in-python.html

https://doc.pypy.org/en/latest/interpreter.html#introduction-and-overview

https://github.com/stackless-dev/stackless/tree/master-slp/Stackless

transpile/compile :
    https://github.com/almarklein/wasmfun
    https://github.com/pfalcon/pycopy-lib/tree/master/utokenize
    https://github.com/pfalcon/ullvm_c
    https://ppci.readthedocs.io/en/latest/reference/lang/python.html
    https://pypi.org/project/py-ts-interfaces/
    https://github.com/numba/numba/issues/3284
    https://00f.net/2019/04/07/compiling-to-webassembly-with-llvm-and-clang/

    https://github.com/pfalcon/awesome-python-compilers

    https://docs.python.org/3/library/inspect.html#retrieving-source-code

    https://01alchemist.com/projects/turboscript/playground/


    https://github.com/lark-parser/lark/blob/master/examples/python_parser.py
    https://github.com/lark-parser/lark

    https://github.com/pyparsing/pyparsing

    https://wiki.python.org/moin/LanguageParsing
    https://github.com/timothycrosley/jiphy
    https://github.com/philhassey/tinypy


    http://gambitscheme.org/wiki/index.php/Main_Page


jit:
    http://llvmlite.pydata.org/en/latest/

interfacing:

    https://foss.heptapod.net/pypy/cffi
    https://github.com/saghul/wasi-lab

native use:
    https://github.com/wasmerio/python-ext-wasm
    https://pypi.org/project/wasmbind/

remote use:
    https://pypi.org/project/jsii/

    https://www.plynth.net/docs?id=async_await

sixel support for remote
    https://github.com/risacher/ttyx/issues/15


ilyaigpetrov/ncurses-for-emscripten
ncurses 6.1 compiled by emscripten for usage in a browser.
It is compiled, loaded, but doesn't work! You are wanted to make it work!
    https://github.com/ilyaigpetrov/ncurses-for-emscripten


------------- architecture / runtimes -------------------------

Stackless design?
    https://github.com/micropython/micropython/issues/1036

Tasking for Emscripten/Wasm target #32532
    https://github.com/JuliaLang/julia/pull/32532/files#diff-15aaf3bb4e0956d73f314b03ddf85c8cR92

hyperdivision/async-wasm
    https://github.com/hyperdivision/async-wasm/blob/master/index.js


wasm/gc/continuations:
    https://github.com/WebAssembly/exception-handling/blob/master/proposals/Exceptions.md
    https://emscripten.org/docs/porting/guidelines/function_pointer_issues.html
    https://github.com/emscripten-core/emscripten/issues/8268#
    http://troubles.md/wasm-is-not-a-stack-machine/
    https://github.com/WebAssembly/design/issues/919#issuecomment-348000242


    https://bitbucket.org/pypy/stmgc/src/default/

    https://github.com/WebAssembly/design/issues/1345#issuecomment-638228041

    finalizers: thx stinos !
        https://github.com/micropython/micropython/issues/1878

    WAPY implements C3:
        https://github.com/WebAssembly/WASI/issues/276
        https://github.com/WebAssembly/design/issues/1252#issuecomment-461604032

Protothreads
    http://dunkels.com/adam/pt/
    http://dunkels.com/adam/download/graham-pt.h

C tools:
    https://github.com/jamesmunns/bbqueue
    https://github.com/willemt/bipbuffer/blob/master/bipbuffer.c

    A circular buffer written in C using Posix calls to create a contiguously mapped memory space.
        https://github.com/willemt/cbuffer

    https://vstinner.readthedocs.io/c_language.html


ASYNC:
    https://github.com/python-trio/trio/issues/79
    https://www.encode.io/httpx/async/
    https://www.python.org/dev/peps/pep-0492/
    https://www.python.org/dev/peps/pep-0525/
    https://www.pythonsheets.com/notes/python-asyncio.html

    https://journal.stuffwithstuff.com/2015/02/01/what-color-is-your-function/

    Re: [PATCH 09/13] aio: add support for async openat()
    [Posted January 12, 2016 by corbet]

    https://lwn.net/Articles/671657/
    "In fact, if we do it well, we can go the other way, and try to
implement the nasty AIO interface on top of the generic "just do
things asynchronously". Linus


VM support:
    https://github.com/cretz/asmble

    https://github.com/CraneStation/wasmtime

    https://nick.zoic.org/art/web-assembly-on-esp32-with-wasm-wamr/
    https://github.com/zephyrproject-rtos/zephyr/issues/21329

    https://github.com/vshymanskyy/Wasm3_RGB_Lamp


https://snarky.ca/what-is-the-core-of-the-python-programming-language/


pywasm: Support WASI(WebAssembly System Interface)
    https://github.com/mohanson/pywasm/issues/25

webuse:
    https://developer.mozilla.org/en-US/docs/Web/API/Page_Visibility_API
    https://makitweb.com/how-to-detect-browser-window-active-or-not-javascript/
    https://codepen.io/jonathan/full/sxgJl

repl:
    Unicode Character 'RIGHT-TO-LEFT OVERRIDE' (U+202E)

couldclose:
    https://github.com/micropython/micropython/issues/3313

net layer:
    https://github.com/moshest/p2p-index
    https://rangermauve.hashbase.io/



    https://github.com/iodide-project/pyodide/pull/606/files
    https://github.com/pyhandle/piconumpy/tree/hpy

*/





/*
CFLAGS="-Wfatal-errors -Wall -Wextra -Wunused -Werror -Wno-format-extra-args -Wno-format-zero-length\
 -Winit-self -Wimplicit -Wimplicit-int -Wmissing-include-dirs -Wswitch-default -Wswitch-enum\
 -Wunused-parameter -Wdouble-promotion -Wchkp -Wno-coverage-mismatch -Wstrict-overflow\
 -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wnonnull -Wnonnull-compare\
 -Wnull-dereference -Wignored-qualifiers -Wignored-attributes\
 -Wmain -Wpedantic -Wmisleading-indentation -Wmissing-braces -Wmissing-include-dirs\
 -Wparentheses -Wsequence-point -Wshift-overflow=2 -Wswitch -Wswitch-default -Wswitch-bool\
 -Wsync-nand -Wunused-but-set-parameter -Wunused-but-set-variable -Wunused-function -Wunused-label\
 -Wunused-parameter -Wunused-result -Wunused-variable -Wunused-const-variable=2 -Wunused-value\
 -Wuninitialized -Winvalid-memory-model -Wmaybe-uninitialized -Wstrict-aliasing=3\
 -Wsuggest-attribute=pure -Wsuggest-attribute=const\
 -Wsuggest-attribute=noreturn -Wsuggest-attribute=format -Wmissing-format-attribute\
 -Wdiv-by-zero -Wunknown-pragmas -Wbool-compare -Wduplicated-cond\
 -Wtautological-compare -Wtrampolines -Wfloat-equal -Wfree-nonheap-object -Wunsafe-loop-optimizations\
 -Wpointer-arith -Wnonnull-compare -Wtype-limits -Wcomments -Wtrigraphs -Wundef\
 -Wendif-labels -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wclobbered\
 -Wconversion -Wdate-time -Wempty-body -Wjump-misses-init -Wsign-compare -Wsign-conversion\
 -Wfloat-conversion -Wsizeof-pointer-memaccess -Wsizeof-array-argument -Wpadded -Wredundant-decls\
 -Wnested-externs -Winline -Wbool-compare -Wno-int-to-pointer-cast -Winvalid-pch -Wlong-long\
 -Wvariadic-macros -Wvarargs -Wvector-operation-performance -Wvla -Wvolatile-register-var\
 -Wpointer-sign -Wstack-protector -Woverlength-strings -Wunsuffixed-float-constants\
 -Wno-designated-init -Whsa\
 -march=x86-64 -m64 -Wformat=2 -Warray-bounds=2 -Wstack-usage=120000 -Wstrict-overflow=5 -fmax-errors=5 -g\
 -std=c99 -D_POSIX_SOURCE -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=700 -pedantic-errors"
*/

#define DBG 0
#define DLOPEN 0


#if !MICROPY_ENABLE_PYSTACK
#error "need MICROPY_ENABLE_PYSTACK (1)
#endif

//static int SHOW_OS_LOOP=0;

static int g_argc;
static char **g_argv; //[];

size_t
bsd_strlen(const char *str) {
        const char *s;
        for (s = str; *s; ++s);
        return (s - str);
}


#if 0 // for esp broken SDK strcmp

    int
    bsd_strcmp(const char *s1, const char *s2) {
        while (*s1 == *s2++)
            if (*s1++ == '\0')
                return (0);
        return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
    }

    int
    endswith(const char * str, const char * suffix) {
      int str_len = bsd_strlen(str);
      int suffix_len = bsd_strlen(suffix);

      return
        (str_len >= suffix_len) && (0 == bsd_strcmp(str + (str_len-suffix_len), suffix));
    }
#endif

int
endswith(const char * str, const char * suffix) {
  int str_len = strlen(str);
  int suffix_len = strlen(suffix);

  return
    (str_len >= suffix_len) && (0 == strcmp(str + (str_len-suffix_len), suffix));
}





/* =====================================================================================
    bad sync experiment with file access trying to help on
        https://github.com/littlevgl/lvgl/issues/792

    status: better than nothing, but wapy can/could/must use aio_* for that.
*/

#include "api/wasm_file_api.c"
#include "api/wasm_import_api.c"

static int KPANIC = 0;







char **copy_argv(int argc, char *argv[]) {
  // calculate the contiguous argv buffer size
  int length=0;
  size_t ptr_args = argc + 1;
  for (int i = 0; i < argc; i++)
  {
    length += (bsd_strlen(argv[i]) + 1);
  }
  char** new_argv = (char**)malloc((ptr_args) * sizeof(char*) + length);
  // copy argv into the contiguous buffer
  length = 0;
  for (int i = 0; i < argc; i++)
  {
    new_argv[i] = &(((char*)new_argv)[(ptr_args * sizeof(char*)) + length]);
    strcpy(new_argv[i], argv[i]);
    length += (bsd_strlen(argv[i]) + 1);
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

// to use kb buffer space for scripts
// if (strlen(io_stdin)>=IO_KBD){ io_stdin[IO_KBD]  = 0;
#define IO_CODE_DONE { io_stdin[0] = 0; }

#if MICROPY_PY_THREAD_GIL && MICROPY_PY_THREAD_GIL_VM_DIVISOR
// This needs to be volatile and outside the VM loop so it persists across handling
// of any exceptions.  Otherwise it's possible that the VM never gives up the GIL.
volatile int gil_divisor = MICROPY_PY_THREAD_GIL_VM_DIVISOR;
#endif


// can't be in loop body because of EM_ASM
void Py_Init() {

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

/*
STATIC void stderr_print_strn2(void *env, const char *str, size_t len) {
    (void)env;
    mp_hal_stdout_tx_strn(str,len);

}

const mp_print_t mp_stderr_print2 = {NULL, stderr_print_strn2};
*/

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
        clog("499:SystemExit");
        return 1;
    }
    MP_STATE_THREAD(active_exception) = NULL;

    // fake sys.excepthook via pythons.__init__
    fprintf(stderr, "*** EXCEPTION ***\n");
    pyv( mp_obj_get_type(ex) );
    pyv( MP_OBJ_FROM_PTR(ex) );
    pyv( MP_ROM_NONE );
    pycore("pyc_excepthook");
    return 0;
}

void
dump_args2(const mp_obj_t *a, size_t sz) {
    fprintf(stderr,"481: %p: ", a);
    for (size_t i = 0; i < sz; i++) {
        fprintf(stderr,"%p ", a[i]);
    }
    fprintf(stderr,"\n");
}


// this is reserved to max speed asynchronous code

int
noint_aio_fsync() {

    if (!io_stdin[0])
        return 0;

    if (!endswith(io_stdin, "#aio.step\n"))
        return 0;

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
/*
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
    */
    // STI
    VMFLAGS_IF = async_state;
    return ex;
}



size_t
has_io() {
    size_t check = strlen(io_stdin);
  if (io_stdin[0] && (check != 38))
      return check;
  return 0;
}


extern int pyexec_repl_repl_restart(int ret);
extern int pyexec_friendly_repl_process_char(int c);

int
main_iteration(void) {
    if (VMOP <= VMOP_INIT) {
        crash_point = &&VM_stackmess;
        #include "vmsl/vmwarmup.c"
        return 0;
    }

    #define cc stdout
    #include "../wapy/vmsl/vm_loop.c"
    #undef cc
    return 0;

} // main_iteration

/*
int
main_loop_warmup(void) {
    #include "vmsl/vmwarmup.c"
} // main_loop_warmup
*/

//***************************************************************************************



#include <dlfcn.h>

void *
import(const char * LIB_NAME) {
    void *lib_handle = dlopen(LIB_NAME, RTLD_NOW | RTLD_GLOBAL);

    if (!lib_handle)
        fprintf(stderr,"\n\nDL ======> cannot load %s module\n\n\n", LIB_NAME);
    else
        fprintf(stderr,"\n\nDL ======> %s module OK !!!!! \n\n\n", LIB_NAME);
    return lib_handle;
}


int
main(int argc, char *argv[]) {

    g_argc = argc;
    g_argv = copy_argv(argc, argv);

    //assert(emscripten_run_preload_plugins(LIB_NAME, NULL, NULL) == 0);
    //void *lib_wapy = import( "libwapy.so");
    //void *lib_sdl2 = import( "libsdl2.so");

    emscripten_set_main_loop( (em_callback_func)(void*)main_iteration, 0, 1);
    return 0;
}




























//



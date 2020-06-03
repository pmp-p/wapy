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



/* full shared

#include "py/mpstate.h"  // mp_state_ctx
mp_state_ctx_t mp_state_ctx;

*/

/*
LOGO:
    https://hackernoon.com/screamin-speed-with-webassembly-b30fac90cd92
    https://www.gatherdigital.co.uk/news/2016/08/an-early-look-at-webassembly

Python and web:
    https://twitter.com/sfermigier/status/1193457847008403456
    https://www.mail-archive.com/web-sig@python.org/msg04347.html
    https://bugs.python.org/issue40280
    https://discuss.python.org/t/python-in-the-browser/4248/6

BOOKMARKS:

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


wasm:
    https://github.com/WebAssembly/exception-handling/blob/master/proposals/Exceptions.md
    https://emscripten.org/docs/porting/guidelines/function_pointer_issues.html
    https://github.com/emscripten-core/emscripten/issues/8268#
    http://troubles.md/wasm-is-not-a-stack-machine/

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

GC:
    https://bitbucket.org/pypy/stmgc/src/default/


VM support:
    https://github.com/cretz/asmble

    https://github.com/CraneStation/wasmtime

pywasm: Support WASI(WebAssembly System Interface)
    https://github.com/mohanson/pywasm/issues/25

webuse:
    https://developer.mozilla.org/en-US/docs/Web/API/Page_Visibility_API
    https://makitweb.com/how-to-detect-browser-window-active-or-not-javascript/
    https://codepen.io/jonathan/full/sxgJl
*/



#include "emscripten.h"

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

static int SHOW_OS_LOOP=0;

static int g_argc;
static char **g_argv; //[];

size_t
bsd_strlen(const char *str) {
        const char *s;
        for (s = str; *s; ++s);
        return (s - str);
}


#ifndef strcmp
#pragma message " ----------- missing strcmp ------------ "
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
#else

int
endswith(const char * str, const char * suffix) {
  int str_len = strlen(str);
  int suffix_len = strlen(suffix);

  return
    (str_len >= suffix_len) && (0 == strcmp(str + (str_len-suffix_len), suffix));
}

#endif

/* =====================================================================================
    bad sync experiment with file access trying to help on
        https://github.com/littlevgl/lvgl/issues/792

    status: better than nothing.
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

#define io_stdin i_main.shm_stdio

#define IO_CODE_DONE { io_stdin[0] = 0; }

#if MICROPY_PY_THREAD_GIL && MICROPY_PY_THREAD_GIL_VM_DIVISOR
// This needs to be volatile and outside the VM loop so it persists across handling
// of any exceptions.  Otherwise it's possible that the VM never gives up the GIL.
volatile int gil_divisor = MICROPY_PY_THREAD_GIL_VM_DIVISOR;
#endif


// can't be in loop body because of EM_ASM
void Py_Init() {

#if 0
    fprintf(stdout,"Testing STDOUT : ");
    printf("\nstdout\n");
    fprintf(stdout,"done\n");

    fprintf(stdout,"Testing STDERR : ");
    fprintf(stderr,"\nstderr\n");
    fprintf(stdout,"done\n");
#endif

    wPy_Initialize();
    wPy_NewInterpreter();

    EM_ASM( {
        window.plink.shm = $0;
        window.plink.io_port_kbd = $1;
        window.PyRun_SimpleString_MAXSIZE = $2;
        console.log("window.plink.shm=" + window.plink.shm);
        console.log("window.plink.io_port_kbd=" + window.plink.io_port_kbd);
        window.setTimeout( init_repl_begin , 1000 );
    }, shm_ptr(), shm_get_ptr(0,0), MP_IO_SHM_SIZE );

    IO_CODE_DONE;

}


static bool def_PyRun_SimpleString_is_repl = false ;
static int async_loop = 1;
static int async_state;


extern int emscripten_GetProcAddress(const char * name);

#if USE_SDL

#include <SDL2/SDL.h>
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_bool done = SDL_FALSE;


void SDL_test(){
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        if (SDL_CreateWindowAndRenderer(320, 200, 0, &window, &renderer) == 0) {
            //emscripten_set_main_loop( loop, 0, 1);
        }
    }
}

void SDL_loop(){
    if (!done) {
        SDL_Event event;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawLine(renderer, 220, 100, 200, 140);
        SDL_RenderDrawLine(renderer, 200, 140, 240, 140);
        SDL_RenderDrawLine(renderer, 240, 140, 220, 100);
        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = SDL_TRUE;
            }
        }
    } else {
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
     // else emscripten_exit
    }

}


#include "dlfcn.h"

#define DL_PATH "/lib/lib"
#define DL_SUFFIX ".so"

#define LIB_NAME "SDL2"

#include <SDL2/SDL.h>
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_bool done = SDL_FALSE;


static void *lib_handle = NULL;

uintptr_t SDL_embed(uintptr_t *ptr){
    if (!lib_handle) {
        clog("SDL dlopen");
        lib_handle = dlopen(NULL, RTLD_NOW | RTLD_GLOBAL);
        //return 0;
    }

    if (!lib_handle)
        fprintf(stderr,"cannot load %s module", LIB_NAME);
    else {
            //void (*SDL_Init)(void) = (void (*)(void))dlsym(lib_handle, "SDL_Init");
            int (*SDL_Init)(int) = (int (*)(int))dlsym(lib_handle, "SDL_Init");
            int res = SDL_Init(0x20);
            if (!res) {
                if (SDL_CreateWindowAndRenderer(320, 200, 0, &window, &renderer) == 0) {
                    *ptr = renderer;
                    clog(" ============ OK rd = %p %p ===============", renderer, ptr);

                }

            } else
                clog("SDL_Init= %i", res );

        /*
        {
            void (*fun_ptr)(void) = (void (*)(void))dlsym(lib_handle, "init_test");
            fun_ptr();
        }

        {
            int (*fun_ptr)(int) = (int (*)(int))dlsym(lib_handle, "init_plus_one");
            if (fun_ptr) {
                fprintf(stdout,"DLTEST 1+1 = %i", fun_ptr(1) );
            }
        }
        */
    }
    return (uintptr_t)renderer;
}
#else

uintptr_t SDL_embed(uintptr_t *ptr){
    return 0;
}

#endif //USE_SDL

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

void
main_loop_or_step(void) {
//call_:;
    if (VMOP < VMOP_WARMUP) {
        if (VMOP < VMOP_INIT) {
            puts("init");
            crash_point = &&VM_stackmess;

            Py_Init();

            VMOP = VMOP_INIT;

            entry_point[0]=JMP_NONE;
            exit_point[0]=JMP_NONE;
            come_from[0]=0;
            type_point[0]=0;

            for (int i=0; i<SYS_MAX_RECURSION; i++)
                mp_new_interpreter(&mpi_ctx, i, 0 , 0);

            // 0 hypervisor with no branching ( can use optimized original vm.c with no hard int )
            // 1 supervisor
            // 2 __main__

            mp_new_interpreter(&mpi_ctx, 1, 0, 2);

            // 2 has no parent for now, just back to OS
            mp_new_interpreter(&mpi_ctx, 2, 0, 0);
            ctx_current = 1;

            while ( mpi_ctx[ctx_current].childcare ) {
                ctx_current = (int)mpi_ctx[ctx_current].childcare;
            }

            fprintf(stdout,"running __main__ on pid=%d\n", ctx_current);

            return;
        } // no continuation -> syscall

        if (VMOP==VMOP_INIT) {
            puts("VMOP_INIT");

            VMOP = VMOP_WARMUP;
            show_os_loop(1);
            // help fix lack of vars()
            PyRun_SimpleString(
                "__dict__ = globals();"
                "import sys;"
                "import embed;"
                "import builtins;"
                "sys.path.extend(['/assets','/assets/packages']);"
                "import site_wapy;"
                "#\n"
            );
            return;
        }
    } else {
        //syscall = 2 cycles , pause = 1,
        if (VMOP>= VMOP_PAUSE) {
            VMOP--;
            return;
        }

        if ( (ENTRY_POINT != JMP_NONE)  && !JUMPED_IN) {
            clog("re-enter-on-entry %d => %d\n", ctx_current, CTX.pointer);
            void* jump_entry;
            jump_entry = ENTRY_POINT;
            // Never to re-enter as this point. can only use the exit.
            JUMPED_IN = 1;
            goto *jump_entry;
        }

        // allow that here ?
        if ( (EXIT_POINT != JMP_NONE)  && JUMPED_IN) {
            clog("re-enter-on-exit %d => %d\n", ctx_current, CTX.pointer);

            // was it gosub
            if (JUMP_TYPE == TYPE_SUB)
                RETURN;

            // was it branching
            if (JUMP_TYPE == TYPE_JUMP)
                COME_FROM;
        }
    }


// this block is the async loop , no preemption should be allowed here.

    if (io_stdin[0]) {
        int ex=-1;
        async_state = VMFLAGS_IF;
        // CLI
        VMFLAGS_IF = 0;

        //is it async top level ? let python access shared mem and rewrite code
        if (endswith(io_stdin, "#async-tl")) {
            cdbg("#async-tl -> aio.asyncify()");
            PyRun_SimpleString("print('aio.asyncify N/I')");
            ex=0;
            IO_CODE_DONE;
        } else {
            if (endswith(io_stdin, "#aio.step\n")) {
                //TODO: maybe somehow consumme kbd data for async inputs ?
                //expect script to be properly async programmed and run them full speed via C stack ?

                if (async_loop) {
                    if ( (async_loop = PyRun_IO_CODE()) ) {
                        ex=0;
                    } else {
                        fprintf(stdout, "ERROR[%s]\n", io_stdin);
                        // ex check
                        ex=1;
                    }
                }
                IO_CODE_DONE;
            }

        }

        if (ex>=0) {
            if (MP_STATE_THREAD(active_exception) != NULL) {
                clog("646: uncaught exception")
                //mp_hal_set_interrupt_char(-1);
                mp_handle_pending(false);
                //handle_uncaught_exception();
                if (uncaught_exception_handler()) {
                    clog("651:SystemExit");
                } else {
                    clog("653: exception done");
                }
                async_loop = 0;
            }
        }
        // STI
        VMFLAGS_IF = async_state;
    }
/*
    // preemption block for repl and scripts.

    if (io_stdin[0]) {
        //then it is toplevel or it's sync top level ( tranpiled by aio on the heap)
/// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
            JUMP( def_PyRun_SimpleString, "main_loop_or_step");
/// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
            // mark done
        IO_CODE_DONE;
    }
*/

    //now flush kbd port
    char *keybuf;
    keybuf = shm_get_ptr( IO_KBD, 0);
    // only when scripting interface is idle and repl ready
    while (!KPANIC && repl_started) {
        // should give a way here to discard repl events feeding  "await input()" instead
        int rx =  keybuf[0] ; //& 0xFF;
        if (rx) {
            // if (rx==12) fprintf(stdout,"IO(12): Form Feed "); //clear screen
            //if (rx>127) cdbg("FIXME:stdin-utf8:%u", rx );
            //pyexec_event_repl_process_char(rx);
            if (pyexec_friendly_repl_process_char(rx)<0)
                cdbg("REPL[%s]", io_stdin);
            *keybuf++ = 0;
        } else break;
    }


    if (io_stdin[0]) {
    //then it is toplevel or it's sync top level ( tranpiled by aio on the heap)
        def_PyRun_SimpleString_is_repl = true;
/// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
        JUMP( def_PyRun_SimpleString, "main_loop_or_step_repl");
/// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
        // mark done
        IO_CODE_DONE;
        pyexec_repl_repl_restart(0);
        def_PyRun_SimpleString_is_repl = false;
    }

    if (VMOP==VM_HCF) {
VM_stackmess:
        puts("no guru meditation, bye");
        #if !ASYNCIFY
        emscripten_cancel_main_loop();
        #endif
    }
goto VM_syscall;

//==================================================================


// def PyRun_SimpleString(const_char_p src) -> int;
def_PyRun_SimpleString: {
//return:
    // should set a global integer
    //int ret = 0;
//args:
    char* src = i_main.shm_stdio;
    mp_parse_input_kind_t input_kind = MP_PARSE_FILE_INPUT;

//vars

//code
    mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);

    if (lex == NULL) {
        clog("syntax error");
        handle_uncaught_exception();
    } else {
        qstr source_name = lex->source_name;

        mp_obj_t exret = MP_OBJ_NULL;

        #if MICROPY_PY___FILE__
        if (input_kind == MP_PARSE_FILE_INPUT) {
            mp_store_global(MP_QSTR___file__, MP_OBJ_NEW_QSTR(source_name));
        }
        #endif

        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);

        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, def_PyRun_SimpleString_is_repl);


        if ( module_fun != MP_OBJ_NULL) {
            //STACKLESS STARTS HERE
            //mp_obj_t exret = mp_call_function_0(module_fun);
#if 0
            ctx_get_next(CTX_NEW);
            NEXT.self_in = module_fun;
            NEXT.n_args = 0;
            NEXT.n_kw = 0;
            NEXT.args = NULL ;
                GOSUB(def_mp_call_function_n_kw,"def_PyRun_SimpleString");
                exret = SUBVAL;
#else
            CTX.self_in = module_fun;
            CTX.n_args = 0;
            CTX.n_kw = 0;
            CTX.args = NULL ;

            const mp_obj_type_t *type = mp_obj_get_type(CTX.self_in);

            exret = MP_OBJ_NULL;

            if (type->call != NULL) {
                if ( (int)*type->call == (int)&fun_bc_call ) {
                    ctx_get_next(CTX_COPY);
                        GOSUB(def_func_bc_call, "mp_call_function_n_kw");
                        exret = SUBVAL; //CTX.sub_value;
                } else {
                    exret = type->call(CTX.self_in, CTX.n_args, CTX.n_kw, CTX.args);
                }

            } else {
                    mp_raise_o(
                        mp_obj_new_exception_msg_varg(
                            &mp_type_TypeError,"'%s' object isn't callable", mp_obj_get_type_str(CTX.self_in)
                        )
                    );
            }
#endif
            if ( exret != MP_OBJ_NULL ) {
                if (MP_STATE_VM(mp_pending_exception) != MP_OBJ_NULL) {
                    clog("645: PENDING EXCEPTION CLEARED AND RAISED");
                    mp_obj_t obj = MP_STATE_VM(mp_pending_exception);
                    MP_STATE_VM(mp_pending_exception) = MP_OBJ_NULL;
                    mp_raise_o(obj);
                }
            }

        }

        // ex check
        if (MP_STATE_THREAD(active_exception) != NULL) {
            clog("656: uncaught exception")
            //mp_hal_set_interrupt_char(-1);
            mp_handle_pending(false);
            //handle_uncaught_exception();
            if (uncaught_exception_handler()) {
                clog("689:SystemExit");
            } else {
                clog("691: exception done");
            }
        }
        RETVAL = exret ;
    }

    COME_FROM;
} // PyRun_SimpleString


#define VM_CANCEL_ACTIVE_FINALLY(sptr) do { \
    if (mp_obj_is_small_int(sptr[-1])) { \
        /* Stack: (..., prev_dest_ip, prev_cause, dest_ip) */ \
        /* Cancel the unwind through the previous finally, replace with current one */ \
        sptr[-2] = sptr[0]; \
        sptr -= 2; \
    } else { \
        assert(sptr[-1] == mp_const_none || mp_obj_is_exception_instance(sptr[-1])); \
        /* Stack: (..., None/exception, dest_ip) */ \
        /* Silence the finally's exception value (may be None or an exception) */ \
        sptr[-1] = sptr[0]; \
        --sptr; \
    } \
} while (0)


#if MICROPY_PY_SYS_SETTRACE

#define FRAME_SETUP() do { \
    assert(CTX.code_state != CTX.code_state->prev_state); \
    MP_STATE_THREAD(current_code_state) = CTX.code_state; \
    assert(CTX.code_state != CTX.code_state->prev_state); \
} while(0)

#define FRAME_ENTER() do { \
    assert(CTX.code_state != CTX.code_state->prev_state); \
    CTX.code_state->prev_state = MP_STATE_THREAD(current_code_state); \
    assert(CTX.code_state != CTX.code_state->prev_state); \
    if (!mp_prof_is_executing) { \
        mp_prof_frame_enter(CTX.code_state); \
    } \
} while(0)

#define FRAME_LEAVE() do { \
    assert(CTX.code_state != CTX.code_state->prev_state); \
    MP_STATE_THREAD(current_code_state) = CTX.code_state->prev_state; \
    assert(CTX.code_state != CTX.code_state->prev_state); \
} while(0)

#define FRAME_UPDATE() do { \
    assert(MP_STATE_THREAD(current_code_state) == CTX.code_state); \
    if (!mp_prof_is_executing) { \
        CTX.code_state->frame = MP_OBJ_TO_PTR(mp_prof_frame_update(CTX.code_state)); \
    } \
} while(0)

#define TRACE_TICK(current_ip, current_sp, is_exception) do { \
    assert(CTX.code_state != CTX.code_state->prev_state); \
    assert(MP_STATE_THREAD(current_code_state) == CTX.code_state); \
    if (!mp_prof_is_executing && CTX.code_state->frame && MP_STATE_THREAD(prof_trace_callback)) { \
        MP_PROF_INSTR_DEBUG_PRINT(code_state->ip); \
    } \
    if (!mp_prof_is_executing && CTX.code_state->frame && CTX.code_state->frame->callback) { \
        mp_prof_instr_tick(CTX.code_state, is_exception); \
    } \
} while(0)

#else // MICROPY_PY_SYS_SETTRACE
#define FRAME_SETUP()
#define FRAME_ENTER()
#define FRAME_LEAVE()
#define FRAME_UPDATE()
#define TRACE_TICK(current_ip, current_sp, is_exception)
#endif // MICROPY_PY_SYS_SETTRACE


def_mp_call_function_n_kw: {
    const mp_obj_type_t *type = mp_obj_get_type(CTX.self_in);
/*
#if VMTRACE
        if (ctx_current>3) {
            if (!CTX.code_state)
                clog("8433: WARNING code_state[%i] is NULL", ctx_current);
        }
#endif
*/

    if (type->call != NULL) {
        if ( (int)*type->call == (int)&fun_bc_call ) {
            ctx_get_next(CTX_COPY);
                GOSUB(def_func_bc_call, "mp_call_function_n_kw");
                RETVAL = SUBVAL; //CTX.sub_value;
        } else {
#if VMTRACE
clog("      899: native call");
#endif
            RETVAL = type->call(CTX.self_in, CTX.n_args, CTX.n_kw, CTX.args);
        }

    } else {
        clog("919:def_mp_call_function_n_kw ex!");
        mp_raise_o(
            mp_obj_new_exception_msg_varg(
                &mp_type_TypeError,"'%s' object isn't callable", mp_obj_get_type_str(CTX.self_in)
            )
        );
        RETVAL = MP_OBJ_NULL;
    }

    RETURN;
}


#define VM_DECODE_CODESTATE_SIZE(bytecode, n_state_out_var, state_size_out_var) \
    { \
        n_state_out_var = mp_decode_uint_value(bytecode);                         \
        size_t n_exc_stack = mp_decode_uint_value(mp_decode_uint_skip(bytecode)); \
                                                                                  \
        state_size_out_var = n_state_out_var * sizeof(mp_obj_t)                   \
                           + n_exc_stack * sizeof(mp_exc_stack_t);                \
    }




/* NOT OK

    when a function has tuple with default value init style the state_size calculated by
    VM_DECODE_CODESTATE_SIZE is just huge and wrong.

*/

//266:objfun.c
def_func_bc_call: {

    RETVAL = MP_OBJ_NULL;

    if (MP_STACK_CHECK()) {
        clog("974:def_func_bc_call: MP_STACK_CHECK ex!");
        goto def_func_bc_call_ret;
    }

    CTX.self_fun = MP_OBJ_TO_PTR(CTX.self_in);

    VM_DECODE_CODESTATE_SIZE(CTX.self_fun->bytecode, CTX.n_state, CTX.state_size);


    // allocate state for locals and stack
    // new frame == new code state.
    if ( CTX.state_size > 32768 ) {
        clog("957:BUG =======> start=%p cur=%p end=%p state_size=%ld",
        MP_STATE_THREAD(pystack_start), MP_STATE_THREAD(pystack_cur), MP_STATE_THREAD(pystack_end)
        , CTX.state_size);

#pragma message "Silly workaround for func with tuple init"
#if 0
        if (CTX.code_state = fun_bc_call_pre(CTX.self_in,  CTX.n_args, CTX.n_kw, CTX.args) ){
#if 1
            CTX.vm_return_kind = mp_execute_bytecode(CTX.code_state, MP_OBJ_NULL);
#else
            ctx_get_next(CTX_COPY);
                NEXT.inject_exc = MP_OBJ_NULL;
                NEXT.ip = NEXT.code_state->ip;
                NEXT.sp = NEXT.code_state->sp;
                GOSUB(def_mp_execute_bytecode,"func_bc_call");
                CTX.vm_return_kind = CTX.sub_vm_return_kind;
#endif
            RETVAL = fun_bc_call_past(CTX.code_state, CTX.vm_return_kind, CTX.self_in,  CTX.n_args, CTX.n_kw, CTX.args);
#endif
        RETVAL = fun_bc_call(CTX.self_in,  CTX.n_args, CTX.n_kw, CTX.args);
        goto def_func_bc_call_ret;
    }

    CTX.code_state = mp_pystack_alloc(sizeof(mp_code_state_t) + CTX.state_size);

    if (!CTX.code_state) {
        clog("974:def_func_bc_call: MP_PYSTACK_ALLOC ex!");
        goto def_func_bc_call_ret;
    }

    CTX.inject_exc =  MP_OBJ_NULL ;
    CTX.code_state->fun_bc = CTX.self_fun;
    CTX.code_state->ip = 0;
    CTX.code_state->n_state = CTX.n_state;

    clog("961:TODO can we save old_globals before this call ?");

    mp_obj_t ret = mp_setup_code_state(CTX.code_state, CTX.n_args, CTX.n_kw, CTX.args);

    //?
    CTX.code_state->old_globals = mp_globals_get();


    if ( ret == MP_OBJ_NULL) {
        clog("999:def_func_bc_call: INIT_CODESTATE ex!");
        mp_nonlocal_free(CTX.code_state, sizeof(mp_code_state_t));
        goto def_func_bc_call_ret;
    }

    ctx_get_next(CTX_COPY);

    // execute the byte code with the correct globals context
    mp_globals_set(NEXT.self_fun->globals);


    if (VMFLAGS_IF) { // FIXED !
        clog("132:unwrap.c ALLOWINT def_func_bc_call->def_mp_execute_bytecode");

        // ip sp would not be set on NEXT
        NEXT.ip = NEXT.code_state->ip;
        NEXT.sp = NEXT.code_state->sp;

if (MP_STATE_THREAD(active_exception) != NULL) clog("WTF %i", __LINE__);

            GOSUB(def_mp_execute_bytecode,"func_bc_call");
            CTX.vm_return_kind = CTX.sub_vm_return_kind;


    } else {
        clog("136:unwrap.c NOINTERRUPT");
        ctx_abort(); //128
        CTX.vm_return_kind = mp_execute_bytecode(CTX.code_state, CTX.inject_exc);
        if (CTX.vm_return_kind == MP_VM_RETURN_NORMAL) {
               CTX.sub_value= *CTX.code_state->sp;
        } else {
            if(CTX.vm_return_kind == MP_VM_RETURN_EXCEPTION)
                CTX.sub_value= CTX.code_state->state[0];
            else
                clog("1031:unwrap.c unrouted .vm_return_kind")
        }

    }

    mp_globals_set(CTX.code_state->old_globals);
    mp_pystack_free(CTX.code_state);

    #if MICROPY_DEBUG_VM_STACK_OVERFLOW
    #error "[...]"
    #endif

//353:objfun.c
        // work in done juste before def_mp_execute_bytecode RETURN
    if (CTX.vm_return_kind == MP_VM_RETURN_NORMAL) {
           RETVAL = CTX.sub_value;
    } else {
        if(CTX.vm_return_kind == MP_VM_RETURN_EXCEPTION)
            RETVAL = mp_raise_o(CTX.sub_value);
        else
            clog("1050:unwrap.c unrouted .sub_vm_return_kind")
    }


def_func_bc_call_ret:
    RETURN;
}


#include "vmsl/unwrap.c"


//==================================================================
// VM_syscall_verbose:;
//    puts("-syscall-");

VM_syscall:;
// TODO: flush all at once
    // STDOUT flush before eventually filling it again
    if (!rbb_is_empty(&out_rbb)) {
        // flush stdout
        unsigned char out_c = 0;
        printf("{\"%c\":\"",49);
        //TODO put a 0 at end and printf buffer directly
        while (rbb_pop(&out_rbb, &out_c))
            printf("%c", out_c );
        printf("\"}\n");
    }
#if 1 //USE_SDL
    //if (renderer)
      //  SDL_RenderPresent(renderer);
#endif
}





//***************************************************************************************





int PyArg_ParseTuple(PyObject *argv, const char *fmt, ...) {
    va_list argptr;
    va_start (argptr, fmt );
    vfprintf(stdout,fmt,argptr);
    va_end (argptr);
    return 0;
}





int
main(int argc, char *argv[]) {

    g_argc = argc;
    g_argv = copy_argv(argc, argv);


    emscripten_GetProcAddress("pouet");


#if !ASYNCIFY
    emscripten_set_main_loop( main_loop_or_step, 0, 1);  // <= this will exit to js now.
#else
    while (!KPANIC) {
        emscripten_sleep(1);
        main_loop_or_step();
    }
#endif
    puts("no guru meditation");
    return 0;
}




























//



//
// core/main.c
// core/vfs.c
//
#define HEAP_SIZE 128 * 1024 * 1024
#define MP_IO_SHM_SIZE 32768

#define MP_IO_SIZE 1024
#define MP_IO_ADDR (MP_IO_SHM_SIZE - MP_IO_SIZE)


#define IO_KBD 0
#define IO_KBD_SIZE MP_IO_SIZE

#if MICROPY_ENABLE_PYSTACK
    #define MP_STACK_SIZE 16384
    static mp_obj_t pystack[MP_STACK_SIZE];
#endif

static char *stack_top;

#include "../wapy/upython.h"

#include "../wapy/core/ringbuf_o.h"
#include "../wapy/core/ringbuf_b.h"

//static
struct wPyInterpreterState i_main ;
//static
struct wPyThreadState i_state ;

RBB_T( out_rbb, 4096);



EMSCRIPTEN_KEEPALIVE int
show_os_loop(int state) {
    int last = SHOW_OS_LOOP;
    if (state>=0) {
        SHOW_OS_LOOP = state;
        if (state>0) {
            //fprintf(stdout,"------------- showing os loop --------------\n");
            fprintf(stdout,"------------- showing os loop / starting repl --------------\n");
            repl_started = 1;
        } else {
            if (last!=state)
                fprintf(stdout,"------------- hiding os loop --------------\n");
        }
    }
    return (last>0);
}

EMSCRIPTEN_KEEPALIVE int
state_os_loop(int state) {
    static int last_state = -666;

    if (!state) {
        last_state = SHOW_OS_LOOP;
        SHOW_OS_LOOP = 0;
    } else {
        if (last_state != -666)
            SHOW_OS_LOOP = last_state;
        last_state = -666;
    }
    return last_state;
}

// ----


// should check null
char*
shm_ptr() {
    return &i_main.shm_stdio[0];
}

char*
shm_get_ptr(int major,int minor) {
    // keyboards
    if (major==IO_KBD) {
        if (minor==0)
            return &i_main.shm_stdio[MP_IO_ADDR];
    }
    return NULL;
}



char*
wPy_NewInterpreter() {
    i_main.shm_stdio = (char *)malloc(MP_IO_SHM_SIZE);
    if (!i_main.shm_stdio)
        fprintf(stdout, "74:shm_stdio malloc failed\n");
    i_main.shm_stdio[0] = 0 ;
    for (int i=0;i<MP_IO_SHM_SIZE;i++)
        i_main.shm_stdio[i]=0;
    i_state.interp = & i_main;
    pyexec_event_repl_init();
    return shm_ptr();
}

void
wPy_Initialize() {
    int stack_dummy;
    stack_top = (char*)&stack_dummy;

#if MICROPY_ENABLE_GC
    char *heap = (char*)malloc(HEAP_SIZE * sizeof(char));
    gc_init(heap, heap + HEAP_SIZE);
#endif

//#if MICROPY_ENABLE_PYSTACK
    mp_pystack_init(pystack, &pystack[MP_ARRAY_SIZE(pystack)]);
//#endif

    mp_init();

    #if MICROPY_EMIT_NATIVE
    // Set default emitter options
    MP_STATE_VM(default_emit_opt) = emit_opt;
    #else
#pragma message "          ------------- (void)emit_opt; ?????????????????????"
    //(void)emit_opt;
    #endif


    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));
    mp_obj_list_init(mp_sys_argv, 0);
}


void
gc_collect(void) {
    void *dummy;
    gc_collect_start();
    clog("122: noop: gc_collect\n");
    gc_collect_root((void*)stack_top, ((mp_uint_t)(void*)(&dummy + 1) - (mp_uint_t)stack_top) / sizeof(mp_uint_t));
    gc_collect_end();
}

int
do_str(const char *src, mp_parse_input_kind_t input_kind) {
    mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), False);

    if (lex == NULL) {
        fprintf(stdout,"148:NULL LEXER->handle_uncaught_exception\nn%s\n", src);
        return 0;
    }
    {
        #if MICROPY_ENABLE_COMPILER
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, false);
        if (module_fun != MP_OBJ_NULL) {
            mp_obj_t ret = mp_call_function_0(module_fun);
            if (ret != MP_OBJ_NULL) {
                if (MP_STATE_VM(mp_pending_exception) != MP_OBJ_NULL) {
                    mp_obj_t obj = MP_STATE_VM(mp_pending_exception);
                    MP_STATE_VM(mp_pending_exception) = MP_OBJ_NULL;
                    mp_raise_o(obj);

                } else  {
                    return 1; //success
                }
            }

        } else {
            // uncaught exception
            fprintf(stdout,"150:NULL FUNCTION\n");
        }
        #else
            #pragma message "compiler is disabled, no repl"
        #endif
    }
    return 0;
}

#include "wasm_mphal.c"

int PyRun_IO_CODE() {
    return do_str(i_main.shm_stdio, MP_PARSE_FILE_INPUT);
}

int
PyRun_SimpleString(const char* command) {
    strcpy( i_main.shm_stdio , command);
    int retval = do_str(i_main.shm_stdio, MP_PARSE_FILE_INPUT);
    i_main.shm_stdio[0] = 0;
    return retval;
}



EMSCRIPTEN_KEEPALIVE int
repl_run(int warmup) {
    if (warmup==1)
        return MP_IO_SHM_SIZE;
    //wPy_NewInterpreter();
    repl_started = MP_IO_SHM_SIZE;
    return 1;
}




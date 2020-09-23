#include <stdlib.h>

#include "py/objlist.h"
#include "py/runtime.h"

#define HEAP_SIZE 64 * 1024 * 1024

// TODO: use a circular buffer for everything io related
#define MP_IO_SHM_SIZE 65535

#define MP_IO_SIZE 512

#define IO_KBD ( MP_IO_SHM_SIZE - (1 * MP_IO_SIZE) )

#define False 0
#define True !False

#if MICROPY_ENABLE_PYSTACK
//#define MP_STACK_SIZE 16384
#define MP_STACK_SIZE 32768
static mp_obj_t pystack[MP_STACK_SIZE];
#endif

static char *stack_top;

/*
extern void gc_init(void *start, void *end);
extern void mp_pystack_init(void *start, void *end);
extern void mp_init(void);
*/


void
Py_Initialize(){
    int stack_dummy;
    stack_top = (char *) &stack_dummy;
    char *heap = (char *) malloc(HEAP_SIZE * sizeof(char));
    gc_init(heap, heap + HEAP_SIZE);
    mp_pystack_init(pystack, &pystack[MP_ARRAY_SIZE(pystack)]);
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));
    mp_obj_list_init(mp_sys_argv, 0);
}



struct
wPyInterpreterState {
    char *shm_stdio ;
    char *shm_input_event_0;
};


struct
wPyThreadState {
    struct wPyInterpreterState *interp;
};


//static
struct wPyInterpreterState i_main;
//static
struct wPyThreadState i_state;

// should check null
char *
shm_ptr() {
    return &i_main.shm_stdio[0];
}


char *
wPy_NewInterpreter() {
    i_main.shm_stdio = (char *) malloc(MP_IO_SHM_SIZE);
    if (!i_main.shm_stdio)
        fprintf(stdout, "74:shm_stdio malloc failed\n");
    i_main.shm_stdio[0] = 0;
    for (int i = 0; i < MP_IO_SHM_SIZE; i++)
        i_main.shm_stdio[i] = 0;
    i_state.interp = &i_main;
    #if MICROPY_REPL_EVENT_DRIVEN
        pyexec_event_repl_init();
    #else
        fprintf(stdout,"no repl\n");
    #endif
    return shm_ptr();
}







#include "py/compile.h"

int
pyeval(const char *src, mp_parse_input_kind_t input_kind) {
    mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), False);

    if (lex == NULL) {
        fprintf(stderr, "148:NULL LEXER->handle_uncaught_exception\n%s\n", src);
        return 0;
    }
    {
#if MICROPY_ENABLE_COMPILER
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, False);
        if (module_fun != MP_OBJ_NULL) {
            mp_obj_t ret = mp_call_function_0(module_fun);
            if (ret != MP_OBJ_NULL) {
                if (MP_STATE_VM(mp_pending_exception) != MP_OBJ_NULL) {
                    mp_obj_t obj = MP_STATE_VM(mp_pending_exception);
                    MP_STATE_VM(mp_pending_exception) = MP_OBJ_NULL;
                    //mp_raise(obj);

                } else {
                    return 1;   //success
                }
            }

        } else {
            // uncaught exception
            fprintf(stderr, "150:NULL FUNCTION\n");
        }
#else
    #pragma message "compiler is disabled, no repl"
#endif
    }
    return 0;
}

int
PyRun_SimpleString(const char *command) {
    int retval = 0;
    if (command) {
        retval = pyeval(command, MP_PARSE_FILE_INPUT);
    } else {
        if (i_main.shm_stdio[0]) {
            retval = pyeval(i_main.shm_stdio, MP_PARSE_FILE_INPUT);
            i_main.shm_stdio[0] = 0;
        }
    }
    return retval;
}


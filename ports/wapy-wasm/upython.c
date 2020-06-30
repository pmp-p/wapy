//
// core/main.c
// core/vfs.c

#define HEAP_SIZE 64 * 1024 * 1024

// TODO: use a circular buffer for everything io related
#define MP_IO_SHM_SIZE 65535

#define MP_IO_SIZE 512

#define IO_KBD ( MP_IO_SHM_SIZE - (1 * MP_IO_SIZE) )


#if MICROPY_ENABLE_PYSTACK
//#define MP_STACK_SIZE 16384
#define MP_STACK_SIZE 32768
static mp_obj_t pystack[MP_STACK_SIZE];
#endif

static char *stack_top;

#include "../wapy/upython.h"

#include "../wapy/core/ringbuf_o.h"
#include "../wapy/core/ringbuf_b.h"

//static
struct wPyInterpreterState i_main;
//static
struct wPyThreadState i_state;

RBB_T(out_rbb, 16384);



EMSCRIPTEN_KEEPALIVE int
show_os_loop(int state) {
    int last = SHOW_OS_LOOP;
    if (state >= 0) {
        SHOW_OS_LOOP = state;
        if (state > 0) {
            //fprintf(stdout,"------------- showing os loop --------------\n");
            fprintf(stdout, "------------- showing os loop / starting repl --------------\n");
            repl_started = 1;
        } else {
            if (last != state)
                fprintf(stdout, "------------- hiding os loop --------------\n");
        }
    }
    return (last > 0);
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
char *
shm_ptr() {
    return &i_main.shm_stdio[0];
}

char *
shm_get_ptr(int major, int minor) {
    // keyboards
    if (major == IO_KBD) {
        if (minor == 0)
            return &i_main.shm_stdio[IO_KBD];
    }
    return NULL;
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
    pyexec_event_repl_init();
    return shm_ptr();
}

void
wPy_Initialize() {
    int stack_dummy;
    stack_top = (char *) &stack_dummy;

#if MICROPY_ENABLE_GC
    char *heap = (char *) malloc(HEAP_SIZE * sizeof(char));
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

// TODO py/stackctrl.c#L40  check stack way


#if !MICROPY_ENABLE_FINALISER
#error requires MICROPY_ENABLE_FINALISER (1)
#endif


#undef gc_collect

#if  0                          // TODO remove those gc bits was for experimenting


#if MICROPY_PY_THREAD && !MICROPY_PY_THREAD_GIL
#define GC_ENTER() mp_thread_mutex_lock(&MP_STATE_MEM(gc_mutex), 1)
#define GC_EXIT() mp_thread_mutex_unlock(&MP_STATE_MEM(gc_mutex))
#else
#define GC_ENTER()
#define GC_EXIT()
#endif

#define TRACE_MARK(block, ptr, i, total) clog("gc_mark(%p) %lu/%i", ptr, i, total)

#define AT_HEAD (1)
#define AT_TAIL (2)
#define AT_MARK (3)

// ptr should be of type void*
#define VERIFY_PTR(ptr) ( \
    ((uintptr_t)(ptr) & (MICROPY_BYTES_PER_GC_BLOCK - 1)) == 0          /* must be aligned on a block */ \
    && ptr >= (void *)MP_STATE_MEM(gc_pool_start)        /* must be above start of pool */ \
    && ptr < (void *)MP_STATE_MEM(gc_pool_end)           /* must be below end of pool */ \
    )


#define BLOCKS_PER_ATB (4)

#define BLOCK_SHIFT(block) (2 * ((block) & (BLOCKS_PER_ATB - 1)))

#define BLOCK_FROM_PTR(ptr) (((byte *)(ptr) - MP_STATE_MEM(gc_pool_start)) / MICROPY_BYTES_PER_GC_BLOCK)
#define PTR_FROM_BLOCK(block) (((block) * MICROPY_BYTES_PER_GC_BLOCK + (uintptr_t)MP_STATE_MEM(gc_pool_start)))

#define ATB_GET_KIND(block) ((MP_STATE_MEM(gc_alloc_table_start)[(block) / BLOCKS_PER_ATB] >> BLOCK_SHIFT(block)) & 3)
#define ATB_HEAD_TO_MARK(block) do { \
 MP_STATE_MEM(gc_alloc_table_start)[(block) / BLOCKS_PER_ATB] |= (AT_MARK << BLOCK_SHIFT(block));\
} while (0)

// Take the given block as the topmost block on the stack. Check all it's
// children: mark the unmarked child blocks and put those newly marked
// blocks on the stack. When all children have been checked, pop off the
// topmost block on the stack and repeat with that one.

//extern void gc_mark_subtree(size_t block);

STATIC void
gc_mark_subtree1(size_t block) {
    // Start with the block passed in the argument.
    size_t sp = 0;
    for (;;) {
        // work out number of consecutive blocks in the chain starting with this one
        size_t n_blocks = 0;
        do {
            n_blocks += 1;
        } while (ATB_GET_KIND(block + n_blocks) == AT_TAIL);

        // check this block's children
        void **ptrs = (void **) PTR_FROM_BLOCK(block);
        for (size_t i = n_blocks * MICROPY_BYTES_PER_GC_BLOCK / sizeof(void *); i > 0; i--, ptrs++) {
            void *ptr = *ptrs;
            if (VERIFY_PTR(ptr)) {
                // Mark and push this pointer
                size_t childblock = BLOCK_FROM_PTR(ptr);
                if (ATB_GET_KIND(childblock) == AT_HEAD) {
                    // an unmarked head, mark it, and push it on gc stack
                    //TRACE_MARK(childblock, ptr);
                    ATB_HEAD_TO_MARK(childblock);
                    if (sp < MICROPY_ALLOC_GC_STACK_SIZE) {
                        MP_STATE_MEM(gc_stack)[sp++] = childblock;
                    } else {
                        MP_STATE_MEM(gc_stack_overflow) = 1;
                    }
                }
            }
        }

        // Are there any blocks on the stack?
        if (sp == 0) {
            break;              // No, stack is empty, we're done.
        }
        // pop the next block off the stack
        block = MP_STATE_MEM(gc_stack)[--sp];
    }
}

void
gc_collect_root1(void **ptrs, size_t len) {
    for (size_t i = 0; i < len; i++) {
        void *ptr = ptrs[i];
        if (VERIFY_PTR(ptr)) {
            size_t block = BLOCK_FROM_PTR(ptr);
            if (ATB_GET_KIND(block) == AT_HEAD) {
                // An unmarked head: mark it, and mark all its children
                //TRACE_MARK(block, ptr, i, len);
                ATB_HEAD_TO_MARK(block);
                gc_mark_subtree1(block);
            }
        }
    }
}

void
gc_collect_start1(void) {
    GC_ENTER();
    MP_STATE_MEM(gc_lock_depth)++;
#if MICROPY_GC_ALLOC_THRESHOLD
    MP_STATE_MEM(gc_alloc_amount) = 0;
#endif
    MP_STATE_MEM(gc_stack_overflow) = 0;

    // Trace root pointers.  This relies on the root pointers being organised
    // correctly in the mp_state_ctx structure.  We scan nlr_top, dict_locals,
    // dict_globals, then the root pointer section of mp_state_vm.
    void **ptrs = (void **) (void *) &mp_state_ctx;
    size_t root_start = offsetof(mp_state_ctx_t, thread.dict_locals);
    size_t root_end = offsetof(mp_state_ctx_t, vm.qstr_last_chunk);
    gc_collect_root(ptrs + root_start / sizeof(void *), (root_end - root_start) / sizeof(void *));

#if MICROPY_ENABLE_PYSTACK
    // Trace root pointers from the Python stack.
    ptrs = (void **) (void *) MP_STATE_THREAD(pystack_start);
    gc_collect_root(ptrs, (MP_STATE_THREAD(pystack_cur) - MP_STATE_THREAD(pystack_start)) / sizeof(void *));
#endif
}


#else


//extern void gc_collect_root(void **ptrs, size_t len);

#include "py/gc.h"

#undef gc_collect


// GC boundaries


// The address of the top of the stack when we first saw it
// This approximates a pointer to the bottom of the stack, but
// may not necessarily _be_ the exact bottom. This is set by
// the entry point of the application
static void *stack_initial = NULL;
// Pointer to the end of the stack
static uintptr_t stack_max = (uintptr_t) NULL;
// The current stack pointer
static uintptr_t stack_ptr_val = (uintptr_t) NULL;
// The amount of stack remaining
static ptrdiff_t stack_left = 0;

// Maximum stack size of 248k
// This limit is arbitrary, but is what works for me under Node.js when compiled with emscripten
static size_t stack_limit = 1024 * 248 * 1;


void
gc_collect(void) {

    gc_dump_info();

    stack_ptr_val = (uintptr_t) __builtin_frame_address(0);

    clog("gc_collect_start");

    gc_collect_start();

    size_t bottom = (uintptr_t) stack_ptr_val;

    void **ptrs = (void **) (void *) stack_ptr_val;

    size_t len = ((uintptr_t) stack_initial - bottom) / sizeof(uintptr_t);

    clog("gc_collect stack_initial=%p bottom=%zu len=%zu", stack_initial, bottom, len);

//343

    gc_collect_root(ptrs, len);
//343

#if MICROPY_PY_THREAD
    mp_thread_gc_others();
#endif
#if MICROPY_EMIT_NATIVE
    mp_unix_mark_exec();
#endif

    clog("gc_collect_end");
    gc_collect_end();
}

#endif // gc

int
pyeval(const char *src, mp_parse_input_kind_t input_kind) {
    mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), False);

    if (lex == NULL) {
        fprintf(stdout, "148:NULL LEXER->handle_uncaught_exception\n%s\n", src);
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

                } else {
                    return 1;   //success
                }
            }

        } else {
            // uncaught exception
            fprintf(stdout, "150:NULL FUNCTION\n");
        }
#else
    #pragma message "compiler is disabled, no repl"
#endif
    }
    return 0;
}

#include "wasm_mphal.c"

int
PyRun_IO_CODE() {
    return pyeval(i_main.shm_stdio, MP_PARSE_FILE_INPUT);
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


EMSCRIPTEN_KEEPALIVE int
repl_run(int warmup) {
    if (warmup == 1)
        return MP_IO_SHM_SIZE;
    //wPy_NewInterpreter();
    repl_started = MP_IO_SHM_SIZE;
    return 1;
}

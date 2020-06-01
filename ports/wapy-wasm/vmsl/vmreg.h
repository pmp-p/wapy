#include "py/obj.h"
#include "py/runtime.h"

//#include "lib/bipbuffer/bipbuffer.h"
//#include "lib/bipbuffer/bipbuffer.c"


typedef struct _mp_obj_closure_t {
    mp_obj_base_t base;
    mp_obj_t fun;
    size_t n_closed;
    mp_obj_t closed[];
} mp_obj_closure_t;


typedef struct _mp_obj_gen_instance_t {
    mp_obj_base_t base;
    mp_obj_dict_t *globals;
    mp_code_state_t code_state;
} mp_obj_gen_instance_t;

#include "vmsl/vmconf.h"

struct mp_registers {
    // builtins override dict  ?
    // __main__ ?
    // sys.(std*) ?
    // sys.argv

    // cpu time load stats ?

    //who created us
    int parent ;

    //
    int sub_id ;

    //who did we create and was running last
    int childcare ;

    int vmloop_state;
    nlr_buf_t nlr;

    mp_lexer_t *lex;
    mp_reader_t reader;

    qstr source_name;
    mp_parse_tree_t parse_tree;

    mp_obj_t * /*const*/ fastn;
    mp_exc_stack_t * /*const*/ exc_stack;

    size_t n_state;
    size_t state_size;

//bytecode switch case

    volatile mp_obj_t inject_exc;
    mp_code_state_t *code_state;
    mp_exc_stack_t *volatile exc_sp;


// execution path
    int pointer;
    int switch_break_for;


// parent return state
    size_t ulab;
    size_t slab;
    mp_obj_t *sp;
    const byte *ip;
    mp_obj_t obj_shared;
    void *ptr;

// mp_import state
    qstr qst;
    mp_obj_t argv[5];


//  mp_call_function_n_kw, closure_call and iterators state
    mp_obj_t self_in;
    mp_obj_fun_builtin_var_t *self_fb;
    mp_obj_fun_bc_t *self_fun;
    mp_obj_closure_t *self_clo;

    int alloc;

    //mp_obj_type_t *type;

    size_t n_args;
    size_t n_kw;
    //const
    mp_obj_t *args;

    mp_obj_gen_instance_t* generator;
    mp_obj_t send_value, throw_value, return_value;

    mp_obj_t sub_value;  // child ctx computed value.
    mp_obj_t *sub_args ; // child allocated memory ptr
    int sub_alloc ; // child allocated memory size
    mp_vm_return_kind_t sub_vm_return_kind; // child result on mp_execute_bytecode() calls ( can be recursive )

    mp_vm_return_kind_t vm_return_kind;
    // last
    //mp_obj_gen_instance_t self_gen;
};

static struct mp_registers mpi_ctx[SYS_MAX_RECURSION];

void
mp_new_interpreter(void * mpi, int ctx, int parent, int childcare) {
    mpi_ctx[ctx].vmloop_state = VM_IDLE;
    mpi_ctx[ctx].parent = parent;
    mpi_ctx[ctx].childcare = childcare;
    mpi_ctx[ctx].pointer = 0;
    mpi_ctx[ctx].code_state = NULL ;
    //?
    mpi_ctx[ctx].state_size = 0;
    mpi_ctx[ctx].n_state = 0;
    //?
    mpi_ctx[ctx].n_args = 0;
    mpi_ctx[ctx].n_kw = 0;
    mpi_ctx[ctx].args = &mpi_ctx[ctx].argv[0];
    mpi_ctx[ctx].alloc = 0;
    mpi_ctx[ctx].switch_break_for = 0;
    mpi_ctx[ctx].sub_id = sub_tracking++;

}

#define VM_FULL 1


struct mp_stack {
    int gen_i;
    int gen_n;
    int has_default;
    int default_value;
    int __line__ ;
    const char *name;
    const char *value;  // some in memory rpc struct
};


// Duff's device thanks a lot to https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html


#define Begin switch(generator_context->__line__) { case 0:

#define yield(x)\
do {\
    generator_context->__line__ = __LINE__;\
    return x; \
case __LINE__:; } while (0)

#define End };


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// static int enumerator=0;
#define vars(generator_name) static struct mp_stack generator_context = { .gen_i=0, .gen_n = 1, .has_default = 0, .default_value = 666, .__line__ = 0, .name=TOSTRING(generator_name) }

#define async_def(generator_name, gen_type, ...) \
    gen_type generator_name(struct mp_stack *generator_context) { \
        Begin; \
        __VA_ARGS__ \
        End;\
        generator_context->gen_n = 0;\
        return generator_context->default_value;\
    }\

#define async_iter(generator_name) \
    vars(generator_name);\
    if (generator_context.gen_n) generator_context.gen_i=generator_name(&generator_context); \
    if (generator_context.gen_n)

#define async_next(generator_name, defval) \
    vars(generator_name); if (!generator_context.has_default) { generator_context.has_default = 1; generator_context.default_value = defval; }\
    int had_next = generator_context.gen_n; \
    if (had_next) generator_context.gen_i = generator_name(&generator_context); \
    if (had_next)


// -------------------------- attempt to be pythonic --------------------
#define self (*generator_context)
#define iterator generator_context->gen_i
#define next generator_context->gen_n


async_def(gen1, int, {
    while (next) {
        printf("gen1 %s ",self.name);
        if (iterator++ == 10) break;
        yield(iterator);
    }
});


async_def(gen2, int, {
    for (iterator = 0; iterator < 5; iterator++) {
        printf("gen2 %s ",self.name);
        yield(iterator);
    }
});

#undef self
#undef iterator
#undef next

#define iterator generator_context.gen_i

// ------------------------------------------------------------------------


#define VM_REG_H 1

#define STRINGIFY(a) #a
#define TOSTRING(x) STRINGIFY(x)

#define STRINGIFY2(a, b) a ## b
#define CONCAT(x, y) STRINGIFY2(x, y)



#ifndef VM_REG_H
    //TODO sys max recursion handling.
    #define SYS_MAX_RECURSION 32
    #define MAX_BRANCHING 128

    //order matters
    #define VM_IDLE     0
    #define VM_WARMUP   1
    #define VM_RUNNING   2
    #define VM_RESUMING  3
    #define VM_PAUSED    4
    #define VM_SYSCALL    5
    #define VM_HCF  6

    static int VMOP = -1;

    #define VMOP_NONE       0
    #define VMOP_INIT       1
    #define VMOP_WARMUP     2
    #define VMOP_CALL       3
    #define VMOP_CRASH      4
    #define VMOP_PAUSE      5
    #define VMOP_SYSCALL    6

    static int sub_tracking = 0;

    static int ctx_current = 1;
    static int ctx_next = 2;



    #define clog(...) { fprintf(stderr, __VA_ARGS__ );fprintf(stderr, "\n"); }

    struct mp_registers {
        int sub_id ;
        //who created us
        int parent ;
        //who did we create and was running last
        int childcare ;

        // execution path
        int pointer;
        int switch_break_for;
        int vmloop_state;
    };

    static struct mp_registers mpi_ctx[SYS_MAX_RECURSION];

    void mp_new_interpreter(void * mpi, int ctx, int parent, int childcare) {

        mpi_ctx[ctx].parent = parent;
        mpi_ctx[ctx].childcare = childcare;
    //
        mpi_ctx[ctx].pointer = 0;
        mpi_ctx[ctx].sub_id = sub_tracking++;
        mpi_ctx[ctx].vmloop_state = VM_IDLE;

    }
#endif


#define JMP_NONE (void*)0
#define TYPE_JUMP 0
#define TYPE_SUB 1

#define JUMPED_IN reached_point[CTX.pointer]
#define JUMP_TYPE type_point[CTX.pointer]
#define ENTRY_POINT entry_point[CTX.pointer]
#define EXIT_POINT exit_point[CTX.pointer]

static int point_ptr = 0;  // index 0 is reserved for no branching

static int come_from[MAX_BRANCHING];
static int type_point[MAX_BRANCHING];
static int reached_point[MAX_BRANCHING];
static void* entry_point[MAX_BRANCHING];
static void* exit_point[MAX_BRANCHING];


static void* crash_point = JMP_NONE;

//need global interrupt state marker ( @syscall / @awaited  ) to choose which VM to enter
// this is different from ctx interrupts ctx_if ( if used )

int VMFLAGS_IF = 0;

static struct mp_registers mpi_ctx[SYS_MAX_RECURSION];

#define CTX  mpi_ctx[ctx_current]
#define NEXT mpi_ctx[ctx_next]
#define PARENT mpi_ctx[CTX.parent]

#define CTX_STATE CTX.vmloop_state
#define NEXT_STATE NEXT.vmloop_state

void *crash(const char *panic){
    clog("%s", panic);
    VMOP = VMOP_CRASH;
    return crash_point;

}

#define FATAL(msg) goto *crash(msg)

#if VM_FULL
    #define SUBVAL CTX.sub_value
    #define RETVAL CTX.return_value
#endif

#define RETURN goto *ctx_return()
#define COME_FROM goto *ctx_come_from()



void ctx_free(){
    clog("CTX %d freed(free) for %d", ctx_current, CTX.parent);
    CTX_STATE = VM_IDLE;
}

void ctx_abort(){
    clog("CTX %d freed(abort) for %d", ctx_next, ctx_current);
    NEXT_STATE = VM_IDLE;
}


static int come_from[MAX_BRANCHING];

#define deferred 1

void zigzag(void* jump_entry, void* jump_back, int jump_type){
    come_from[++point_ptr] = CTX.pointer; // when set to 0 its origin mark.
    CTX.pointer = point_ptr;
    JUMPED_IN = 0;
    ENTRY_POINT = jump_entry;
    EXIT_POINT = jump_back;
    JUMP_TYPE = jump_type;
}




#define CTX_COPY 1
#define CTX_NEW 0

// prepare a child context for a possibly recursive call,
// NEXT.*  will then gives access to that ctx (inited from CTX if CTX_COPY ) until GOSUB()
// after GOSUB CTX is restored automaticallly to parent and return value is in CTX.sub_value

#if VM_FULL
void ctx_get_next(int copy) {
    //push
    int ctx;
    for (ctx=3; ctx<SYS_MAX_RECURSION; ctx++)
        if (mpi_ctx[ctx].vmloop_state == VM_IDLE){
            // should default to syscall ? VM_PAUSED
            mpi_ctx[ctx].vmloop_state = VM_WARMUP ;
            break;
        }
    if (ctx==SYS_MAX_RECURSION)
        fprintf(stderr,"FATAL: SYS_MAX_RECURSION reached");
    #if CTX_DEBUG
    else
        clog("CTX reservation %d",ctx);
    #endif
    //track
    CTX.childcare = ctx ;
    mp_new_interpreter(&mpi_ctx, ctx, ctx_current, 0);
    ctx_next = ctx;

    if (copy) {
        if ( (ctx_current>2) && !CTX.code_state)
            clog(" ======== no code_state for slot %i->%i ============", ctx_current, ctx_next);

        NEXT.self_in = CTX.self_in;
        NEXT.self_fun = CTX.self_fun;
        NEXT.ip = CTX.ip;
        NEXT.sp = CTX.sp;
        NEXT.code_state = CTX.code_state ;
        NEXT.exc_stack = CTX.exc_stack;
        NEXT.n_args = CTX.n_args;
        NEXT.n_kw = CTX.n_kw;
        NEXT.args = CTX.args;

//?
        NEXT.n_state = CTX.n_state;
        NEXT.state_size = CTX.state_size;
        NEXT.inject_exc = CTX.inject_exc;
    }
}
#endif


void ctx_switch(){
    NEXT_STATE  = VM_RUNNING;
    ctx_current = ctx_next;
    #if CTX_DEBUG
    clog("CTX %d locked for %d",ctx_current,CTX.parent);
    #endif
}

void* ctx_branch(void* jump_entry,int vmop, void *jump_back, const char *jto, const char *jback, const char* context, int defer) {
    VMOP = vmop;
    zigzag(jump_entry, jump_back, TYPE_JUMP);
    clog("    ZZ > %s(...) %s -> %s  @%d",context, jto, jback, ctx_current);
    if (!defer)
        JUMPED_IN = 1 ;
    return jump_entry;
}

#define BRANCH(arg0, vmop, arg1, arg2) goto *ctx_branch(&&arg0, vmop, &&arg1, TOSTRING(arg0), TOSTRING(arg1), arg2, !deferred)

#define SYSCALL(arg0, vmop, arg1, arg2) \
{\
 ctx_branch(&&arg0, vmop, &&arg1, TOSTRING(arg0), TOSTRING(arg1), arg2, deferred);\
 goto VM_syscall;\
}


void* ctx_call(void* jump_entry, void *jump_back, const char *jt_origin,const char *context, int defer) {
    VMOP = VMOP_CALL;
    zigzag(jump_entry, jump_back, TYPE_JUMP);
    clog("    CC > %s->%s(...) @%d", context, jt_origin, ctx_current);
    if (!defer)
        JUMPED_IN = 1 ;
    return jump_entry;
}

#define CALL(arg0, caller) \
{\
 ctx_call(&&arg0, &&CONCAT(call_,__LINE__), TOSTRING(arg0), caller, deferred);\
 goto VM_syscall;\
 CONCAT(call_,__LINE__):;\
}

#define JUMP(arg0, caller) \
{\
 goto *ctx_call(&&arg0, &&CONCAT(call_,__LINE__), TOSTRING(arg0), caller, !deferred);\
 CONCAT(call_,__LINE__):;\
}

void* ctx_sub(void* jump_entry, void* jump_back, const char* jto, const char* jback, const char* context) {
    // set the new context so zigzag @ are set
    ctx_switch();

    zigzag(jump_entry, jump_back, TYPE_SUB);
    clog("    Begin[%d:%d] %s(...) %s -> %s  %d->%d", ctx_current, CTX.sub_id, context, jto, jback,  CTX.parent, ctx_current);
    JUMPED_IN = 1 ;
    return jump_entry;
}

//#define GOSUB(arg0, arg1, arg2) goto *ctx_sub(&&arg0, &&arg1, TOSTRING(arg0), TOSTRING(arg1), arg2 )

#define GOSUB(arg0, caller) \
{\
 goto *ctx_sub(&&arg0, &&CONCAT(call_,__LINE__), TOSTRING(arg0), TOSTRING(CONCAT(call_,__LINE__)), caller ) ;\
 CONCAT(call_,__LINE__):;\
}


void* ctx_come_from() {
    // just going back from branching
    int ptr_back = come_from[CTX.pointer];

    void* return_point;

    if (JUMP_TYPE!=TYPE_JUMP)
        return crash("BRANCH EXIT in SUB CONTEXT");

    return_point = EXIT_POINT;

    if (point_ptr!=CTX.pointer) {
        return crash("ERROR: jumping back from upper branch not allowed, maybe ctx_get_next missing for GOSUB ?");
    } else
        point_ptr--;
    clog("<<< ZZ[%d]",ctx_current);
    // go up one level of branching ( or if 0 reach root )
    CTX.pointer = ptr_back;
    return return_point;
}



void* ctx_return(){
    void* return_point;
    int ptr_back = come_from[CTX.pointer];

#if VM_FULL
    PARENT.sub_vm_return_kind = CTX.vm_return_kind;
    PARENT.sub_value = CTX.return_value;
    PARENT.sub_alloc = CTX.alloc;
    PARENT.sub_args  = CTX.args ;
#endif

    // possibly return in upper branch ?
    if (point_ptr!=CTX.pointer) {
        clog("ERROR not on leaf branch");
        emscripten_cancel_main_loop();
    } else {
        point_ptr -= 1;
    }
    return_point = exit_point[CTX.pointer];


    // not a zigzag branching free registers
    if (!ptr_back)
        ctx_free();

    // not a zigzag go upper context
    if ( JUMP_TYPE == TYPE_SUB)
        ctx_current = CTX.parent;
    else
        return crash("239:ERROR: RETURN in non SUB BRANCH");

    return return_point;
}




// #define FUN_NAME "FUN_NAME"




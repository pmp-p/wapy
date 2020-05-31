#define VMTRACE (1)


#if !MICROPY_ENABLE_PYSTACK
    #error "need MICROPY_ENABLE_PYSTACK (1)"
#endif

#if !MICROPY_PERSISTENT_CODE
    #pragma message "VM_DECODE_QSTR/VM_DECODE_PTR/VM_DECODE_OBJ"
#endif


#if VMTRACE
    #define VM_TRACE_QSTR(opc, opv) clog("      %i:%s=%i '%s'", __LINE__, opc, opv, qstr_str(CTX.qst))
    #define VM_TRACE(opc, opv) clog("      %i:%s=%i", __LINE__, opc, opv)

    #define RAISE(o) do { \
 MP_STATE_THREAD(active_exception) = MP_OBJ_TO_PTR(o);\
 clog("%i@%s:exit on ex!", __LINE__, __FILE__);\
 goto exception_handler; } while (0)

    #define RAISE_IF(arg) if (arg) { clog("%i@%s:exit on ex!", __LINE__, __FILE__); goto exception_handler; }
#else
    #define VM_TRACE_QSTR(opc)
    #define VM_TRACE(opc)

    #define RAISE(o) do { \
 MP_STATE_THREAD(active_exception) = MP_OBJ_TO_PTR(o);\
 goto exception_handler; } while (0)

    #define RAISE_IF(arg) if (arg) { goto exception_handler; }

#endif

#define VM_return(vm_ret_kind) { FRAME_LEAVE();\
CTX.vm_return_kind =  vm_ret_kind;\
CTX.switch_break_for = 1;\
break; }

#define RAISE(o) do { MP_STATE_THREAD(active_exception) = MP_OBJ_TO_PTR(o); goto exception_handler; } while (0)

#define RAISE_IF_NULL(arg) RAISE_IF(arg == MP_OBJ_NULL)
#define THE_EXC the_exc

//-----------------------------------------


#define EX_DECODE_ULABEL size_t ulab = (exip[0] | (exip[1] << 8)); exip += 2

#define VM_PUSH_EXC_BLOCK(with_or_finally) do { \
    VM_DECODE_ULABEL; /* except labels are always forward */ \
    ++CTX.exc_sp; \
    CTX.exc_sp->handler = CTX.ip + CTX.ulab; \
    CTX.exc_sp->val_sp = MP_TAGPTR_MAKE(CTX.sp, ((with_or_finally) << 1)); \
    CTX.exc_sp->prev_exc = NULL; \
} while (0)

#define VM_POP_EXC_BLOCK() \
    CTX.exc_sp--; /* pop back to previous exception handler */ \
    CLEAR_SYS_EXC_INFO() /* just clear sys.exc_info(), not compliant, but it shouldn't be used in 1st place */

#define VM_PUSH(val) (*++CTX.sp = (val))
#define VM_POP() (*CTX.sp--)
#define VM_TOP() (*CTX.sp)
#define VM_SET_TOP(val) (*CTX.sp = (val))

#define VM_DECODE_UINT \
    mp_uint_t unum = 0; \
    do { \
        unum = (unum << 7) + (*CTX.ip & 0x7f); \
    } while ((*CTX.ip++ & 0x80) != 0)

#define VM_DECODE_ULABEL CTX.ulab = (CTX.ip[0] | (CTX.ip[1] << 8)); CTX.ip += 2
#define VM_DECODE_SLABEL CTX.slab = (CTX.ip[0] | (CTX.ip[1] << 8)) - 0x8000; CTX.ip += 2

#define VM_DECODE_QSTR \
    CTX.qst = CTX.ip[0] | CTX.ip[1] << 8; \
    CTX.ip += 2;

#define VM_DECODE_PTR \
    VM_DECODE_UINT; \
    CTX.ptr = (void*)(uintptr_t)CTX.code_state->fun_bc->const_table[unum]

#define VM_DECODE_OBJ \
    VM_DECODE_UINT; \
    mp_obj_t obj = (mp_obj_t)CTX.code_state->fun_bc->const_table[unum]

#define LOCAL_NAME_ERROR() RAISE(mp_obj_new_exception_msg(&mp_type_NameError, MP_ERROR_TEXT("local variable referenced before assignment")))


#if MICROPY_PY_SYS_EXC_INFO
#define CLEAR_SYS_EXC_INFO() MP_STATE_VM(cur_exception) = NULL;
#else
#define CLEAR_SYS_EXC_INFO()
#endif


static mp_obj_t obj_shared;
static int GOTO_OUTER_VM_DISPATCH = 1;

def_mp_execute_bytecode: {

    // MUST BE SET CTX.inject_exc = MP_OBJ_NULL ;

    #if VM_OPT_COMPUTED_GOTO
        #error "no wasm support for VM_OPT_COMPUTED_GOTO"
    #else
        #define VM_ENTRY(op) case op
    #endif
//247
#if MICROPY_STACKLESS
run_code_state: ;
#endif

//252
FRAME_ENTER()
#if MICROPY_STACKLESS
run_code_state_from_return: ;
#endif
FRAME_SETUP();  clog("~vm.c:258 VM(%d)run_code_state", ctx_current);
//257
    // Pointers which are constant for particular invocation of mp_execute_bytecode()
    //mp_obj_t * /*const*/ fastn; mp_exc_stack_t * /*const*/ exc_stack;
    {
        size_t n_state = mp_decode_uint_value(CTX.code_state->fun_bc->bytecode);
        CTX.fastn = &CTX.code_state->state[n_state - 1];
        CTX.exc_stack = (mp_exc_stack_t*)(CTX.code_state->state + n_state);
    }
//266
    // variables that are visible to the exception handler (declared volatile)
    // CTX.exc_sp = MP_TAGPTR_PTR(CTX.code_state->exc_sp); // stack grows up, exc_sp points to top of stack
    CTX.exc_sp = MP_CODE_STATE_EXC_SP_IDX_TO_PTR(CTX.exc_stack, CTX.code_state->exc_sp_idx);
VM_DISPATCH_loop:

    if ( CTX.vmloop_state == VM_RESUMING ) {
        //? restore what ?
        clog("127:unwrap.c VM(%d)restored", ctx_current);
        //CTX.ip = CTX.code_state->ip;
        //CTX.sp = CTX.code_state->sp;
        //obj_shared = CTX.obj_shared ;
        CTX.vmloop_state = VM_RUNNING;
    }

    if (GOTO_OUTER_VM_DISPATCH) {
        clog("134:unwrap.c VM(%d)OUTER_DISPATCH for(){ ip:sp }", ctx_current); // loop to execute byte code
        GOTO_OUTER_VM_DISPATCH = 0;
        // local variables [that were not visible to the exception handler]
        //static const byte *ip;
        CTX.ip = CTX.code_state->ip;
        //static mp_obj_t *sp;
        CTX.sp = CTX.code_state->sp;
    }

#if VMTRACE
if (MP_STATE_THREAD(active_exception) != NULL) {
    clog("160:ERROR EXCEPTION ON ENTER[%d]", ctx_current);
} else {
    clog("162:unwrap.c VM(%d)DISPATCH->for()", ctx_current); // loop to execute byte code
}
#endif

    for (;;) {

        MICROPY_VM_HOOK_INIT

        // If we have exception to inject, now that we finish setting up
        // execution context, raise it. This works as if RAISE_VARARGS
        // bytecode was executed.
        // Injecting exc into yield from generator is a special case,
        // handled by MP_BC_YIELD_FROM itself
        if ( (CTX.inject_exc != MP_OBJ_NULL) && (*CTX.ip != MP_BC_YIELD_FROM) ) {
clog("157:unwrap.c pending_exception to inject");
            mp_obj_t exc = CTX.inject_exc;
            CTX.inject_exc = MP_OBJ_NULL;
            exc = mp_make_raise_obj(exc);
            RAISE(exc);
        }

        // stores ip pointing to last opcode
        CTX.code_state->ip = CTX.ip;

#if VMTRACE
if (SHOW_OS_LOOP>0) {
        #include "vmsl/vmbc_trace.c"
}
#endif
        switch (*CTX.ip++) {
            // opcode table, can be optimized later with computed goto
            // if compiler does not already.

            #include "vmsl/vmswitch.c"

        } // switch

/*
        if return was requested the switch is broken and  :

            - return value type is set in CTX.vm_return_kind

            - if (vm_return_kind == MP_VM_RETURN_NORMAL) then result is in CTX.code_state->sp

            - CTX.switch_break_for may be set to break from the for loop from here

*/

        if (CTX.switch_break_for) {
#if VMTRACE
clog("      195:switch_break_for")
#endif

            if (CTX.vm_return_kind == MP_VM_RETURN_NORMAL) {
#if VMTRACE
clog("      200:switch_break_for/vm_return_kind return value is set")
#endif
                RETVAL = *CTX.code_state->sp ;

            // an exception : returned exception is in state[0]
            } else if (CTX.vm_return_kind == MP_VM_RETURN_EXCEPTION) {
#if VMTRACE
clog("      208:switch_break_for/vm_return_kind exception value is set")
#endif
                RETVAL = CTX.code_state->state[0];
            } else {
    clog("      212:switch_break_for/unrouted vm_return_kind");
            }
            break; // go to end of } // for loop
        }

//1 463
//pending_exception_check:
        MICROPY_VM_HOOK_LOOP

        #if MICROPY_ENABLE_SCHEDULER
        // This is an inlined variant of mp_handle_pending
        if (MP_STATE_VM(sched_state) == MP_SCHED_PENDING) {
            mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
            mp_obj_t obj = MP_STATE_VM(mp_pending_exception);
            if (obj != MP_OBJ_NULL) {
                MP_STATE_VM(mp_pending_exception) = MP_OBJ_NULL;
                if (!mp_sched_num_pending()) {
                    MP_STATE_VM(sched_state) = MP_SCHED_IDLE;
                }
                MICROPY_END_ATOMIC_SECTION(atomic_state);
                RAISE(obj);
            }
            mp_handle_pending_tail(atomic_state);
        }
        #else
        // This is an inlined variant of mp_handle_pending
        if (MP_STATE_VM(mp_pending_exception) != MP_OBJ_NULL) {
            mp_obj_t obj = MP_STATE_VM(mp_pending_exception);
            MP_STATE_VM(mp_pending_exception) = MP_OBJ_NULL;
            RAISE(obj);
        }
        #endif

        #if MICROPY_PY_THREAD_GIL
        #if MICROPY_PY_THREAD_GIL_VM_DIVISOR
        if (--gil_divisor == 0)
        #endif
        {
            #if MICROPY_PY_THREAD_GIL_VM_DIVISOR
            gil_divisor = MICROPY_PY_THREAD_GIL_VM_DIVISOR;
            #endif
            #if MICROPY_ENABLE_SCHEDULER
            // can only switch threads if the scheduler is unlocked
            if (MP_STATE_VM(sched_state) == MP_SCHED_IDLE)
            #endif
            {
            MP_THREAD_GIL_EXIT();
            MP_THREAD_GIL_ENTER();
            }
        }
        #endif


        continue;


    exception_handler:
        // exception occurred
        assert(MP_STATE_THREAD(active_exception) != NULL);
#if VMTRACE
    clog("      508:loop exit on EX! CTX.ip=%i CTX.code_state->ip=%i", CTX.ip , CTX.code_state->ip );
#endif
        // clear exception because we caught it
        mp_obj_base_t *the_exc = MP_STATE_THREAD(active_exception);
        MP_STATE_THREAD(active_exception) = NULL;

        #if MICROPY_PY_SYS_EXC_INFO
        MP_STATE_VM(cur_exception) = the_exc;
        #endif

        #if SELECTIVE_EXC_IP
        // with selective ip, we store the ip 1 byte past the opcode, so move ptr back
        CTX.code_state->ip -= 1;
        #endif

//1 569
        #if MICROPY_PY_SYS_SETTRACE
        // Exceptions are traced here
        if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(((mp_obj_base_t*)the_exc)->type), MP_OBJ_FROM_PTR(&mp_type_Exception))) {
            TRACE_TICK(CTX.code_state->ip, CTX.code_state->sp, true /* yes, it's an exception */);
        }
        #endif

    #if MICROPY_STACKLESS
    unwind_loop:
    #endif
        // Set traceback info (file and line number) where the exception occurred, but not for:
        // - constant GeneratorExit object, because it's const
        // - exceptions re-raised by END_FINALLY
        // - exceptions re-raised explicitly by "raise"

        if (
                (THE_EXC != &mp_const_GeneratorExit_obj.base)
                && (*CTX.code_state->ip != MP_BC_END_FINALLY)
                && (*CTX.code_state->ip != MP_BC_RAISE_LAST)
            ) {
// same as vmbc_trace
            const byte *ipval = CTX.code_state->fun_bc->bytecode;
            MP_BC_PRELUDE_SIG_DECODE(ipval);
            MP_BC_PRELUDE_SIZE_DECODE(ipval);
            const byte *bytecode_start = ipval + n_info + n_cell;
            #if !MICROPY_PERSISTENT_CODE
                // so bytecode is aligned
                bytecode_start = MP_ALIGN(bytecode_start, sizeof(mp_uint_t));
            #endif
            size_t bc = CTX.code_state->ip - bytecode_start;
            #if MICROPY_PERSISTENT_CODE
                qstr block_name = ipval[0] | (ipval[1] << 8);
                qstr source_file = ipval[2] | (ipval[3] << 8);
                ipval += 4;
            #else
                qstr block_name = mp_decode_uint_value(ipval);
                ipval = mp_decode_uint_skip(ipval);
                qstr source_file = mp_decode_uint_value(ipval);
                ipval = mp_decode_uint_skip(ipval);
            #endif
            size_t source_line = mp_bytecode_get_source_line(ipval, bc);
// /vmbc_trace
            mp_obj_exception_add_traceback(MP_OBJ_FROM_PTR(THE_EXC), source_file, source_line, block_name);
        }
//1 615
        while ((CTX.exc_sp >= CTX.exc_stack) && (CTX.exc_sp->handler <= CTX.code_state->ip)) {

            // nested exception

            assert(CTX.exc_sp >= CTX.exc_stack);

            // TODO make a proper message for nested exception
            // at the moment we are just raising the very last exception (the one that caused the nested exception)

            clog("// move up to previous exception handler"); // move up to previous exception handler
            VM_POP_EXC_BLOCK();
        }

        if (CTX.exc_sp >= CTX.exc_stack) {
            // catch exception and pass to byte code

//PROBLEM HERE, ip was not set.
            CTX.ip = (CTX.code_state->ip = CTX.exc_sp->handler);
            mp_obj_t *sptr = MP_TAGPTR_PTR(CTX.exc_sp->val_sp);
            // save this exception in the stack so it can be used in a reraise, if needed
            CTX.exc_sp->prev_exc = THE_EXC;
            // push exception object so it can be handled by bytecode
            VM_PUSH(MP_OBJ_FROM_PTR(THE_EXC));
            CTX.code_state->sp = sptr;
#if VMTRACE
    clog("      638: where's my EX handler ???");
#endif

            continue;

//PROBLEM IS HERE
        #if MICROPY_STACKLESS

        } else if (CTX.code_state->prev != NULL) {
            mp_globals_set(CTX.code_state->old_globals);
            mp_code_state_t *new_code_state = CTX.code_state->prev;
            #if MICROPY_ENABLE_PYSTACK
            // Free code_state, and args allocated by mp_call_prepare_args_n_kw_var
            // (The latter is implicitly freed when using pystack due to its LIFO nature.)
            // The sizeof in the following statement does not include the size of the variable
            // part of the struct.  This arg is anyway not used if pystack is enabled.
            mp_nonlocal_free(CTX.code_state, sizeof(mp_code_state_t));
            #endif
            CTX.code_state = new_code_state;
            size_t n_state = CTX.code_state->n_state;
            CTX.fastn = &CTX.code_state->state[n_state - 1];
            CTX.exc_stack = (mp_exc_stack_t*)(CTX.code_state->state + n_state);
            // variables that are visible to the exception handler (declared volatile)
            CTX.exc_sp = MP_CODE_STATE_EXC_SP_IDX_TO_PTR(CTX.exc_stack, CTX.code_state->exc_sp_idx); // stack grows up, exc_sp points to top of stack
            goto unwind_loop;

        #endif
        } else {
            // propagate exception to higher level
            // Note: ip and sp don't have usable values at this point
            CTX.code_state->state[0] = MP_OBJ_FROM_PTR(THE_EXC); // put exception here because sp is invalid
            RETVAL = MP_OBJ_FROM_PTR(THE_EXC);
            FRAME_LEAVE();
            CTX.vm_return_kind = MP_VM_RETURN_EXCEPTION;
            clog("667: return EX");
            break;
        }

    } // for loop

    CTX.switch_break_for = 0 ;
    RETURN;



} // def_mp_execute_bytecode

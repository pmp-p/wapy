    static size_t iolen;

    if (VMOP>= VMOP_PAUSE) {
        VMOP--;
        if ( (iolen = has_io()) ) {
            // we have a chance to process pending Inputs, so ...
            cdbg("syscall/pause WITH IO=%lu", iolen );
            if (noint_aio_fsync()>0)
                goto VM_exhandler;
            else {
                IO_CODE_DONE;
            }

        } else {
            clog("syscall/pause -> io flush");
            // else just jump to flush Outputs
        }
        goto VM_syscall;
    }

// TODO: is it usefull ?
    if ( (ENTRY_POINT != JMP_NONE)  && !JUMPED_IN) {
        clog("re-enter-on-entry %d => %d\n", ctx_current, CTX.pointer);
        void* jump_entry;
        jump_entry = ENTRY_POINT;
        // Never to re-enter as this point. can only use the previous exit point.
        JUMPED_IN = 1;
        goto *jump_entry;
    }

// ==========================================================================================
    // this call the async loop , no preemption should be allowed in there.
    if (noint_aio_fsync()>0)
        goto VM_exhandler;
    else {
        IO_CODE_DONE;
    }
// ==========================================================================================

    // return to where we were going just before giving hand to host
    if ( (EXIT_POINT != JMP_NONE)  && JUMPED_IN) {
        //cdbg("re-enter-on-exit IO=%lu", strlen(io_stdin) );

        // was it gosub
        if (JUMP_TYPE == TYPE_SUB)
            RETURN;

        // was it branching
        if (JUMP_TYPE == TYPE_JUMP)
            COME_FROM;
    }


// All I/O stuff IS WRONG and should use a circular buffer.


    while (1){

/// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
        // process pending repl data
        if (io_stdin[0]) {
    //then it is toplevel or TODO: it's sync top level ( tranpiled by aio on the heap)


            if (def_PyRun_SimpleString_is_repl) {
                cdbg("REPL-INPUT-BEGIN[%s]", io_stdin);
            } else {
                cdbg("REPL-RAW-BEBIN[%s]", io_stdin);
            }

            // keep some space for moving labels


            JUMP( def_PyRun_SimpleString, "main_iteration_repl");





            if (def_PyRun_SimpleString_is_repl) {
                cdbg("REPL-INPUT-END");
            } else {
                cdbg("REPL-RAW-END");
            }

            if (RETVAL == MP_OBJ_NULL) {
                if (MP_STATE_THREAD(active_exception) != NULL) {
                    clog("64: RT Exception");
                }
            }


            // mark done
            IO_CODE_DONE;
        }
/// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
        //only flush kbd port on idle stdin

        char *keybuf;
        keybuf =  shm_get_ptr( IO_KBD, 0);

        def_PyRun_SimpleString_is_repl = false;
        while (!KPANIC) {
            // should give a way here to discard repl events feeding  "await input()" instead
            int rx =  keybuf[0] ; //& 0xFF;
            if (rx) {

                // if (rx==12) fprintf(stdout,"IO(12): Form Feed "); //clear screen
                //if (rx>127) cdbg("FIXME:stdin-utf8:%u", rx );
                //pyexec_event_repl_process_char(rx);
                if (pyexec_friendly_repl_process_char(rx)<0) {
                    pyexec_repl_repl_restart(0);
                    cdbg("REPL-INPUT-SET[%s]", io_stdin);
                    def_PyRun_SimpleString_is_repl = true;
                    break; // goto repl loop execution
                }

                *keybuf++ = 0;
            } else break; // no more input
        }

        // always reset io buffer no matter what since io_stdin can overwrite it in script mode
        keybuf[0]=0;

        // exit repl loop
        if (!io_stdin[0])
            break;
    }


    if (VMOP==VM_HCF) {
VM_stackmess:;
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

    //default is return EX
    mp_obj_t exret = MP_OBJ_NULL;

    if (lex == NULL) {
        clog("113:syntax error");
        //handle_uncaught_exception();
    } else {
        qstr source_name = lex->source_name;


        #if MICROPY_PY___FILE__
        if (input_kind == MP_PARSE_FILE_INPUT) {
            mp_store_global(MP_QSTR___file__, MP_OBJ_NEW_QSTR(source_name));
        }
        #endif

        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);

        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, def_PyRun_SimpleString_is_repl);


        if ( module_fun != MP_OBJ_NULL) {
            //STACKLESS STARTS HERE

            // unlock I/O buffer
            IO_CODE_DONE;

            CTX.self_in = module_fun;
            CTX.n_args = 0;
            CTX.n_kw = 0;
            CTX.args = NULL ;

            const mp_obj_type_t *type = mp_obj_get_type(CTX.self_in);

            exret = MP_OBJ_NULL;

            if (type->call != NULL) {
                if ( (int)*type->call == (int)&fun_bc_call ) {
                    // >>>
                    ctx_get_next(CTX_COPY);
                    GOSUB(def_func_bc_call, "mp_call_function_n_kw");
                    exret = SUBVAL; //CTX.sub_value;
                    // <<<
                } else {
                    clog("151: native call %p %p ?", *type->call , fun_bc_call);
                    exret = type->call(CTX.self_in, CTX.n_args, CTX.n_kw, CTX.args);
                }

            } else {
                    mp_raise_o(
                        mp_obj_new_exception_msg_varg(
                            &mp_type_TypeError,"'%s' object isn't callable", mp_obj_get_type_str(CTX.self_in)
                        )
                    );
            }

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
            clog("178: uncaught exception");
        }
        /*
        if (MP_STATE_THREAD(active_exception) != NULL) {
            clog("176: uncaught exception")
            //mp_hal_set_interrupt_char(-1);
            mp_handle_pending(false);
            //handle_uncaught_exception();
            if (uncaught_exception_handler()) {
                clog("181:SystemExit");
            } else {
                clog("183: exception done");
            }

        }
            */

    }
    RETVAL = exret ;
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

    if (type->call != NULL) {
        if ( (int)*type->call == (int)&fun_bc_call ) {
            // >>>
            ctx_get_next(CTX_COPY);
            GOSUB(def_func_bc_call, "mp_call_function_n_kw");
            RETVAL = SUBVAL; //CTX.sub_value;
            // <<<
        } else {
            clog("      899: native call %p %p", *type->call , &fun_bc_call);
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
    if ( CTX.state_size > 41360 ) {
#define TRACE_ON (1)
        #include "vmsl/vmbc_trace.c"
#undef TRACE_ON
        cdbg("882:BUG: =======> start=%p cur=%p end=%p state_size=%ld",
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
        clog("908:def_func_bc_call: MP_PYSTACK_ALLOC ex!");
        goto def_func_bc_call_ret;
    }

    CTX.inject_exc =  MP_OBJ_NULL ;
    CTX.code_state->fun_bc = CTX.self_fun;
    CTX.code_state->ip = 0;
    CTX.code_state->n_state = CTX.n_state;

    clog("917:TODO: can we save old_globals before this call ?");

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


    if (VMFLAGS_IF>0) { // FIXED !
        clog("132:unwrap.c ALLOWINT def_func_bc_call->def_mp_execute_bytecode");

        // ip sp would not be set on NEXT
        NEXT.ip = NEXT.code_state->ip;
        NEXT.sp = NEXT.code_state->sp;

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

VM_exhandler:;
    cdbg("457: EXCEPTION");
    // IO_CODE_DONE; follows
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

VM_syscall:;
    IO_CODE_DONE;
// TODO: flush all at once
    // STDOUT flush before eventually filling it again

#if __EMSCRIPTEN__

    // use json  { "channel" : "hexdata" }\n

    if (!rbb_is_empty(&out_rbb)) {
        // flush stdout
        unsigned char out_c = 0;
        fprintf(cc, "{\"%c\":\"", 48+1);
        //TODO put a 0 at end and printf buffer directly
        while (rbb_pop(&out_rbb, &out_c))
            fprintf(cc, "%c", out_c );
        fprintf(cc, "\"}\n");
    }
#else

    // @channel:hexdata\n

    if (!rbb_is_empty(&out_rbb)) {
        int cnt = 0;
        unsigned char out_c = 0;

        //TODO put a 0 at end and printf buffer directly via shm

        while (rbb_pop(&out_rbb, &out_c)) {
            if (!cnt--) {
                fprintf(cc, "\n@%c:%c", 48+1,out_c);
                cnt = 251;
            } else
                fprintf(cc, "%c", out_c );
        }
        fprintf(cc, "\n");
    }

#endif
































//

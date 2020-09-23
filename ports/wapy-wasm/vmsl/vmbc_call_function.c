










//1022
    VM_ENTRY(MP_BC_CALL_FUNCTION): {
        VM_TRACE("MP_BC_CALL_FUNCTION", MP_BC_CALL_FUNCTION);

        FRAME_UPDATE();
        VM_DECODE_UINT;

        CTX.sp -= (unum & 0xff) + ((unum >> 7) & 0x1fe);
        #if MICROPY_STACKLESS
        if (mp_obj_get_type(*CTX.sp) == &mp_type_fun_bc) {
            CTX.code_state->ip = CTX.ip;
            CTX.code_state->sp = CTX.sp;
            CTX.code_state->exc_sp_idx = MP_CODE_STATE_EXC_SP_IDX_FROM_PTR(CTX.exc_stack, CTX.exc_sp);
            mp_code_state_t *new_state = mp_obj_fun_bc_prepare_codestate(*CTX.sp, unum & 0xff, (unum >> 8) & 0xff, CTX.sp + 1);
            #if !MICROPY_ENABLE_PYSTACK
            if (new_state == NULL) {
                // Couldn't allocate codestate on heap: in the strict case raise
                // an exception, otherwise just fall through to stack allocation.
                #if MICROPY_STACKLESS_STRICT
                goto deep_recursion_error;
/*
deep_recursion_error:
    mp_raise_recursion_depth();
    RAISE_IF(true);
*/
                #endif //MICROPY_STACKLESS_STRICT
            } else
            #endif //!MICROPY_ENABLE_PYSTACK
            {
                new_state->prev = CTX.code_state;
                CTX.code_state = new_state;
                goto run_code_state;
            }
        } else
            clog("35:MP_BC_CALL_FUNCTION not mp_type_fun_bc");
        #endif //MICROPY_STACKLESS
    //1058






























        const char *fn;




        ctx_get_next(CTX_NEW);
        NEXT.self_in = *CTX.sp;
        NEXT.sp = CTX.sp;
        NEXT.n_args = unum & 0xff;
        NEXT.n_kw =  (unum >> 8) & 0xff;
        NEXT.args = CTX.sp + 1;



        if (!*CTX.sp) {
            ctx_abort(); // 39
            clog("47:MP_BC_CALL_FUNCTION NPE");
            mp_obj_t obj = mp_obj_new_exception_msg(&mp_type_RuntimeError, "Invalid Function Name->Pointer mapping called");
            mp_raise_o(obj);
            RAISE_IF_NULL(VM_SET_TOP(MP_OBJ_NULL));
            continue;
        }

        fn = qstr_str(mp_obj_fun_get_name(NEXT.self_in));

        if ( strlen(fn)>1 ) {
#if DEBUG_BC
    clog("      99:MP_BC_CALL_FUNCTION [%s %zu '%s']",mp_obj_get_type_str(*CTX.sp), mp_obj_fun_get_name(*CTX.sp), fn);
#endif
            GOSUB(def_mp_call_function_n_kw, fn );
        } else {
#if DEBUG_BC
    clog("      102:MP_BC_CALL_FUNCTION [%s %zu]",mp_obj_get_type_str(*CTX.sp), mp_obj_fun_get_name(*CTX.sp));
#endif
                GOSUB(def_mp_call_function_n_kw, mp_obj_get_type_str(NEXT.self_in) );
        }

        if (MP_STATE_THREAD(active_exception) != NULL) {
            clog("120:MP_BC_CALL_FUNCTION[%s]/exit on EX!", fn);
        }

        RAISE_IF_NULL(VM_SET_TOP(SUBVAL));

        continue;

    }




























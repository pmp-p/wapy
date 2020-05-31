// OK+

VM_ENTRY(MP_BC_CALL_METHOD): {
    VM_DECODE_UINT;
    // unum & 0xff == n_positional
    // (unum >> 8) & 0xff == n_keyword
    CTX.sp -= (unum & 0xff) + ((unum >> 7) & 0x1fe) + 1;
    int native = 0;
    #if MICROPY_STACKLESS
    if (mp_obj_get_type(*CTX.sp) == &mp_type_fun_bc) {
        CTX.code_state->ip = CTX.ip;
        CTX.code_state->sp = CTX.sp;
        CTX.code_state->exc_sp_idx = MP_CODE_STATE_EXC_SP_IDX_FROM_PTR(CTX.exc_stack, CTX.exc_sp);

        size_t n_args = unum & 0xff;
        size_t n_kw = (unum >> 8) & 0xff;
        int adjust = (CTX.sp[1] == MP_OBJ_NULL) ? 0 : 1;

        mp_code_state_t *new_state = mp_obj_fun_bc_prepare_codestate(*CTX.sp, n_args + adjust, n_kw, CTX.sp + 2 - adjust);
        #if !MICROPY_ENABLE_PYSTACK
        if (new_state == NULL) {
            // Couldn't allocate codestate on heap: in the strict case raise
            // an exception, otherwise just fall through to stack allocation.
            #if MICROPY_STACKLESS_STRICT
                goto deep_recursion_error;
            #endif
        } else
        #endif
        {
            new_state->prev = CTX.code_state;
            CTX.code_state = new_state;
            goto run_code_state;
        }
    } else {
        clog("38:MP_BC_CALL_METHOD native");
        native = 1;
    }

    #endif

    ctx_get_next(CTX_NEW);
    NEXT.args = CTX.sp ;

    NEXT.self_in = NEXT.args[0];
    NEXT.n_args = unum & 0xff;
    NEXT.n_kw = (unum >> 8) & 0xff;

    {
        int adjust = (NEXT.args[1] == MP_OBJ_NULL) ? 0 : 1;
        NEXT.n_args += adjust;
        NEXT.args += 2 - adjust;
    }



//57: DO NOT USE LINE 57 FOR SUB












    if (native) {
        GOSUB(def_mp_call_function_n_kw, "BC_CALL_METHOD(native)" );
    } else {
#pragma message "TODO FIXME sometimes mp_obj_fun_get_name() leads to out of bound access eg on .popitem() "
/*
        qstr FUN_QSTR = mp_obj_fun_get_name( NEXT.self_in );
        if (FUN_QSTR != MP_QSTR_ ) {
            GOSUB(def_mp_call_function_n_kw, qstr_str(FUN_QSTR) );
        } else*/
            GOSUB(def_mp_call_function_n_kw, "BC_CALL_METHOD(?name?)" );
    }

    VM_SET_TOP(SUBVAL);
    continue;
}



// OK+

#ifndef MICROPY_CAN_OVERRIDE_BUILTINS
#define MICROPY_CAN_OVERRIDE_BUILTINS 1
#endif

//1364
VM_ENTRY(MP_BC_IMPORT_NAME): {
    FRAME_UPDATE();
    VM_DECODE_QSTR;  // qst => import [name]

    clog("BC_IMPORT_NAME(%d:%d): [%s]\n", ctx_current, CTX.sub_id, qstr_str(CTX.qst) );

    ctx_get_next(CTX_COPY);
    //NEXT.qst = CTX.qst;
    NEXT.n_args = 5 ;
    NEXT.n_kw = 0;
    NEXT.argv[0] = MP_OBJ_NEW_QSTR(CTX.qst) ;
    NEXT.argv[1] = mp_const_none; // TODO should be globals
    NEXT.argv[2] = mp_const_none; // TODO should be globals
    NEXT.argv[3] = VM_POP(); // fromlist
    NEXT.argv[4] = VM_TOP();

    NEXT.args  = &NEXT.argv[0] ; //implicit

    //if ( !strcmp(qstr_str(CTX.qst),"syscall") )
      //  clog("    BC_IMPORT_NAME(%d:%d) import %s->pause", ctx_current, CTX.sub_id, qstr_str(CTX.qst) );


        #if MICROPY_CAN_OVERRIDE_BUILTINS
        {
            mp_obj_dict_t *bo_dict = MP_STATE_VM(mp_module_builtins_override_dict);
            // lookup __import__ and call that instead of going straight to builtin implementation
            if (bo_dict != NULL) {
                mp_map_elem_t *cust_imp = mp_map_lookup(&bo_dict->map, MP_OBJ_NEW_QSTR(MP_QSTR___import__), MP_MAP_LOOKUP);
                if (cust_imp != NULL) {
                    NEXT.self_in = cust_imp->value ;
                    GOSUB(def_mp_call_function_n_kw, qstr_str(CTX.qst));

                    goto RET_import_name;
                } // no continuation
            }
        }
        #endif

        SUBVAL = mp_builtin___import__(5, NEXT.argv);

        //ctx_release();

    RET_import_name:
        // not a sub, here we don't return
        RETVAL = SUBVAL ; //VM_SET_TOP( CTX.sub_value );
        VM_SET_TOP( RETVAL );

        RAISE_IF_NULL( RETVAL );
    // TODO:CTX

    if ( !strcmp(qstr_str(CTX.qst),"syscall") ) {
        //clog("    BC_IMPORT_NAME(%d:%d) import %s->pause", ctx_current, CTX.sub_id, qstr_str(CTX.qst) );
        BRANCH(VM_syscall, VMOP_PAUSE, VM_DISPATCH_loop, "BC_IMPORT_NAME(syscall)");
    }

    continue;
}

//1374
VM_ENTRY(MP_BC_IMPORT_FROM): {
    FRAME_UPDATE();
    VM_DECODE_QSTR;
    mp_obj_t obj = mp_import_from(VM_TOP(), CTX.qst);
    RAISE_IF_NULL(obj);
    VM_PUSH(obj);
    continue;
}

//1386
VM_ENTRY(MP_BC_IMPORT_STAR): {
    mp_import_all(VM_POP());
    continue;
}



















//

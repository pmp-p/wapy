
//1266
        VM_ENTRY(MP_BC_RAISE_LAST): {
            // search for the inner-most previous exception, to reraise it
            mp_obj_t obj = MP_OBJ_NULL;
            for (mp_exc_stack_t *e = CTX.exc_sp; e >= CTX.exc_stack; --e) {
                if (e->prev_exc != NULL) {
                    obj = MP_OBJ_FROM_PTR(e->prev_exc);
                    break;
                }
            }
            if (obj == MP_OBJ_NULL) {
                obj = mp_obj_new_exception_msg(&mp_type_RuntimeError, "no active exception to reraise");
            }
            RAISE(obj);
        }
//1282
        VM_ENTRY(MP_BC_RAISE_OBJ): {
            mp_obj_t obj = mp_make_raise_obj(VM_TOP());
            RAISE(obj);
        }
//1288
        VM_ENTRY(MP_BC_RAISE_FROM): {
            mp_warning(NULL, "exception chaining not supported");
            CTX.sp--; // ignore (pop) "from" argument
            mp_obj_t obj = mp_make_raise_obj(VM_TOP());
            RAISE(obj);
        }

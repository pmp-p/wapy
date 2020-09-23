//385
    VM_ENTRY(MP_BC_LOAD_NAME): {
        VM_DECODE_QSTR;
        mp_map_elem_t *elem = mp_map_cached_lookup(&mp_locals_get()->map, CTX.qst, (uint8_t*)CTX.ip);

        mp_obj_t obj;
        if (elem != NULL) {
            obj = elem->value;
        } else {
            obj = mp_load_name(CTX.qst);
            RAISE_IF_NULL(obj);
        }
        VM_PUSH(obj);
        CTX.ip++;
        continue;
    }

//413
    VM_ENTRY(MP_BC_LOAD_GLOBAL): {
        VM_DECODE_QSTR;
        mp_map_elem_t *elem = mp_map_cached_lookup(&mp_globals_get()->map, CTX.qst, (uint8_t*)CTX.ip);
        mp_obj_t obj;
        if (elem != NULL) {
            obj = elem->value;
        } else {
            obj = mp_load_global(CTX.qst);
            RAISE_IF_NULL(obj);
        }
        VM_PUSH(obj);
        CTX.ip++;
        continue;
    }

//442
    VM_ENTRY(MP_BC_LOAD_ATTR): {
        FRAME_UPDATE();
        VM_DECODE_QSTR;
        mp_obj_t top = VM_TOP();
        mp_map_elem_t *elem = NULL;
        if (mp_obj_is_instance_type(mp_obj_get_type(top))) {
            mp_obj_instance_t *self = MP_OBJ_TO_PTR(top);
            elem = mp_map_cached_lookup(&self->members, CTX.qst, (uint8_t*)CTX.ip);
        }
        mp_obj_t obj;
        if (elem != NULL) {
            obj = elem->value;
        } else {
            obj = mp_load_attr(top, CTX.qst);
            RAISE_IF_NULL(obj);
        }
        VM_SET_TOP(obj);
        CTX.ip++;
        continue;
    }

    // This caching code works with MICROPY_PY_BUILTINS_PROPERTY and/or
    // MICROPY_PY_DESCRIPTORS enabled because if the attr exists in
    // self->members then it can't be a property or have descriptors.  A
    // consequence of this is that we can't use MP_MAP_LOOKUP_ADD_IF_NOT_FOUND
    // in the fast-path below, because that store could override a property.
    VM_ENTRY(MP_BC_STORE_ATTR): {
        FRAME_UPDATE();
        VM_DECODE_QSTR;
        mp_map_elem_t *elem = NULL;
        mp_obj_t top = VM_TOP();
        if (mp_obj_is_instance_type(mp_obj_get_type(top)) && CTX.sp[-1] != MP_OBJ_NULL) {
            mp_obj_instance_t *self = MP_OBJ_TO_PTR(top);
            elem = mp_map_cached_lookup(&self->members, CTX.qst, (uint8_t*)CTX.ip);
        }
        if (elem != NULL) {
            elem->value = CTX.sp[-1];
        } else {
            RAISE_IF_NULL(mp_store_attr(CTX.sp[0], CTX.qst, CTX.sp[-1]));
        }
        CTX.sp -= 2;
        CTX.ip++;
        continue;
    }

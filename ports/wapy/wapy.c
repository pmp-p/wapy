
STATIC void stderr_print_strn2(void *env, const char *str, size_t len) {
    (void)env;
    mp_hal_stdout_tx_strn(str,len);
}

const mp_print_t mp_stderr_print2 = {NULL, stderr_print_strn2};

#if 0
    int uncaught_exception_handler(void) {
        mp_obj_base_t *exc = MP_STATE_THREAD(active_exception);
        // check for SystemExit
        if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(exc->type), MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
            // None is an exit value of 0; an int is its value; anything else is 1
            /*
            mp_obj_t exit_val = mp_obj_exception_get_value(MP_OBJ_FROM_PTR(exc));
            mp_int_t val = 0;
            if (exit_val != mp_const_none && !mp_obj_get_int_maybe(exit_val, &val)) {
                val = 1;
            }
            return FORCED_EXIT | (val & 255);
            */
            #if defined(__EMSCRIPTEN__)
                EM_ASM({console.log("91:SystemExit");});
            #endif

            return 1;
        }
        MP_STATE_THREAD(active_exception) = NULL;
        // Report all other exceptions
        cdbg("mp_stderr_print2=%p",exc)
        cdbg("mp_obj=%p", MP_OBJ_FROM_PTR(exc) );
        mp_obj_print_exception(&mp_stderr_print2, MP_OBJ_FROM_PTR(exc));
        return 0;
    }
#else
    // user defined exception handler
    int uncaught_exception_handler(void) {
        mp_obj_base_t *ex = MP_STATE_THREAD(active_exception);
        // check for SystemExit
        if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(ex->type), MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
            // None is an exit value of 0; an int is its value; anything else is 1
            /*
            mp_obj_t exit_val = mp_obj_exception_get_value(MP_OBJ_FROM_PTR(exc));
            mp_int_t val = 0;
            if (exit_val != mp_const_none && !mp_obj_get_int_maybe(exit_val, &val)) {
                val = 1;
            }
            return FORCED_EXIT | (val & 255);
            */
            clog("499:SystemExit");
            return 1;
        }
        MP_STATE_THREAD(active_exception) = NULL;

        // fake sys.excepthook via pythons.__init__
        fprintf(stderr, "*** EXCEPTION ***\n");
        pyv( mp_obj_get_type(ex) );
        pyv( MP_OBJ_FROM_PTR(ex) );
        pyv( MP_ROM_NONE );
        pycore("pyc_excepthook");
        return 0;
    }
#endif

void
dump_args2(const mp_obj_t *a, size_t sz) {
    fprintf(stderr,"331: %p: ", a);
    for (size_t i = 0; i < sz; i++) {
        fprintf(stderr,"%p ", a[i]);
    }
    fprintf(stderr,"\n");
}



// this is reserved to max speed asynchronous code

int
noint_aio_fsync() {

    if (!io_stdin[0])
        return 0;

    if (!endswith(io_stdin, "#aio.step\n"))
        return 0;

    int ex=-1;
    async_state = VMFLAGS_IF;
    // CLI
    VMFLAGS_IF = 0;

    //TODO: maybe somehow consumme kbd data for async inputs ?
    //expect script to be properly async programmed and run them full speed via C stack ?

    if (async_loop) {

        if ( (async_loop = pyeval(i_main.shm_stdio, MP_PARSE_FILE_INPUT))  ) {
            ex=0;
        } else {
            fprintf(stdout, "ERROR[%s]\n", io_stdin);
            // ex check
            ex=1;
        }

    }

    // STI
    VMFLAGS_IF = async_state;
    return ex;
}



int
endswith(const char * str, const char * suffix) {
  int str_len = strlen(str);
  int suffix_len = strlen(suffix);

  return
    (str_len >= suffix_len) && (0 == strcmp(str + (str_len-suffix_len), suffix));
}



// do not run lines starting with #
size_t
has_io() {
    size_t check = strlen(io_stdin);
    if (io_stdin[0] && (check != 38))
        return check;
    return 0;
}








#if 0 // for old esp broken SDK strcmp
    size_t
    bsd_strlen(const char *str) {
            const char *s;
            for (s = str; *s; ++s);
            return (s - str);
    }

    int
    bsd_strcmp(const char *s1, const char *s2) {
        while (*s1 == *s2++)
            if (*s1++ == '\0')
                return (0);
        return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
    }

    int
    endswith(const char * str, const char * suffix) {
      int str_len = bsd_strlen(str);
      int suffix_len = bsd_strlen(suffix);

      return
        (str_len >= suffix_len) && (0 == bsd_strcmp(str + (str_len-suffix_len), suffix));
    }

#endif
































//

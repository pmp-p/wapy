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
        qstr source_file = ipval[2] | (ipval[3] << 8);
        ipval += 4;
        #else
        ipval = mp_decode_uint_skip(ipval);
        qstr source_file = mp_decode_uint_value(ipval);
        ipval = mp_decode_uint_skip(ipval);
        #endif
        size_t source_line = mp_bytecode_get_source_line(ipval, bc);
        static size_t last_line = 0;

        if (source_line!=last_line) {
#if TRACE_ON
            cdbg("\nbc:%i ctx=%i %s:%zu", *CTX.ip , ctx_current,qstr_str(source_file), source_line);
#else
            clog("\nbc:%i ctx=%i %s:%zu", *CTX.ip , ctx_current,qstr_str(source_file), source_line);
#endif

            last_line = source_line;
        }


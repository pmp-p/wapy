/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/frozenmod.h"
#include "py/mphal.h"
#if MICROPY_HW_ENABLE_USB
#include "irq.h"
#include "usb.h"
#endif

#include "lib/mp-readline/readline.h"

#include "lib/utils/pyexec.h"
#include "genhdr/mpversion.h"

// #include "lib/utils/interrupt_char.h" // for mp_hal_set_interrupt_char
extern void mp_hal_set_interrupt_char(int c);

pyexec_mode_kind_t pyexec_mode_kind = PYEXEC_MODE_FRIENDLY_REPL;
int pyexec_system_exit = 0;
STATIC bool repl_display_debugging_info = 0;

#define EXEC_FLAG_PRINT_EOF (1)
#define EXEC_FLAG_ALLOW_DEBUGGING (2)
#define EXEC_FLAG_IS_REPL (4)
#define EXEC_FLAG_SOURCE_IS_RAW_CODE (8)
#define EXEC_FLAG_SOURCE_IS_VSTR (16)
#define EXEC_FLAG_SOURCE_IS_FILENAME (32)

// parses, compiles and executes the code in the lexer
// frees the lexer before returning
// EXEC_FLAG_PRINT_EOF prints 2 EOF chars: 1 after normal output, 1 after exception output
// EXEC_FLAG_ALLOW_DEBUGGING allows debugging info to be printed after executing the code
// EXEC_FLAG_IS_REPL is used for REPL inputs (flag passed on to mp_compile)



#if NO_NLR
    #include "../wapy/upython.h"

    #if defined(__EMSCRIPTEN__)
        #pragma message "TODO: pyexec->no_nlr is crude"
        #include "emscripten.h"
    #endif


    #define FORCED_EXIT (0x100)
    // If exc is SystemExit, return value where FORCED_EXIT bit set,
    // and lower 8 bits are SystemExit value. For all other exceptions,
    // return 1.

    STATIC void stderr_print_strn(void *env, const char *str, size_t len) {
        (void)env;
        mp_hal_stdout_tx_strn(str,len);
    }

    const mp_print_t mp_stderr_print = {NULL, stderr_print_strn};


    STATIC int handle_uncaught_exception(void) {
        mp_obj_base_t *exc = MP_STATE_THREAD(active_exception);
        // check for SystemExit
        if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(exc->type), MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
            //clog("89:SystemExit");
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
        mp_obj_print_exception(&mp_stderr_print, MP_OBJ_FROM_PTR(exc));
        return 0;
    }

    STATIC int parse_compile_execute(const void *source, mp_parse_input_kind_t input_kind, int exec_flags) {
        int retval = 0;
        uint32_t start = 0;

        // by default a SystemExit exception returns 0
        pyexec_system_exit = 0;


        mp_obj_t module_fun;
        #if MICROPY_MODULE_FROZEN_MPY
        if (exec_flags & EXEC_FLAG_SOURCE_IS_RAW_CODE) {
            // source is a raw_code object, create the function
            module_fun = mp_make_function_from_raw_code(source, MP_OBJ_NULL, MP_OBJ_NULL);
        } else
        #endif
        {
            #if MICROPY_ENABLE_COMPILER
            mp_lexer_t *lex;
            if (exec_flags & EXEC_FLAG_SOURCE_IS_VSTR) {
                const vstr_t *vstr = source;
                lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, vstr->buf, vstr->len, 0);
            } else if (exec_flags & EXEC_FLAG_SOURCE_IS_FILENAME) {
                lex = mp_lexer_new_from_file(source);
            } else {
                lex = (mp_lexer_t*)source;
            }
            // source is a lexer, parse and compile the script
            qstr source_name = lex->source_name;
            mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
            module_fun = mp_compile(&parse_tree, source_name, exec_flags & EXEC_FLAG_IS_REPL);
            #else
            mp_raise_msg(&mp_type_RuntimeError, "script compilation not supported");
            #endif
        }


        if ( module_fun != MP_OBJ_NULL) {
            // execute code
            mp_hal_set_interrupt_char(CHAR_CTRL_C); // allow ctrl-C to interrupt us
            start = mp_hal_ticks_ms();
            mp_obj_t ret = mp_call_function_0(module_fun);
            mp_hal_set_interrupt_char(-1); // disable interrupt

            if (exec_flags & EXEC_FLAG_PRINT_EOF) {
                mp_hal_stdout_tx_strn("\x04", 1);
            }

            if (ret != MP_OBJ_NULL && MP_STATE_VM(mp_pending_exception) != MP_OBJ_NULL) {
                mp_obj_t obj = MP_STATE_VM(mp_pending_exception);
                MP_STATE_VM(mp_pending_exception) = MP_OBJ_NULL;
                mp_raise_o(obj);
            }

            if (MP_STATE_THREAD(active_exception) != NULL) {
                // uncaught exception
                return handle_uncaught_exception();
            }
        }

        return retval;
    }

#else
    STATIC int parse_compile_execute(const void *source, mp_parse_input_kind_t input_kind, int exec_flags) {
        int ret = 0;
        uint32_t start = 0;

        // by default a SystemExit exception returns 0
        pyexec_system_exit = 0;

        nlr_buf_t nlr;
        if (nlr_push(&nlr) == 0) {
            mp_obj_t module_fun;
            #if MICROPY_MODULE_FROZEN_MPY
            if (exec_flags & EXEC_FLAG_SOURCE_IS_RAW_CODE) {
                // source is a raw_code object, create the function
                module_fun = mp_make_function_from_raw_code(source, MP_OBJ_NULL, MP_OBJ_NULL);
            } else
            #endif
            {
                #if MICROPY_ENABLE_COMPILER
                mp_lexer_t *lex;
                if (exec_flags & EXEC_FLAG_SOURCE_IS_VSTR) {
                    const vstr_t *vstr = source;
                    lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, vstr->buf, vstr->len, 0);
                } else if (exec_flags & EXEC_FLAG_SOURCE_IS_FILENAME) {
                    lex = mp_lexer_new_from_file(source);
                } else {
                    lex = (mp_lexer_t*)source;
                }
                // source is a lexer, parse and compile the script
                qstr source_name = lex->source_name;
                mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
                module_fun = mp_compile(&parse_tree, source_name, exec_flags & EXEC_FLAG_IS_REPL);
                #else
                mp_raise_msg(&mp_type_RuntimeError, "script compilation not supported");
                #endif
            }

            // execute code
            mp_hal_set_interrupt_char(CHAR_CTRL_C); // allow ctrl-C to interrupt us
            start = mp_hal_ticks_ms();
            mp_call_function_0(module_fun);
            mp_hal_set_interrupt_char(-1); // disable interrupt
            nlr_pop();
            ret = 1;
            if (exec_flags & EXEC_FLAG_PRINT_EOF) {
                mp_hal_stdout_tx_strn("\x04", 1);
            }
        } else {
            // uncaught exception
            // FIXME it could be that an interrupt happens just before we disable it here
            mp_hal_set_interrupt_char(-1); // disable interrupt
            // print EOF after normal output
            if (exec_flags & EXEC_FLAG_PRINT_EOF) {
                mp_hal_stdout_tx_strn("\x04", 1);
            }
            // check for SystemExit
            if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(((mp_obj_base_t*)nlr.ret_val)->type), MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
                // at the moment, the value of SystemExit is unused
                ret = pyexec_system_exit;
            } else {
                mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));
                ret = 0;
            }
        }

        // display debugging info if wanted
        if ((exec_flags & EXEC_FLAG_ALLOW_DEBUGGING) && repl_display_debugging_info) {
            mp_uint_t ticks = mp_hal_ticks_ms() - start; // TODO implement a function that does this properly
            printf("took " UINT_FMT " ms\n", ticks);
            // qstr info
            {
                size_t n_pool, n_qstr, n_str_data_bytes, n_total_bytes;
                qstr_pool_info(&n_pool, &n_qstr, &n_str_data_bytes, &n_total_bytes);
                printf("qstr:\n  n_pool=%u\n  n_qstr=%u\n  "
                       "n_str_data_bytes=%u\n  n_total_bytes=%u\n",
                       (unsigned)n_pool, (unsigned)n_qstr, (unsigned)n_str_data_bytes, (unsigned)n_total_bytes);
            }

            #if MICROPY_ENABLE_GC
            // run collection and print GC info
            gc_collect();
            gc_dump_info();
            #endif
        }

        if (exec_flags & EXEC_FLAG_PRINT_EOF) {
            mp_hal_stdout_tx_strn("\x04", 1);
        }

        return ret;
    }
#endif


#if MICROPY_ENABLE_COMPILER

    #if MICROPY_REPL_EVENT_DRIVEN
        #if NO_NLR
            #include "../wapy/repl.c"
        #else
            #include "../wapy/repl_nlr.c"
        #endif

        #if 0
            extern repl_t repl;
            extern int pyexec_repl_repl_restart(int ret);
            extern int pyexec_raw_repl_process_char(int c);
            extern int pyexec_friendly_repl_process_char(int c);
        #endif

    #else // MICROPY_REPL_EVENT_DRIVEN
        #pragma message "[redacted]"
    #endif // MICROPY_REPL_EVENT_DRIVEN

#endif // MICROPY_ENABLE_COMPILER

int pyexec_file(const char *filename) {
    return parse_compile_execute(filename, MP_PARSE_FILE_INPUT, EXEC_FLAG_SOURCE_IS_FILENAME);
}

int pyexec_file_if_exists(const char *filename) {
    #if MICROPY_MODULE_FROZEN
    if (mp_frozen_stat(filename) == MP_IMPORT_STAT_FILE) {
        return pyexec_frozen_module(filename);
    }
    #endif
    if (mp_import_stat(filename) != MP_IMPORT_STAT_FILE) {
        return 1; // success (no file is the same as an empty file executing without fail)
    }
    return pyexec_file(filename);
}

#if MICROPY_MODULE_FROZEN
int pyexec_frozen_module(const char *name) {
    void *frozen_data;
    int frozen_type = mp_find_frozen_module(name, strlen(name), &frozen_data);

    switch (frozen_type) {
        #if MICROPY_MODULE_FROZEN_STR
        case MP_FROZEN_STR:
            return parse_compile_execute(frozen_data, MP_PARSE_FILE_INPUT, 0);
        #endif

        #if MICROPY_MODULE_FROZEN_MPY
        case MP_FROZEN_MPY:
            return parse_compile_execute(frozen_data, MP_PARSE_FILE_INPUT, EXEC_FLAG_SOURCE_IS_RAW_CODE);
        #endif

        default:
            printf("could not find module '%s'\n", name);
            return false;
    }
}
#endif

mp_obj_t pyb_set_repl_info(mp_obj_t o_value) {
    repl_display_debugging_info = mp_obj_get_int(o_value);
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_1(pyb_set_repl_info_obj, pyb_set_repl_info);

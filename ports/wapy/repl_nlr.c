
typedef struct _repl_t {
    // This structure originally also held current REPL line,
    // but it was moved to MP_STATE_VM(repl_line) as containing
    // root pointer. Still keep structure in case more state
    // will be added later.
    // vstr_t line;
    bool cont_line;
    bool paste_mode;
} repl_t;

repl_t repl;

STATIC int pyexec_raw_repl_process_char(int c);
STATIC int pyexec_friendly_repl_process_char(int c);

void pyexec_event_repl_init(void) {
    MP_STATE_VM(repl_line) = vstr_new(32);
    repl.cont_line = false;
    repl.paste_mode = false;
    // no prompt before printing friendly REPL banner or entering raw REPL
    readline_init(MP_STATE_VM(repl_line), "");
    if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
        pyexec_raw_repl_process_char(CHAR_CTRL_A);
    } else {
        pyexec_friendly_repl_process_char(CHAR_CTRL_B);
    }
}

STATIC int pyexec_raw_repl_process_char(int c) {
    if (c == CHAR_CTRL_A) {
        // reset raw REPL
        mp_hal_stdout_tx_str("raw REPL; CTRL-B to exit\r\n");
        goto reset;
    } else if (c == CHAR_CTRL_B) {
        // change to friendly REPL
        pyexec_mode_kind = PYEXEC_MODE_FRIENDLY_REPL;
        vstr_reset(MP_STATE_VM(repl_line));
        repl.cont_line = false;
        repl.paste_mode = false;
        pyexec_friendly_repl_process_char(CHAR_CTRL_B);
        return 0;
    } else if (c == CHAR_CTRL_C) {
        // clear line
        vstr_reset(MP_STATE_VM(repl_line));
        return 0;
    } else if (c == CHAR_CTRL_D) {
        // input finished
    } else {
        // let through any other raw 8-bit value
        vstr_add_byte(MP_STATE_VM(repl_line), c);
        return 0;
    }

    // indicate reception of command
    mp_hal_stdout_tx_str("OK");

    if (MP_STATE_VM(repl_line)->len == 0) {
        // exit for a soft reset
        mp_hal_stdout_tx_str("\r\n");
        vstr_clear(MP_STATE_VM(repl_line));
        return PYEXEC_FORCED_EXIT;
    }

    int ret = parse_compile_execute(MP_STATE_VM(repl_line), MP_PARSE_FILE_INPUT, EXEC_FLAG_PRINT_EOF | EXEC_FLAG_SOURCE_IS_VSTR);
    if (ret & PYEXEC_FORCED_EXIT) {
        return ret;
    }

reset:
    vstr_reset(MP_STATE_VM(repl_line));
    mp_hal_stdout_tx_str(">");

    return 0;
}

STATIC int pyexec_friendly_repl_process_char(int c) {
    if (repl.paste_mode) {
        if (c == CHAR_CTRL_C) {
            // cancel everything
            mp_hal_stdout_tx_str("\r\n");
            goto input_restart;
        } else if (c == CHAR_CTRL_D) {
            // end of input
            mp_hal_stdout_tx_str("\r\n");
            int ret = parse_compile_execute(MP_STATE_VM(repl_line), MP_PARSE_FILE_INPUT, EXEC_FLAG_ALLOW_DEBUGGING | EXEC_FLAG_IS_REPL | EXEC_FLAG_SOURCE_IS_VSTR);
            if (ret & PYEXEC_FORCED_EXIT) {
                return ret;
            }
            goto input_restart;
        } else {
            // add char to buffer and echo
            vstr_add_byte(MP_STATE_VM(repl_line), c);
            if (c == '\r') {
                mp_hal_stdout_tx_str("\r\n=== ");
            } else {
                char buf[1] = {c};
                mp_hal_stdout_tx_strn(buf, 1);
            }
            return 0;
        }
    }

    int ret = readline_process_char(c);

    if (!repl.cont_line) {

        if (ret == CHAR_CTRL_A) {
            // change to raw REPL
            pyexec_mode_kind = PYEXEC_MODE_RAW_REPL;
            mp_hal_stdout_tx_str("\r\n");
            pyexec_raw_repl_process_char(CHAR_CTRL_A);
            return 0;
        } else if (ret == CHAR_CTRL_B) {
            // reset friendly REPL
            mp_hal_stdout_tx_str("\r\n");
            mp_hal_stdout_tx_str("MicroPython " MICROPY_GIT_TAG " on " MICROPY_BUILD_DATE "; " MICROPY_HW_BOARD_NAME " with " MICROPY_HW_MCU_NAME "\r\n");
            #if MICROPY_PY_BUILTINS_HELP
            mp_hal_stdout_tx_str("Type \"help()\" for more information.\r\n");
            #endif
            goto input_restart;
        } else if (ret == CHAR_CTRL_C) {
            // break
            mp_hal_stdout_tx_str("\r\n");
            goto input_restart;
        } else if (ret == CHAR_CTRL_D) {
            // exit for a soft reset
            mp_hal_stdout_tx_str("\r\n");
            vstr_clear(MP_STATE_VM(repl_line));
            return PYEXEC_FORCED_EXIT;
        } else if (ret == CHAR_CTRL_E) {
            // paste mode
            mp_hal_stdout_tx_str("\r\npaste mode; Ctrl-C to cancel, Ctrl-D to finish\r\n=== ");
            vstr_reset(MP_STATE_VM(repl_line));
            repl.paste_mode = true;
            return 0;
        }

        if (ret < 0) {
            return 0;
        }

        if (!mp_repl_continue_with_input(vstr_null_terminated_str(MP_STATE_VM(repl_line)))) {
            goto exec;
        }

        vstr_add_byte(MP_STATE_VM(repl_line), '\n');
        repl.cont_line = true;
        readline_note_newline("... ");
        return 0;

    } else {

        if (ret == CHAR_CTRL_C) {
            // cancel everything
            mp_hal_stdout_tx_str("\r\n");
            repl.cont_line = false;
            goto input_restart;
        } else if (ret == CHAR_CTRL_D) {
            // stop entering compound statement
            goto exec;
        }

        if (ret < 0) {
            return 0;
        }

        if (mp_repl_continue_with_input(vstr_null_terminated_str(MP_STATE_VM(repl_line)))) {
            vstr_add_byte(MP_STATE_VM(repl_line), '\n');
            readline_note_newline("... ");
            return 0;
        }

    exec:;
        int ret = parse_compile_execute(MP_STATE_VM(repl_line), MP_PARSE_SINGLE_INPUT, EXEC_FLAG_ALLOW_DEBUGGING | EXEC_FLAG_IS_REPL | EXEC_FLAG_SOURCE_IS_VSTR);
        if (ret & PYEXEC_FORCED_EXIT) {
            return ret;
        }

    input_restart:
        vstr_reset(MP_STATE_VM(repl_line));
        repl.cont_line = false;
        repl.paste_mode = false;
        readline_init(MP_STATE_VM(repl_line), ">>> ");
        return 0;
    }
}

uint8_t pyexec_repl_active;
int pyexec_event_repl_process_char(int c) {
    pyexec_repl_active = 1;
    int res;
    if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
        res = pyexec_raw_repl_process_char(c);
    } else {
        res = pyexec_friendly_repl_process_char(c);
    }
    pyexec_repl_active = 0;
    return res;
}

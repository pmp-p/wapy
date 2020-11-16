/* http://github.com/pmp-p  target pym:/data/cross/wapy/cmod/common/embed.pym */
/*
  embed.c AUTO-GENERATED by /data/cross/wapy/wapy-lib/modgen/__main__.py
*/

// ======= STATIC HEADER ========

#include <string.h>
#include <stdio.h>
#include <stdlib.h> // for free()

#include "py/obj.h"
#include "py/runtime.h"

#ifndef STATIC
#define STATIC static
#endif


#define None mp_const_none
#define bytes(cstr) PyBytes_FromString(cstr)
#define PyMethodDef const mp_map_elem_t
#define PyModuleDef const mp_obj_module_t

#define mp_obj_get_double mp_obj_get_float
#define mp_obj_new_int_from_ptr mp_obj_new_int_from_ull

#define mp_obj_new_int_from_unsigned_long mp_obj_new_int_from_uint
#define unsigned_long unsigned long





// embed exports
void print(mp_obj_t str) {
    mp_obj_print(str, PRINT_STR);
    mp_obj_print(mp_obj_new_str_via_qstr("\n",1), PRINT_STR);
}

void
null_pointer_exception(void) {
    fprintf(stderr, "null pointer exception in function pointer call\n");
}

mp_obj_t
PyBytes_FromString(char *string){
    vstr_t vstr;
    vstr_init_len(&vstr, strlen(string));
    strcpy(vstr.buf, string);
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}

const char *nullbytes = "";
//static int orem_id = 0;


// =========== embedded header from .pym ============

// modgen
// python annotations describe the C types you need, argv is always a variable size array of mp_obj_t.
// glue code will do its best to do the conversion or init.
//

#ifndef wa_clock_gettime
    #define wa_clock_gettime(clockid, timespec) clock_gettime(clockid, timespec)
    #define wa_gettimeofday(timeval, tmz) gettimeofday(timeval, tmz)
    static struct timespec t_timespec;
    static struct timeval t_timeval;
#else
    extern struct timespec t_timespec;
    extern struct timeval t_timeval;
#endif


#if defined(__EMSCRIPTEN__) || defined(__WASM__)

    #define WAPY_VALUE (1)
    extern int VMFLAGS_IF;
    extern int show_os_loop(int state);
    extern int state_os_loop(int state);
#else
    #define WAPY_VALUE (0)
    int VMFLAGS_IF = 0;
    int show_os_loop(int state) {return 0;}
    int state_os_loop(int state) {return 0;}
    void emscripten_sleep(int t){}
#endif

#include "emscripten.h"

#include "py/emitglue.h"

#include "py/lexer.h"
#include "py/compile.h"

#include <time.h>
#include <sys/time.h>


#include "py/smallint.h"

char * cstr ;
size_t cstr_max=0;

extern mp_lexer_t* mp_lexer_new_from_file(const char *filename);

//#include <dlfcn.h>
//extern uintptr_t SDL_embed(uintptr_t *ptr);

#if MICROPY_PERSISTENT_CODE_SAVE
    extern void mp_raw_code_save_file(mp_raw_code_t *rc, const char *filename);
// Save .mpy file to file system
    int raw_code_save_file(mp_raw_code_t *rc, const char *filename) {  return 0; }
#endif

//  this one is used on MCU to run asyncio loop from repl  loop idle state

//  TODO: fix NO_NLR mode

#if NO_NLR
mp_obj_t execute_from_str(const char *str) {
      return (mp_obj_t)0;
}
#else
mp_obj_t execute_from_str(const char *str) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        qstr src_name = 1/*MP_QSTR_*/;
        mp_lexer_t *lex = mp_lexer_new_from_str_len(src_name, str, strlen(str), false);
        mp_parse_tree_t pt = mp_parse(lex, MP_PARSE_FILE_INPUT);
        mp_obj_t module_fun = mp_compile(&pt, src_name, false);
        mp_call_function_0(module_fun);
        nlr_pop();
        return 0;
    } else {
        // uncaught exception
        return (mp_obj_t)nlr.ret_val;
    }
}
#endif

STATIC void coropass(void) {
    const char sched[] =
        "__module__ = __import__('sys').modules.get('asyncio',None);"
        "__module__ = __module__ and __module__.get_event_loop().step()";

    const char *sched_ptr = &sched[0];
    execute_from_str( sched_ptr);
    MP_STATE_PORT(coro_call_counter++);
}

//  code trace

//  ==========================================

qstr trace_prev_file = 1/*MP_QSTR_*/;
qstr trace_file = 1/*MP_QSTR_*/;

size_t trace_prev_line;
size_t trace_line;

int trace_on;

//  corepy

//  ==========================================


static mp_obj_t * ffpy = NULL;
static mp_obj_t * ffpy_add = NULL;

void
pyv(mp_obj_t value) {
    if (ffpy_add)
        mp_call_function_n_kw((mp_obj_t *)ffpy_add, 1, 0, &value);
}

mp_obj_t
pycore(const char *fn) {
    uintptr_t __creturn__ = 0;
    qstr qfn ;
    qfn = qstr_from_strn(fn, strlen(fn));
    mp_obj_t qst = MP_OBJ_NEW_QSTR(qfn);

    // ------- method body (try/finally) -----
    fprintf(stderr,"FFYPY[%p->%s]\n", ffpy, fn );
    if (ffpy) {
        mp_call_function_n_kw((mp_obj_t *)ffpy, 1, 0, &qst);
    }
    return mp_obj_new_int_from_ptr(__creturn__);
}

//  finalizers

//  ==========================================


typedef struct _on_del_t {
    mp_obj_base_t base;
    mp_obj_t fun;
} on_del_t;

extern mp_obj_type_t mp_type_on_del;

STATIC mp_obj_t new_on_del(const mp_obj_type_t *type_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    on_del_t *o = m_new_obj_with_finaliser(on_del_t);
    o->base.type = &mp_type_on_del;
    o->fun = args[0];
    return o;
}

STATIC mp_obj_t on_del_del(mp_obj_t self_in) {
    return mp_call_function_0(((on_del_t *)self_in)->fun);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(on_del_del_obj, on_del_del);

STATIC void on_del_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
    if (dest[0] == MP_OBJ_NULL && attr == MP_QSTR___del__) {
        dest[0] = MP_OBJ_FROM_PTR(&on_del_del_obj);
        dest[1] = self_in;
    }
}

mp_obj_type_t mp_type_on_del = {
    {&mp_type_type},
    .name = MP_QSTR_on_del,
    .make_new = new_on_del,
    .attr = on_del_attr,
};


// end of embedded header
// =============================================================



STATIC mp_obj_t // void -> void
embed_set_io_buffer(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def set_io_buffer(ptr: int = 0, ptr_max: int = 0):
    // ('int', '0') => int = 0
    
        int ptr; // ('int', 'ptr', '0')
        if (argc>0) { ptr = mp_obj_get_int(argv[0]); } else { ptr = 0 ; }
    
    
        int ptr_max; // ('int', 'ptr_max', '0')
        if (argc>1) { ptr_max = mp_obj_get_int(argv[1]); } else { ptr_max = 0 ; }
    

    // ------- method body (try/finally) -----
    uintptr_t * addr;
    addr = (uintptr_t *)(uintptr_t)ptr;
    cstr = (char *)addr;
    cstr_max = (size_t)ptr_max;
    cstr[0]=0;
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_set_io_buffer_obj, 0, 2, embed_set_io_buffer);



STATIC mp_obj_t // void -> void
embed_run(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def run(runstr : const_char_p = "[]"):
    // ('const_char_p', '"[]"') => const char * = "[]"
    
        const char *runstr; // ('const char *', 'runstr', '"[]"')
        if (argc>0) { runstr = mp_obj_str_get_str(argv[0]); } else { runstr = mp_obj_new_str_via_qstr("[]",2); }
    

    // ------- method body (try/finally) -----
    size_t ln = strlen(runstr);
    if ( ln < cstr_max ) {
        strcpy(cstr,runstr);
    } else {
        fprintf(stderr, "buffer overrun in embed.run %zu >= %zu", ln, cstr_max);
    };
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_run_obj, 0, 1, embed_run);



STATIC mp_obj_t // void -> void
embed_disable_irq(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return

    // ------- method body (try/finally) -----
    VMFLAGS_IF--;
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_disable_irq_obj, 0, 0, embed_disable_irq);



STATIC mp_obj_t // void -> void
embed_enable_irq(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return

    // ------- method body (try/finally) -----
    VMFLAGS_IF++;
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_enable_irq_obj, 0, 0, embed_enable_irq);



STATIC mp_obj_t // _int_from_uint -> _int_from_uint
embed_FLAGS_IF(size_t argc, const mp_obj_t *argv) {
// opt: no finally _int_from_uint slot
    uint __creturn__ = 0;

    // ------- method body (try/finally) -----
    { __creturn__ = (uint)VMFLAGS_IF; goto lreturn__; };
lreturn__: return mp_obj_new_int_from_uint(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_FLAGS_IF_obj, 0, 0, embed_FLAGS_IF);



STATIC mp_obj_t // _int_from_uint -> _int_from_uint
embed_WAPY(size_t argc, const mp_obj_t *argv) {
// opt: no finally _int_from_uint slot
    uint __creturn__ = 0;

    // ------- method body (try/finally) -----
    { __creturn__ = (uint)WAPY_VALUE; goto lreturn__; };
lreturn__: return mp_obj_new_int_from_uint(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_WAPY_obj, 0, 0, embed_WAPY);



STATIC mp_obj_t // void -> void
embed_os_print(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def os_print( data : const_char_p = "{}" ) -> void :
    // ('const_char_p', '"{}"') => const char * = "{}"
    
        const char *data; // ('const char *', 'data', '"{}"')
        if (argc>0) { data = mp_obj_str_get_str(argv[0]); } else { data = mp_obj_new_str_via_qstr("{}",2); }
    

    // ------- method body (try/finally) -----
    fprintf( stdout , "%s\n" , data );
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_os_print_obj, 0, 1, embed_os_print);



STATIC mp_obj_t // void -> void
embed_os_write(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def os_write( data : const_char_p = "{}" ) -> void :
    // ('const_char_p', '"{}"') => const char * = "{}"
    
        const char *data; // ('const char *', 'data', '"{}"')
        if (argc>0) { data = mp_obj_str_get_str(argv[0]); } else { data = mp_obj_new_str_via_qstr("{}",2); }
    

    // ------- method body (try/finally) -----
    fprintf( stdout , "%s" , data );
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_os_write_obj, 0, 1, embed_os_write);



STATIC mp_obj_t // void -> void
embed_os_stderr(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def os_stderr( data : const_char_p = "" ) -> void :
    // ('const_char_p', '""') => const char * = ""
    
        const char *data; // ('const char *', 'data', '""')
        if (argc>0) { data = mp_obj_str_get_str(argv[0]); } else { data = mp_obj_new_str_via_qstr("",0); }
    

    // ------- method body (try/finally) -----
    fprintf( stderr, "embed.os_stderr(%s)\n", data );
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_os_stderr_obj, 0, 1, embed_os_stderr);



STATIC mp_obj_t // void -> void
embed_log(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def log( data : const_char_p = "" ) -> void :
    // ('const_char_p', '""') => const char * = ""
    
        const char *data; // ('const char *', 'data', '""')
        if (argc>0) { data = mp_obj_str_get_str(argv[0]); } else { data = mp_obj_new_str_via_qstr("",0); }
    

    // ------- method body (try/finally) -----
    fprintf( stderr, "%s\n", data );
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_log_obj, 0, 1, embed_log);



STATIC mp_obj_t // dict -> dict
embed_builtins_vars(size_t argc, const mp_obj_t *argv) {
// opt: no finally dict slot
    mp_obj_dict_t * __creturn__;
    // def builtins_vars(module_obj : mp_obj_t = None ) -> dict:
    // ('mp_obj_t', 'None') => mp_obj_t = None
    
        mp_obj_t module_obj; // ('mp_obj_t', 'module_obj', 'NULL')
        if (argc>0) { module_obj = (mp_obj_t)argv[0]; }
        else { module_obj = NULL ; }
    

    // ------- method body (try/finally) -----
    mp_obj_dict_t *mod_globals = mp_obj_module_get_globals(module_obj);
    { __creturn__ = (mp_obj_dict_t *)mod_globals; goto lreturn__; };
lreturn__: return (mp_obj_t)__creturn__ ;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_builtins_vars_obj, 0, 1, embed_builtins_vars);



STATIC mp_obj_t // int -> int
embed_os_state_loop(size_t argc, const mp_obj_t *argv) {
// opt: no finally int slot
    long __creturn__ = 0;
    // def os_state_loop(state:int = 0)->int:
    // ('int', '0') => int = 0
    
        int state; // ('int', 'state', '0')
        if (argc>0) { state = mp_obj_get_int(argv[0]); } else { state = 0 ; }
    

    // ------- method body (try/finally) -----
    { __creturn__ = (long)mp_obj_new_int( state_os_loop(state) ); goto lreturn__; };
lreturn__: return mp_obj_new_int(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_os_state_loop_obj, 0, 1, embed_os_state_loop);



STATIC mp_obj_t // void -> void
embed_os_showloop(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return

    // ------- method body (try/finally) -----
    fprintf(stderr,"will show begin/end for os loop\n");
    show_os_loop(1);
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_os_showloop_obj, 0, 0, embed_os_showloop);



STATIC mp_obj_t // void -> void
embed_os_hideloop(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return

    // ------- method body (try/finally) -----
    fprintf(stderr,"will hide begin/end for os loop\n");
    show_os_loop(0);
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_os_hideloop_obj, 0, 0, embed_os_hideloop);



STATIC mp_obj_t // void -> void
embed_os_hook(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return

    // ------- method body (try/finally) -----
    void (*void_ptr)(int) = MP_STATE_PORT(PyOS_InputHook);
    if ( void_ptr != NULL ) {
        printf("PyOS_InputHook %p TODO: allow py callback ptr\n", void_ptr);
    } else {
        printf("PyOS_InputHook undef TODO: allow py callback ptr\n");
        if ( !MP_STATE_PORT(coro_call_counter)) {
            MP_STATE_PORT(PyOS_InputHook) = &coropass;
            printf("coro task started\n");
            coropass();
        };
    };
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_os_hook_obj, 0, 0, embed_os_hook);



STATIC mp_obj_t // int -> int
embed_time_ns(size_t argc, const mp_obj_t *argv) {
// opt: no finally int slot
    long __creturn__ = 0;

    // ------- method body (try/finally) -----
    wa_clock_gettime(CLOCK_MONOTONIC, &t_timespec);
    unsigned long long ul = t_timespec.tv_sec * 1000000000 + t_timespec.tv_nsec;
    { __creturn__ = (long)mp_obj_new_int_from_ull(ul); goto lreturn__; };
lreturn__: return mp_obj_new_int(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_time_ns_obj, 0, 0, embed_time_ns);



STATIC mp_obj_t // int -> int
embed_time_ms(size_t argc, const mp_obj_t *argv) {
// opt: no finally int slot
    long __creturn__ = 0;

    // ------- method body (try/finally) -----
    wa_clock_gettime(CLOCK_MONOTONIC, &t_timespec);
    unsigned long long ul = t_timespec.tv_sec * 1000 + t_timespec.tv_nsec / 1000000;
    { __creturn__ = (long)mp_obj_new_int_from_ull(ul); goto lreturn__; };
lreturn__: return mp_obj_new_int(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_time_ms_obj, 0, 0, embed_time_ms);



STATIC mp_obj_t // int -> int
embed_ticks_add(size_t argc, const mp_obj_t *argv) {
// opt: no finally int slot
    long __creturn__ = 0;
    // def ticks_add(ticks : int=0, delta : int=0)->int:
    // ('int', '0') => int = 0
    
        int ticks; // ('int', 'ticks', '0')
        if (argc>0) { ticks = mp_obj_get_int(argv[0]); } else { ticks = 0 ; }
    
    
        int delta; // ('int', 'delta', '0')
        if (argc>1) { delta = mp_obj_get_int(argv[1]); } else { delta = 0 ; }
    

    // ------- method body (try/finally) -----
    { __creturn__ = (long)mp_obj_new_int_from_ull( ticks + delta ); goto lreturn__; };
lreturn__: return mp_obj_new_int(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_ticks_add_obj, 0, 2, embed_ticks_add);



STATIC mp_obj_t // int -> int
embed_ticks_diff(size_t argc, const mp_obj_t *argv) {
// opt: no finally int slot
    long __creturn__ = 0;
    // def ticks_diff(ticks : int=0, delta : int=0)->int:
    // ('int', '0') => int = 0
    
        int ticks; // ('int', 'ticks', '0')
        if (argc>0) { ticks = mp_obj_get_int(argv[0]); } else { ticks = 0 ; }
    
    
        int delta; // ('int', 'delta', '0')
        if (argc>1) { delta = mp_obj_get_int(argv[1]); } else { delta = 0 ; }
    

    // ------- method body (try/finally) -----
    { __creturn__ = (long)mp_obj_new_int_from_ull( ticks - delta ); goto lreturn__; };
lreturn__: return mp_obj_new_int(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_ticks_diff_obj, 0, 2, embed_ticks_diff);



STATIC mp_obj_t // void -> void
embed_sleep_ms(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def sleep_ms(ms : int =0) -> void :
    // ('int', '0') => int = 0
    
        int ms; // ('int', 'ms', '0')
        if (argc>0) { ms = mp_obj_get_int(argv[0]); } else { ms = 0 ; }
    

    // ------- method body (try/finally) -----
    emscripten_sleep(ms);
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_sleep_ms_obj, 0, 1, embed_sleep_ms);



STATIC mp_obj_t // void -> void
embed_sleep(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def sleep(s : float =0) -> void :
    // ('float', '0') => float = 0
    
        double s; // ('double', 's', '0')
        if (argc>0) { s = mp_obj_get_double(argv[0]); } else { s = 0 ; }
    

    // ------- method body (try/finally) -----
    emscripten_sleep( (int)(s*1000) );
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_sleep_obj, 0, 1, embed_sleep);



STATIC mp_obj_t // int -> int
embed_ticks_period(size_t argc, const mp_obj_t *argv) {
// opt: no finally int slot
    long __creturn__ = 0;

    // ------- method body (try/finally) -----
    { __creturn__ = (long)mp_obj_new_int_from_uint( MICROPY_PY_UTIME_TICKS_PERIOD ); goto lreturn__; };
lreturn__: return mp_obj_new_int(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_ticks_period_obj, 0, 0, embed_ticks_period);



STATIC mp_obj_t // bytes -> bytes
embed_os_read_useless(size_t argc, const mp_obj_t *argv) {
// opt: no finally bytes slot
    char * __creturn__ = (char *)nullbytes;

    // ------- method body (try/finally) -----
    static char buf[256];
    char *s = fgets(buf, sizeof(buf), stdin);
    if (!s) {
        buf[0]=0;
        fprintf(stderr,"embed.os_read EOF\n" );
    } else {
        int l = strlen(buf);
        if (buf[l - 1] == '\n') {
            if ( (l>1) && (buf[l - 2] == '\r') ) {
                buf[l - 2] = 0;
            } else {
                buf[l - 1] = 0;
            };
        } else {
            l++;
        };
        fprintf(stderr,"embed.os_read [%s]\n", buf );
    };
    { __creturn__ = (char *)bytes(buf); goto lreturn__; };
lreturn__: return PyBytes_FromString(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_os_read_useless_obj, 0, 0, embed_os_read_useless);



STATIC mp_obj_t // int -> int
embed_echosum1(size_t argc, const mp_obj_t *argv) {
// opt: no finally int slot
    long __creturn__ = 0;
    // def echosum1(num : int=0) -> int:
    // ('int', '0') => int = 0
    
        int num; // ('int', 'num', '0')
        if (argc>0) { num = mp_obj_get_int(argv[0]); } else { num = 0 ; }
    

    // ------- method body (try/finally) -----
    { __creturn__ = (long)MP_OBJ_NEW_SMALL_INT(num+1); goto lreturn__; };
lreturn__: return mp_obj_new_int(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_echosum1_obj, 0, 1, embed_echosum1);



STATIC mp_obj_t // void -> void
embed_callsome(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def callsome(fnptr : void_p=npe) -> void:
    // ('void_p', 'npe') => void * = NULL
    
        void * fnptr; // ('void *', 'fnptr', 'NULL')
        if (argc>0) { fnptr = (void *)argv[0]; }
        else { fnptr = NULL ; }
    

    // ------- method body (try/finally) -----
    void (*fn)() = fnptr;
    (*fn)();
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_callsome_obj, 0, 1, embed_callsome);



STATIC mp_obj_t // void -> void
embed_callpy(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def callpy(fn: const_char_p ="") -> void:
    // ('const_char_p', '""') => const char * = ""
    
        const char *fn; // ('const char *', 'fn', '""')
        if (argc>0) { fn = mp_obj_str_get_str(argv[0]); } else { fn = mp_obj_new_str_via_qstr("",0); }
    

    // ------- method body (try/finally) -----
    fprintf(stderr,"embed.callpy[%s]\n", fn );
    pycore(fn);
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_callpy_obj, 0, 1, embed_callpy);



STATIC mp_obj_t // _int_from_unsigned_long -> _int_from_unsigned_long
embed_hash_djb2(size_t argc, const mp_obj_t *argv) {
// opt: no finally _int_from_unsigned_long slot
    unsigned_long __creturn__ = 0;
    // def hash_djb2(cstr : const_char_p = "") -> _int_from_unsigned_long:
    // ('const_char_p', '""') => const char * = ""
    
        const char *cstr; // ('const char *', 'cstr', '""')
        if (argc>0) { cstr = mp_obj_str_get_str(argv[0]); } else { cstr = mp_obj_new_str_via_qstr("",0); }
    

    // ------- method body (try/finally) -----
        unsigned long hash = 5381;
        int c;
        while ((c = *cstr++)) {
            hash = ((hash << 5) + hash) + c;
        }
        { __creturn__ = (unsigned_long)hash % 0xFFFFFFFF; goto lreturn__; };
lreturn__: return mp_obj_new_int_from_unsigned_long(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_hash_djb2_obj, 0, 1, embed_hash_djb2);



STATIC mp_obj_t // int -> int
embed_show_trace(size_t argc, const mp_obj_t *argv) {
// opt: no finally int slot
    long __creturn__ = 0;

    // ------- method body (try/finally) -----
    trace_on = 1;
    fprintf(stderr,"TRACE[%s:%zu -> %s:%zu]\n", qstr_str(trace_prev_file), trace_prev_line, qstr_str(trace_file), trace_line);
    { __creturn__ = (long)trace_prev_line-1; goto lreturn__; };
lreturn__: return mp_obj_new_int(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_show_trace_obj, 0, 0, embed_show_trace);



STATIC mp_obj_t // _int_from_ptr -> _int_from_ptr
embed_address_of(size_t argc, const mp_obj_t *argv) {
// opt: no finally _int_from_ptr slot
    uintptr_t __creturn__ = 0;
    // def address_of(ptr : void_p = NULL ) -> _int_from_ptr:
    // ('void_p', 'NULL') => void * = NULL
    
        void * ptr; // ('void *', 'ptr', 'NULL')
        if (argc>0) { ptr = (void *)argv[0]; }
        else { ptr = NULL ; }
    

    // ------- method body (try/finally) -----
    uintptr_t * ptraddr = (uintptr_t *)ptr;
    uintptr_t ptrvalue = (uintptr_t)(void *)ptraddr;
    { __creturn__ = (uintptr_t)ptrvalue; goto lreturn__; };
lreturn__: return mp_obj_new_int_from_ptr(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_address_of_obj, 0, 1, embed_address_of);



STATIC mp_obj_t // void -> void
embed_set_ffpy(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def set_ffpy(fn: void_p) -> void:
    // ('void_p', None) => void * = NULL
    
        void * fn; // ('void *', 'fn', 'NULL')
        if (argc>0) { fn = (void *)argv[0]; }
        else { fn = NULL ; }
    

    // ------- method body (try/finally) -----
    fprintf(stderr,"embed.ffpy[%p]\n", fn );
    ffpy = (mp_obj_t *)fn;
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_set_ffpy_obj, 0, 1, embed_set_ffpy);



STATIC mp_obj_t // void -> void
embed_set_ffpy_add(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def set_ffpy_add(fn: void_p) -> void:
    // ('void_p', None) => void * = NULL
    
        void * fn; // ('void *', 'fn', 'NULL')
        if (argc>0) { fn = (void *)argv[0]; }
        else { fn = NULL ; }
    

    // ------- method body (try/finally) -----
    fprintf(stderr,"embed.ffpy[%p]\n", fn );
    ffpy_add = (mp_obj_t *)fn;
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_set_ffpy_add_obj, 0, 1, embed_set_ffpy_add);



STATIC mp_obj_t // _int_from_ptr -> _int_from_ptr
embed_corepy(size_t argc, const mp_obj_t *argv) {
// opt: no finally _int_from_ptr slot
    uintptr_t __creturn__ = 0;
    // def corepy(fn: const_char_p = "") -> _int_from_ptr:
    // ('const_char_p', '""') => const char * = ""
    
        const char *fn; // ('const char *', 'fn', '""')
        if (argc>0) { fn = mp_obj_str_get_str(argv[0]); } else { fn = mp_obj_new_str_via_qstr("",0); }
    

    // ------- method body (try/finally) -----
    fprintf(stderr,"embed.ffipy[%p(%s)]\n", ffpy, fn );
    if (ffpy) {
        mp_call_function_n_kw((mp_obj_t *)ffpy, 1, 0, &argv[0]);
    }
return mp_obj_new_int_from_ptr(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_corepy_obj, 0, 1, embed_corepy);



STATIC mp_obj_t // void -> void
embed_somecall(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return
    // def somecall(s:str='pouet'):
    // ('str', "'pouet'") => str = 'pouet'
    
        const char *s; // ('str', 's', "'pouet'")
        if (argc>0) { s = mp_obj_str_get_str(argv[0]); } else { s = mp_obj_new_str_via_qstr("pouet",5); }
    

    // ------- method body (try/finally) -----
    fprintf(stderr, "FPRINTF[%s]\n", mp_obj_str_get_str((char *)s) );
    print( (char *)s);
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(embed_somecall_obj, 0, 1, embed_somecall);



// global module dict :



STATIC const mp_map_elem_t embed_dict_table[] = {
// builtins
  {MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_embed) },
  {MP_OBJ_NEW_QSTR(MP_QSTR___file__), MP_OBJ_NEW_QSTR(MP_QSTR_flashrom) },

// extensions
    {MP_OBJ_NEW_QSTR(MP_QSTR_on_del), MP_ROM_PTR(&mp_type_on_del) },


// Classes : 


// __main__
    {MP_OBJ_NEW_QSTR(MP_QSTR_set_io_buffer), (mp_obj_t)&embed_set_io_buffer_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_run), (mp_obj_t)&embed_run_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_disable_irq), (mp_obj_t)&embed_disable_irq_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_enable_irq), (mp_obj_t)&embed_enable_irq_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_FLAGS_IF), (mp_obj_t)&embed_FLAGS_IF_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_WAPY), (mp_obj_t)&embed_WAPY_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_os_print), (mp_obj_t)&embed_os_print_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_os_write), (mp_obj_t)&embed_os_write_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_os_stderr), (mp_obj_t)&embed_os_stderr_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_log), (mp_obj_t)&embed_log_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_builtins_vars), (mp_obj_t)&embed_builtins_vars_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_os_state_loop), (mp_obj_t)&embed_os_state_loop_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_os_showloop), (mp_obj_t)&embed_os_showloop_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_os_hideloop), (mp_obj_t)&embed_os_hideloop_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_os_hook), (mp_obj_t)&embed_os_hook_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_time_ns), (mp_obj_t)&embed_time_ns_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_time_ms), (mp_obj_t)&embed_time_ms_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_ticks_add), (mp_obj_t)&embed_ticks_add_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_ticks_diff), (mp_obj_t)&embed_ticks_diff_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_sleep_ms), (mp_obj_t)&embed_sleep_ms_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_sleep), (mp_obj_t)&embed_sleep_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_ticks_period), (mp_obj_t)&embed_ticks_period_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_os_read_useless), (mp_obj_t)&embed_os_read_useless_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_echosum1), (mp_obj_t)&embed_echosum1_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_callsome), (mp_obj_t)&embed_callsome_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_callpy), (mp_obj_t)&embed_callpy_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_hash_djb2), (mp_obj_t)&embed_hash_djb2_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_show_trace), (mp_obj_t)&embed_show_trace_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_address_of), (mp_obj_t)&embed_address_of_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_set_ffpy), (mp_obj_t)&embed_set_ffpy_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_set_ffpy_add), (mp_obj_t)&embed_set_ffpy_add_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_corepy), (mp_obj_t)&embed_corepy_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_somecall), (mp_obj_t)&embed_somecall_obj },

//  {NULL, NULL, 0, NULL} // cpython
};

STATIC MP_DEFINE_CONST_DICT(embed_dict, embed_dict_table);



//const mp_obj_module_t STATIC
PyModuleDef module_embed = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&embed_dict,
};

// Register the module to make it available
MP_REGISTER_MODULE(MP_QSTR_embed, module_embed, MODULE_EMBED_ENABLED);


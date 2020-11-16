/* http://github.com/pmp-p  target pym:/data/cross/wapy/cmod/common/_zipfile.pym */
/*
  _zipfile.c AUTO-GENERATED by /data/cross/wapy/wapy-lib/modgen/__main__.py
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





// reuse embed exports
extern void print(mp_obj_t str);
extern void null_pointer_exception(void);
extern mp_obj_t PyBytes_FromString(char *string);
extern const char *nullbytes;

// =========== embedded header from .pym ============

#include "zip.h"

// end of embedded header
// =============================================================

typedef struct __zipfile_ZipFile_obj_t {
    mp_obj_base_t base;
    uint8_t hash;
    void * zbuf;
    size_t zbuf_size;
    struct zip_t * zip;
    char * path;
} _zipfile_ZipFile_obj_t;



const mp_obj_type_t _zipfile_ZipFile_type;  //forward decl

mp_obj_t
_zipfile_ZipFile_make_new( const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args ) {

    mp_arg_check_num(n_args, n_kw, 0, 0, true);

    _zipfile_ZipFile_obj_t *self = m_new_obj(_zipfile_ZipFile_obj_t);

    self->base.type = &_zipfile_ZipFile_type;

// self->hash = object_id++;
// printf("Object serial #%d\n", self->hash );

    self->hash = 0;
    self->zbuf = NULL;
    self->zbuf_size = 0;
    self->zip = None;
    self->path = NULL;

    return MP_OBJ_FROM_PTR(self);
}


void
_zipfile_ZipFile_print( const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind ) {
    // get a ptr to the C-struct of the object
    _zipfile_ZipFile_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // print the number
    mp_printf (print, "<_zipfile.ZipFile at 0x%p>", self);
}


//    Begin :  class ZipFile:



STATIC mp_obj_t // void -> void
_zipfile_ZipFile_open(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return

    _zipfile_ZipFile_obj_t *self = (_zipfile_ZipFile_obj_t *)MP_OBJ_TO_PTR(argv[0]);
    (void)self;

    // def open(name: const_char_p="")->void:
    // ('const_char_p', '""') => const char * = ""
    
        const char *name; // ('const char *', 'name', '""')
        if (argc>1) { name = mp_obj_str_get_str(argv[1]); } else { name = mp_obj_new_str_via_qstr("",0); }
    

    // ------- method body (try/finally) -----
    zip_entry_open(self->zip, name);
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(_zipfile_ZipFile_open_obj, 0, 2, _zipfile_ZipFile_open);



STATIC mp_obj_t // bytes -> bytes
_zipfile_ZipFile_read(size_t argc, const mp_obj_t *argv) {
    mp_obj_t __preturn__;
    char * __creturn__ = (char *)nullbytes;

    _zipfile_ZipFile_obj_t *self = (_zipfile_ZipFile_obj_t *)MP_OBJ_TO_PTR(argv[0]);
    (void)self;


    // ------- method body --------
    zip_entry_read(self->zip, &self->zbuf, &self->zbuf_size);
    zip_entry_close(self->zip);
    { //try:
        if (self->zbuf) {
            __creturn__ = (char *)self->zbuf;
        }
    { //finally:
        __preturn__ =  PyBytes_FromString(__creturn__);
        free(self->zbuf);
    }
    }
return __preturn__;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(_zipfile_ZipFile_read_obj, 0, 1, _zipfile_ZipFile_read);



STATIC mp_obj_t // void -> void
_zipfile_ZipFile_init(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return

    _zipfile_ZipFile_obj_t *self = (_zipfile_ZipFile_obj_t *)MP_OBJ_TO_PTR(argv[0]);
    (void)self;

    // def init(path : const_char_p="") -> void:
    // ('const_char_p', '""') => const char * = ""
    
        const char *path; // ('const char *', 'path', '""')
        if (argc>1) { path = mp_obj_str_get_str(argv[1]); } else { path = mp_obj_new_str_via_qstr("",0); }
    

    // ------- method body (try/finally) -----
    self->path = strdup(path);
    self->hash = strlen(path);
    printf("ZIPFILE[%d;%s]self\n", self->hash, self->path);
    self->zip = zip_open(self->path, 0, 'r');
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(_zipfile_ZipFile_init_obj, 0, 2, _zipfile_ZipFile_init);



STATIC mp_obj_t // bytes -> bytes
_zipfile_ZipFile_filename(size_t argc, const mp_obj_t *argv) {
// opt: no finally bytes slot
    char * __creturn__ = (char *)nullbytes;

    _zipfile_ZipFile_obj_t *self = (_zipfile_ZipFile_obj_t *)MP_OBJ_TO_PTR(argv[0]);
    (void)self;


    // ------- method body (try/finally) -----
    { __creturn__ = (char *)self->path; goto lreturn__; };
lreturn__: return PyBytes_FromString(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(_zipfile_ZipFile_filename_obj, 0, 1, _zipfile_ZipFile_filename);



STATIC mp_obj_t // void -> void
_zipfile_ZipFile_close(size_t argc, const mp_obj_t *argv) {
// opt: no finally void slot
// opt : void return

    _zipfile_ZipFile_obj_t *self = (_zipfile_ZipFile_obj_t *)MP_OBJ_TO_PTR(argv[0]);
    (void)self;


    // ------- method body (try/finally) -----
    if (self->path) {
        zip_close(self->zip);
        free(self->path);
    }
return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(_zipfile_ZipFile_close_obj, 0, 1, _zipfile_ZipFile_close);



STATIC mp_obj_t // int -> int
_zipfile_ZipFile_pouet(size_t argc, const mp_obj_t *argv) {
// opt: no finally int slot
    long __creturn__ = 0;

    _zipfile_ZipFile_obj_t *self = (_zipfile_ZipFile_obj_t *)MP_OBJ_TO_PTR(argv[0]);
    (void)self;

    // TODO: async : resume with go after last yield 

    // ------- method body (try/finally) -----
    if (1) {
        printf(" pouet arg0 %zu\n", argc );
        { __creturn__ = (long)42; goto lreturn__; };
    }
lreturn__: return mp_obj_new_int(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(_zipfile_ZipFile_pouet_obj, 0, 1, _zipfile_ZipFile_pouet);



STATIC mp_obj_t // int -> int
_zipfile_ZipFile_pouet2(size_t argc, const mp_obj_t *argv) {
// opt: no finally int slot
    long __creturn__ = 0;

    _zipfile_ZipFile_obj_t *self = (_zipfile_ZipFile_obj_t *)MP_OBJ_TO_PTR(argv[0]);
    (void)self;

    // TODO: async : resume with go after last yield 

    // ------- method body (try/finally) -----
    if (1) {
        if (2) {
            { __creturn__ = (long)43; goto lreturn__; };
        }
    }
lreturn__: return mp_obj_new_int(__creturn__);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(_zipfile_ZipFile_pouet2_obj, 0, 1, _zipfile_ZipFile_pouet2);



// ++++++++ class ZipFile interface +++++++



STATIC const mp_map_elem_t _zipfile_ZipFile_dict_table[] = {
    {MP_OBJ_NEW_QSTR(MP_QSTR_open), (mp_obj_t)&_zipfile_ZipFile_open_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&_zipfile_ZipFile_read_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&_zipfile_ZipFile_init_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_filename), (mp_obj_t)&_zipfile_ZipFile_filename_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_close), (mp_obj_t)&_zipfile_ZipFile_close_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_pouet), (mp_obj_t)&_zipfile_ZipFile_pouet_obj },
    {MP_OBJ_NEW_QSTR(MP_QSTR_pouet2), (mp_obj_t)&_zipfile_ZipFile_pouet2_obj },

//  {NULL, NULL, 0, NULL} // cpython
};

STATIC MP_DEFINE_CONST_DICT(_zipfile_ZipFile_dict, _zipfile_ZipFile_dict_table);



const mp_obj_type_t _zipfile_ZipFile_type = {

    // "inherit" the type "type"
    { &mp_type_type },

     // give it a name
    .name = MP_QSTR_ZipFile,

     // give it a print-function
    .print = _zipfile_ZipFile_print,

     // give it a constructor
    .make_new = _zipfile_ZipFile_make_new,

     // and its locals members
    .locals_dict = (mp_obj_dict_t*)&_zipfile_ZipFile_dict,
};

// End:  **************** class ZipFile ************



// global module dict :



STATIC const mp_map_elem_t _zipfile_dict_table[] = {
// builtins
  {MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR__zipfile) },
  {MP_OBJ_NEW_QSTR(MP_QSTR___file__), MP_OBJ_NEW_QSTR(MP_QSTR_flashrom) },

// extensions


// Classes : 

// ZipFile class
    {MP_OBJ_NEW_QSTR(MP_QSTR_ZipFile), (mp_obj_t)&_zipfile_ZipFile_type },


// __main__

//  {NULL, NULL, 0, NULL} // cpython
};

STATIC MP_DEFINE_CONST_DICT(_zipfile_dict, _zipfile_dict_table);



//const mp_obj_module_t STATIC
PyModuleDef module__zipfile = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&_zipfile_dict,
};

// Register the module to make it available
MP_REGISTER_MODULE(MP_QSTR__zipfile, module__zipfile, MODULE__ZIPFILE_ENABLED);


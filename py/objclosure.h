#ifndef MICROPY_INCLUDED_PY_OBJCLOSURE_H
#include "py/obj.h"
typedef struct _mp_obj_closure_t {
    mp_obj_base_t base;
    mp_obj_t fun;
    size_t n_closed;
    mp_obj_t closed[];
} mp_obj_closure_t;

#define MICROPY_INCLUDED_PY_OBJCLOSURE_H
#endif

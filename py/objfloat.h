#ifndef MICROPY_INCLUDED_PY_OBJFLOAT_H
#include "mpconfig.h" // for mp_float_t 
#include "py/obj.h"
#if MICROPY_PY_BUILTINS_FLOAT
typedef struct _mp_obj_float_t {
    mp_obj_base_t base;
    mp_float_t value;
} mp_obj_float_t;
#endif

#define MICROPY_INCLUDED_PY_OBJFLOAT_H
#endif

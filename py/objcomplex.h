#ifndef MICROPY_INCLUDED_PY_OBJCOMPLEX_H
#include "py/obj.h"
#if MICROPY_PY_BUILTINS_COMPLEX
typedef struct _mp_obj_complex_t {
    mp_obj_base_t base;
    mp_float_t real;
    mp_float_t imag;
} mp_obj_complex_t;
#endif

#define MICROPY_INCLUDED_PY_OBJCOMPLEX_H
#endif

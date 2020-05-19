#include <py/runtime.h>

NORETURN void abort_(void);

NORETURN void abort_(void) {
#if NO_NLR
    mp_raise_o( mp_obj_new_exception_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("abort() called")));
    for(;;){}
#else
    mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("abort() called"));
#endif
}

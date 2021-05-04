#ifndef WAPY_INCLUDED_REPL_H

extern struct wPyInterpreterState i_main ;

struct
wPyInterpreterState {
    char *shm_stdio ;
    char *shm_input_event_0;
};


#define WAPY_INCLUDED_REPL_H
#endif

#ifndef EMSCRIPTEN_H

#define EM_ASM_INT(...) 0
#define EM_ASM(...)

typedef void (*funcptr)();

#if __MAIN__

int emscripten_GetProcAddress(const char * name) {
    return 0;
}

int emscripten_run_preload_plugins(const char* file, char * onload, char * onerror) {
    return 0;
}

void emscripten_sleep(int s) {
}


static int emscripten_loop_run = 1;

void emscripten_cancel_main_loop(){
    emscripten_loop_run = 0;
};

void emscripten_set_main_loop(funcptr emfunc, int a, int b){
    while(emscripten_loop_run){
        emfunc();
    }
}

#else

extern int emscripten_GetProcAddress(const char * name);
extern int emscripten_run_preload_plugins(const char* file, char * onload, char * onerror);
extern void emscripten_sleep(int s);
//extern int emscripten_loop_run;
extern void emscripten_cancel_main_loop();
extern void emscripten_set_main_loop(funcptr emfunc, int a, int b);

#endif

#define EMSCRIPTEN_H

#endif

#include <stdlib.h>
#include <string.h>

#include "../wapy/wapy.c"

#include "vmsl/vmreg.c"


static int KPANIC = 1;
static int g_argc;
static char **g_argv; //[];



/*
#include "../wapy/core/ringbuf_o.h"
#include "../wapy/core/ringbuf_b.h"
*/


char **
copy_argv(int argc, char *argv[]) {
    // calculate the contiguous argv buffer size
    int length=0;

    size_t ptr_args = argc + 1;
    for (int i = 0; i < argc; i++) {
        length += (strlen(argv[i]) + 1);
    }
    char** new_argv = (char**)malloc((ptr_args) * sizeof(char*) + length);
    // copy argv into the contiguous buffer
    length = 0;
    for (int i = 0; i < argc; i++) {
        new_argv[i] = &(((char*)new_argv)[(ptr_args * sizeof(char*)) + length]);
        strcpy(new_argv[i], argv[i]);
        cdbg("52: argv[%d] = %s\n", i, argv[i]);
        length += (strlen(argv[i]) + 1);
    }
    // insert NULL terminating ptr at the end of the ptr array
    new_argv[ptr_args-1] = NULL;
    return (new_argv);
}



int io_encode_hex = 1;
//static int loops = 0;

#if __ARDUINO__
    #pragma message "NO MAIN"

#else

int
main(int argc, char *argv[]) {

    if (KPANIC!=0) {

        if (KPANIC>0) {

            g_argc = argc;
            g_argv = copy_argv(argc, argv);

            if (argc)
                io_encode_hex = !argc;

            KPANIC = 0;
            // init
            crash_point = &&VM_stackmess;

            cdbg("270: argv[0..%d] env=%s memory=%p\n", argc, getenv("WAPY"), shm_ptr() );

            if (argc) {
                printf("\nnode.js detected, running repl in infinite loop ...\n");
                // do not multiplex output
                cc = stdout;
            }

            #include "vmsl/vmwarmup.c"

            if (!argc) {
                // WASI syscall
                return 0;
            }

        } else {
            printf("\nno guru meditation [%d,%d]", getchar(), fgetc(cc) );
    // FIXME: exitcode
            return 1;
        }

    }



    while (!KPANIC) {

        #include "../wapy/vmsl/vm_loop.c"

// node.js
        if(argc) {
            fflush(stdout);
            fgets( io_stdin, MP_IO_SHM_SIZE, stdin );
            continue;
        }

// wasi
        break;
    }

    return 0;
} // main_iteration


#endif

//



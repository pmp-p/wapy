if (VMOP < VMOP_INIT) {
    puts("init");

    Py_Init();

    stack_initial = __builtin_frame_address(0); // could also use alloca(0)
    stack_max = (uintptr_t)stack_initial - stack_limit;


    VMOP = VMOP_INIT;

    entry_point[0]=JMP_NONE;
    exit_point[0]=JMP_NONE;
    come_from[0]=0;
    type_point[0]=0;

    for (int i=0; i<SYS_MAX_RECURSION; i++)
        mp_new_interpreter(&mpi_ctx, i, 0 , 0);

    // 0 hypervisor with no branching ( can use optimized original vm.c with no hard int )
    // 1 supervisor
    // 2 __main__

    mp_new_interpreter(&mpi_ctx, 1, 0, 2);

    // 2 has no parent for now, just back to OS
    mp_new_interpreter(&mpi_ctx, 2, 0, 0);
    ctx_current = 1;

    while ( mpi_ctx[ctx_current].childcare ) {
        ctx_current = (int)mpi_ctx[ctx_current].childcare;
    }

    fprintf(stdout,"running __main__ on pid=%d\n", ctx_current);

    return;
} // no continuation -> syscall

if (VMOP==VMOP_INIT) {
    puts("VMOP_INIT");

    VMOP = VMOP_WARMUP;
    show_os_loop(1);
    // help fix lack of vars()
    PyRun_SimpleString(
        "__dict__ = globals();"
        "import sys;"
        "import embed;"
        "import builtins;"
        "sys.path.extend(['/assets','/assets/packages']);"
        "import site_wapy;"
        "#\n"
    );
    emscripten_cancel_main_loop();
    emscripten_set_main_loop( main_loop_or_step, 0, 1);
}

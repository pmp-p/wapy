// TODO: that can't go inside a shared wasm lib

mp_import_stat_t
wasm_find_module(const char *modname) {
    if( access( modname, F_OK ) != -1 ) {
        struct stat stats;
        stat(modname, &stats);
        if (S_ISDIR(stats.st_mode))
            return MP_IMPORT_STAT_DIR;
        return MP_IMPORT_STAT_FILE;
    }

    int found = EM_ASM_INT({return vm.fsync.exists(UTF8ToString($0), true); }, modname ) ;

    if ( found>0 ) {
        int dl = EM_ASM_INT({return vm.fsync.open(UTF8ToString($0),UTF8ToString($0)); }, modname );

        if (found==1) {
            cdbg("19: wasm_find_module: DL FILE %s size=%d ", modname, dl);
            return MP_IMPORT_STAT_FILE;
        }
        if (found==2) {
            cdbg("23: wasm_find_module: IS DIR %s size=%d ", modname, dl);
            return MP_IMPORT_STAT_DIR;
        }
    }
    cdbg("404:wasm_find_module '%s' (%d)\n", modname, found);
    return MP_IMPORT_STAT_NO_EXIST;
}

#if !MICROPY_VFS
uint mp_import_stat(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return MP_IMPORT_STAT_DIR;
        } else if (S_ISREG(st.st_mode)) {
            return MP_IMPORT_STAT_FILE;
        }
    }
    return MP_IMPORT_STAT_NO_EXIST;
}
#endif


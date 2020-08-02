#!/bin/bash
reset

CC=${CC:-clang}

export PORT=$(pwd)/build-no_nlr/ports/wapy-wasm

if echo "$@" |grep -q clean
then
    make -C $PORT clean && rm -rf $PORT/build
    exit 0
fi

if echo $0|grep -q ash
then
    echo not to be sourced, use chmod +x on script
else
    if $CC -### 2>&1 | grep -q android
    then
        export PYTHONDONTWRITEBYTECODE=1
        for py in 8 7 6
        do
            if which python3.${py}
            then
                export PYTHON=python3.${py}
                break
            fi
        done
        echo Will use python $PYTHON
        echo

        export ROOT=$(dirname $(realpath "$0") )

        if cd $ROOT
        then

            echo
            echo Building ...

            export USER_C_MODULES=${ROOT}/cmod/wasm
            if PYTHONPATH=./wapy-lib $PYTHON -u -B -m modgen
            then
                echo ok
            else
                echo fail
                exit 1
            fi

#exit 1

            # future-fstrings-show now used to serve file via http
            #$PYTHON -u -B -m fstrings_helper micropython/link.cpy > micropython/ulink.py

            #export EMCC_FORCE_STDLIBS=libc,libc++abi,libc++,libdlmalloc
            export MOD="-DMODULE_EMBED_ENABLED=1 -DMODULE_EXAMPLE_ENABLED=1 -DMODULE_ZIPFILE_ENABLED=1"
            # -DMODULE_LVGL_ENABLED=1"

            if make NDK=/data/cross/pydk/android-sdk/ndk-bundle NDK_CC=${CC} NDK_LD=${LD} WAPY=1 NO_NLR=1 CWARN="-Wall" \
 USER_C_MODULES=${USER_C_MODULES} \
 CFLAGS_EXTRA="${MOD}" \
 FROZEN_MPY_DIR=${ROOT}/rom \
 FROZEN_DIR=flash \
 $@ -C $PORT
            then
                cp -vf $PORT/libwapy.so /data/cross/pydk-applications/pydk/prebuilt/armeabi-v7a/wapy/
            fi
        fi
    else
        echo "shell.xxx.sh android toolchain env not sourced !"
    fi
fi


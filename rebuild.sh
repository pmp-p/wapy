#!/bin/bash
reset
# BLOBS="--preload-file assets@/assets --preload-file micropython/lib@/lib"

rm -vf demos/wapy.wasm.map demos/wapy.wasm demos/wapy.js demos/wapy.data

if echo $0|grep -q bash
then
    echo not to be sourced, use chmod +x on script
else
    if command -v emcc
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
            if echo "$@"|grep -q fast
            then
                    rm -vf ./$1/lib*.* ./$1/build/main.o
            else
                echo "Cleaning up ( use 'fast' arg to to partial cleanup only )"
                emmake make clean
            fi

            echo
            echo Building ...

            export USER_C_MODULES=${ROOT}/cmod/wasm
            $PYTHON -u -B -m modgen

            # future-fstrings-show now used to serve file via http
            #$PYTHON -u -B -m fstrings_helper micropython/link.cpy > micropython/ulink.py

            #export EMCC_FORCE_STDLIBS=libc,libc++abi,libc++,libdlmalloc
            export MOD="-DMODULE_EMBED_ENABLED=1 -DMODULE_EXAMPLE_ENABLED=1 -DMODULE_LVGL_ENABLED=1"

            cp -vf /bin/true /data/cross/pydk/emsdk/upstream/emscripten/emstrip

            if emmake make WAPY=1 NO_NLR=1 STATIC=1 CWARN="-Wall" \
 JSFLAGS="-s WARN_ON_UNDEFINED_SYMBOLS=1" \
 USER_C_MODULES=${USER_C_MODULES} \
 CFLAGS_EXTRA="${MOD}" \
 FROZEN_MPY_DIR=${ROOT}/rom \
 FROZEN_DIR=flash \
 -C $@
            then
                mv -vf $1/wapy.* ${ROOT}/demos/
                #this is default emscripten's one
                rm ${ROOT}/demos/wapy.html
                cd ${ROOT}/demos
                PYTHONPATH=. $PYTHON -u -B -m pythons.js ${WEB:-"8000"}
            fi
        fi
    else
        echo "emsdk_env.sh not sourced !"
    fi
fi


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

            if PYTHONPATH=./wapy-lib $PYTHON -u -B -m modgen
            then
                echo ok
            else
                exit 1
            fi

            # future-fstrings-show now used to serve file via http
            #$PYTHON -u -B -m fstrings_helper micropython/link.cpy > micropython/ulink.py

            #export EMCC_FORCE_STDLIBS=libc,libc++abi,libc++,libdlmalloc
            export MOD="-DMODULE_EMBED_ENABLED=1 -DMODULE_EXAMPLE_ENABLED=1 -DMODULE_ZIPFILE_ENABLED=1"

            if [ -f pic-deps ]
            then
                echo pic-deps ok
            else
                # fake strip
                cp -vf /bin/true $EMSDK/upstream/emscripten/emstrip

                # on first time they are not built
                embuilder --pic build sdl2 vorbis
                touch pic-deps
            fi

export PORT=$(pwd)/build-no_nlr/ports/wapy-wasm

rm demos/libwapy.so ${PORT}/libwapy.* $PORT/libpanda3d.*

export LP3D=/data/cross/pydk/wasm/build-wasm/panda3dffi-wasm/lib
export DST=/data/cross/wapy/build-no_nlr/ports/wapy-wasm
export TMP=/data/cross/wapy/artmp

if false
then

    ar --output $TMP x ${LP3D}/libp3dtool.a
    ar --output $TMP x ${LP3D}/libp3dtoolconfig.a
    ar --output $TMP x ${LP3D}/libpandagles2.a
    ar --output $TMP x ${LP3D}/libp3openal_audio.a
    ar --output $TMP x ${LP3D}/libp3interrogatedb.a
    ar --output $TMP x ${LP3D}/libp3direct.a
    ar --output $TMP x ${LP3D}/libpandabullet.a
    ar --output $TMP x ${LP3D}/libpandaexpress.a
    ar --output $TMP x ${LP3D}/libpanda.a
    ar --output $TMP x ${LP3D}/libp3framework.a

    emar rcs $DST/libpanda3d.a $TMP/*.o
    file $DST/libpanda3d.a
fi



            if emmake make WAPY=1 NO_NLR=1 CWARN="-Wall" \
 JSFLAGS="-s WARN_ON_UNDEFINED_SYMBOLS=1" \
 USER_C_MODULES=${USER_C_MODULES} \
 CFLAGS_EXTRA="${MOD}" \
 FROZEN_MPY_DIR=${ROOT}/rom \
 FROZEN_DIR=flash \
 $@ -C $PORT
            then
                mv -vf ${PORT}/wapy.* ${ROOT}/demos/
                #this is default emscripten's one
                rm ${ROOT}/demos/wapy.html

# maintenance of lib folder
# --lz4
$PYTHON -u -B wapy/ports/wapy-wasm/file_packager.py wapy-lib.data --use-preload-plugins --no-heap-copy --separate-metadata\
 --js-output=wapy-lib.js \
 --preload ${ROOT}/demos/samples/libtest.wasm@/lib/libtest.so \
 --preload ${ROOT}/demos/assets/resources/hello.bmp@/hello.bmp \
 --preload ${ROOT}/sources.py/pythons@/assets/pythons \
 --preload /data/git/wapy-lib/pythons/fixes@/assets/pythons/fixes \
 --preload ${ROOT}/sources.py/wapy_wasm/wapy_wasm_site.py@/assets/site.py \
> packager.log
mv wapy-lib.* ${ROOT}/demos/

                mv ${PORT}/libwapy.wasm ../demos/libwapy.so
                mv ${PORT}/libwapy.wasm.map ../demos/

                mv ${PORT}/libsdl2.wasm ../demos/libsdl2.so
                mv ${PORT}/libsdl2.wasm.map ../demos/
                cd ${ROOT}/demos
                PYTHONPATH=. $PYTHON -u -B -m pythons.js ${WEB:-"8000"}

            fi
        fi
    else
        echo "emsdk_env.sh not sourced !"
    fi
fi


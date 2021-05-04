#PORTS=$(EM_CACHE)/asmjs/ports-builds
#FROZEN_MPY_DIR ?= modules
#FROZEN_DIR ?= flash


BASENAME=wapy
PROG=$(BASENAME).html
LIBPYTHON = lib$(BASENAME).a
CROSS = 0
# clang has slightly different options to GCC
#CLANG = 1

# compiler settings
ifdef CWARN
else
CFLAGS += $(CWARN) -Wall -Werror
endif

CFLAGS += -Wpointer-arith -Wuninitialized -ferror-limit=2


include ../../py/mkenv.mk

#EM_CACHE ?= $(HOME)/.emscripten_cache

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

ifdef EMSCRIPTEN
	ifdef WASM_FILE_API
# note that WASM_FILE_API will pull filesystem function into the wasm lib
		CPPFLAGS += -DWASM_FILE_API=1
		LD_LIB += -s FORCE_FILESYSTEM=1
		COPT += -s FORCE_FILESYSTEM=1
	endif

	CC = emcc
	LD = $(CC)

	ifeq ($(DEBUG), 1)
		OG = -O0 -g4
	else
		#OPTIM=1
		#DLO=1
		CPPFLAGS += -DNDEBUG
		OG = -Oz -g0
	endif


	ifdef LVGL
		CPPFLAGS = -DMICROPY_PY_LVGL=1
		JSFLAGS += -s USE_SDL=2
		CFLAGS_USERMOD += -DMICROPY_PY_LVGL=1 -Wno-unused-function -Wno-for-loop-analysis
		CPP += -DMICROPY_PY_LVGL=1
	endif

	#Debugging/Optimization
	WASM_FLAGS += $(OG) --source-map-base http://localhost:8000/
	LD_SHARED += $(OG) --source-map-base http://localhost:8000/

	# make clang++ Act like 'emcc' but for preprocessing use
	CPP = ${EMSCRIPTEN}/../bin/clang -E -Wno-\#pragma-messages -undef -D__CPP__ -D__EMSCRIPTEN__
	CPP += --sysroot $(EMSCRIPTEN)/system
	CPP += -include $(BUILD)/clang_predefs.h
	CPP += $(addprefix -isystem, $(shell env LC_ALL=C $(CC) $(JSFLAGS) $(CFLAGS_EXTRA) -E -x c++ /dev/null -v 2>&1 |sed -e '/^\#include <...>/,/^End of search/{ //!b };d'))

	CPPFLAGS += -D__WASM__ -DMICROPY_PY_FFI=1 -Wno-unused-variable
	CPP += -D__WASM__ -DMICROPY_PY_FFI=1
	CPPFLAGS += $(INC) -Wall -ansi -std=gnu11

	# NO FFI on upstream for now

else
	CPPFLAGS += $(INC) $(WARN) -DMICROPY_PY_FFI=1 -ansi -std=gnu11
	CPP += -D__CPP__  -D__EMSCRIPTEN__ -DMICROPY_PY_FFI=1
endif


CC += $(OG)
LD += $(OG)


# include py core make definitions
include ../../py/py.mk


SIZE = echo

INC += -I.
INC += -I../..
INC += -I$(BUILD)



#LD_SHARED = -Wl,-map,$@.map -Wl,-dead_strip -Wl,-no_pie
LD_SHARED += -fno-exceptions -fno-rtti





ifneq ($(FROZEN_DIR),)
# To use frozen source modules, put your .py files in a subdirectory (eg scripts/)
# and then invoke make with FROZEN_DIR=scripts (be sure to build from scratch).
CPPFLAGS += -DMICROPY_MODULE_FROZEN_STR
endif

ifneq ($(FROZEN_MPY_DIR),)
# To use frozen bytecode, put your .py files in a subdirectory (eg frozen/) and
# then invoke make with FROZEN_MPY_DIR=frozen (be sure to build from scratch).
# //for qstr.c

CPPFLAGS += -DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool
CPPFLAGS += -DMICROPY_MODULE_FROZEN_MPY
endif

# //for build/genhdr/qstr.i.last
# QSTR_GEN_EXTRA_CFLAGS=-DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool

MPY_CROSS_FLAGS += -mcache-lookup-bc

include ../wapy/wapy.mk


SRC_C = \
	../wapy/core/vfs.c \
	../wapy/core/ringbuf_b.c \
        ../wapy/core/ringbuf_o.c \
	../wapy/core/file.wapy.c \
	mod/modos.c \
	mod/modtime.c \
	mod/moduos_vfs.c \


LIB_SRC_C= $(addprefix lib/, \
	utils/stdout_helpers.c \
	utils/pyexec.c \
	utils/interrupt_char.c \
        mp-readline/readline.c \
	)

# now using user mod dir, SRC_MOD use USER_C_MODULES

ifdef LVGL
	LIB_SRC_C_EXTRA += $(addprefix lib/,\
    	lv_bindings/driver/SDL/SDL_monitor.c \
        lv_bindings/driver/SDL/SDL_mouse.c \
        lv_bindings/driver/SDL/modSDL.c \
        timeutils/timeutils.c \
	)
	CPPFLAGS += -Wno-unused-function -Wno-for-loop-analysis
endif


ifdef NDK
	SRC_C += ../wapy-wasm/main.c mod/modffi.c
else
# optionnal experimental FFI
	SRC_C+= \
		mod/modffi.c \
		mod/ffi/ffi.c \
		mod/ffi/types.c \
		mod/ffi/prep_cif.c
endif

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) qstr/objtype.c qstr/modbuiltins.c
SRC_QSTR += $(LIB_SRC_C)


# Append any auto-generated sources that are needed by sources listed in
# SRC_QSTR
SRC_QSTR_AUTO_DEPS += SRC_QSTR

OBJ = $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_MOD:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(LIB_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(LIB_SRC_C_EXTRA:.c=.o))


CPPFLAGS += -DNO_NLR=$(NO_NLR)
CFLAGS += $(CPPFLAGS) $(CFLAGS_EXTRA)


all: $(PROG)

include ../../py/mkrules.mk


# one day, maybe go via a *embeddable* static lib first  ?
LIBPYTHON = lib$(BASENAME)$(TARGET).a

#force preprocessor env to be created before qstr extraction
ifdef EMSCRIPTEN
$(BUILD)/clang_predefs.h:
	$(Q)mkdir -p $(dir $@)
	$(Q)emcc $(CFLAGS) $(JSFLAGS) -E -x c /dev/null -dM > $@

# Create `clang_predefs.h` as soon as possible, using a Makefile trick

Makefile: $(BUILD)/clang_predefs.h
endif



ifdef NDK
COPT +=
LD_PROG +=
WASM_FLAGS =
endif

ifndef NDK
COPT += --memory-init-file 0
COPT += -s NO_EXIT_RUNTIME=1
COPT += -s ALLOW_MEMORY_GROWTH=0  -s TOTAL_STACK=16777216
COPT += -s TOTAL_MEMORY=512MB
#	COPT+= -s "ENVIRONMENT=web"
#	COPT+= -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1
# compression and dlopen(wasm) can fail on chrom* ?
# -s LZ4=0
COPT += $(JSFLAGS) $(WASM_FLAGS) -s ASSERTIONS=2

LD_PROG += -s FORCE_FILESYSTEM=1 -s EXPORT_ALL=1
LD_PROG += -s EXPORTED_FUNCTIONS="['_main', '_shm_ptr', '_shm_get_ptr', '_show_os_loop']"
LD_PROG += -s ERROR_ON_UNDEFINED_SYMBOLS=1 -s LLD_REPORT_UNDEFINED=1


WASM_FLAGS += -s LZ4=1 -s WASM=1
WASM_FLAGS += -s ASSERTIONS=1 -s DEMANGLE_SUPPORT=1 -s DISABLE_EXCEPTION_CATCHING=1

# -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_OGG=1
# COPT += -s USE_BULLET=1 -s USE_FREETYPE=1
#	LD_EXTRA += -lbullet -lfreetype
THR_FLAGS=-s FETCH=1 -s USE_PTHREADS=0
endif


clean:
	$(RM) -rf $(BUILD) $(CLEAN_EXTRA) $(LIBPYTHON)
	$(shell rm $(BASENAME)/$(BASENAME).* || echo echo test data cleaned up)


lib-static: $(OBJ)
	$(ECHO) "Linking static $(LIBPYTHON)"
	$(Q)$(AR) rcs $(LIBPYTHON) $(OBJ)


lib-shared: $(OBJ)
	$(ECHO) "Linking shared lib$(BASENAME)$(TARGET).wasm"
	$(Q)$(LD) $(LD_LIB) $(LD_SHARED) -o lib$(BASENAME)$(TARGET).wasm $(OBJ) -ldl -lc -s FORCE_FILESYSTEM=1 --use-preload-plugins

lib-android: $(OBJ)
	$(ECHO) "Linking shared lib$(BASENAME)$(TARGET).so"
	$(Q)$(LD) $(LD_LIB) $(LD_SHARED) -o lib$(BASENAME)$(TARGET).so $(OBJ) $(LD_EXTRA)

lib-allshared: $(OBJ)
	$(ECHO) "Linking shared lib$(BASENAME)$(TARGET).wasm"
	$(Q)$(LD) $(LD_LIB) $(LD_SHARED) -o lib$(BASENAME)$(TARGET).wasm $(OBJ)
#
	$(ECHO) "Linking shared libsdl2.wasm"
	$(Q)$(LD) $(LD_LIB) $(LD_SHARED) -o libsdl2.wasm -lSDL2 -logg -lfreetype

libs: lib-static lib-shared

ifdef HALF
#============
LD_PROG += $(WASM_FLAGS) -s MAIN_MODULE=1
LD_LIB += $(WASM_FLAGS) -s SIDE_MODULE=1
CFLAGS += -fPIC

# lib-static <= beware to clean the .a could be linked in
$(PROG): lib-allshared
	$(ECHO) "Building shared executable $@"
	$(shell cp lib$(BASENAME)$(TARGET).wasm lib$(BASENAME)$(TARGET).so)
	$(shell cp libsdl2.wasm libsdl2.so)
	$(Q)$(CC) $(CFLAGS) $(INC) $(COPT) $(LD_PROG) $(THR_FLAGS) $(BLOBS)\
 -s BINARYEN_ASYNC_COMPILATION=1 --use-preload-plugins\
 -o $@ main.c -l$(BASENAME)$(TARGET) -lSDL2 -lGL -lEGL -lGLESv2 -ldl -lm
	$(shell mv $(BASENAME).* $(BASENAME)/)
#-L. lib$(BASENAME)$(TARGET).a
endif

ifdef SHARED
#============
LD_PROG += $(WASM_FLAGS) -s MAIN_MODULE=1
LD_LIB += $(WASM_FLAGS) -s SIDE_MODULE=1
CFLAGS += -fPIC

$(PROG): lib-static lib-allshared
	$(ECHO) "Building shared executable $@"
	$(shell cp lib$(BASENAME)$(TARGET).wasm lib$(BASENAME)$(TARGET).so)
	$(shell cp libsdl2.wasm libsdl2.so)
	$(Q)$(CC) $(CFLAGS) $(INC) $(COPT) $(LD_PROG) $(THR_FLAGS) $(BLOBS)\
 -s BINARYEN_ASYNC_COMPILATION=1 --use-preload-plugins\
 -o $@ main.c -L. -lwapy -lsdl2 -ldl -lm
	$(shell mv $(BASENAME).* $(BASENAME)/)

endif


ifdef STATIC_NOPANDA
# ============ static + pic was SERIOUSLY BROKEN until clang 11 ==========
# but this is the good one
CFLAGS += -fPIC
LD_PROG += $(CFLAGS) $(WASM_FLAGS) -s MAIN_MODULE=1 -s LZ4=1
LD_LIB += $(WASM_FLAGS) -s LINKABLE=1 -s EXPORT_ALL=1


$(PROG): lib-static
	$(ECHO) "Building static executable $@"
	$(Q)$(CC) $(INC) $(COPT) $(LD_PROG) $(THR_FLAGS) \
 -o $@ main.c $(OBJ) $(BLOBS) -ldl -lm $(LD_EXTRA)
	$(shell mv $(BASENAME).* $(BASENAME)/)
# ============================================================
endif



ifdef STATIC
# ============ static + pic was SERIOUSLY BROKEN until clang 11 ==========
# but this is the good one
CFLAGS += -fPIC
LD_PROG += $(CFLAGS) $(WASM_FLAGS) -s MAIN_MODULE=1 -s LZ4=1
LD_LIB += $(WASM_FLAGS) -s LINKABLE=1 -s EXPORT_ALL=1

#LD_EXTRA += -lbullet $(LnkP3D)

$(PROG): lib-static
	$(ECHO) "Building static executable $@"
	$(Q)$(CC) $(INC) $(COPT) $(LD_PROG) $(THR_FLAGS) \
 -o $@ main.c  lib$(BASENAME)$(TARGET).a $(BLOBS) -s USE_WEBGL2=1 -ldl -lm $(LD_EXTRA)
	$(shell mv $(BASENAME).* $(BASENAME)/)
# ============================================================
endif



ifdef NDK
# ============ static + pic was SERIOUSLY BROKEN until clang 11 ==========
# but this is the good one
LD_PROG += $(CFLAGS)
CFLAGS += -fPIC
LD_LIB =

$(PROG): lib-android
	$(ECHO) "Building static executable $@"
	$(CC) $(INC) $(COPT) $(LD_PROG) $(THR_FLAGS) \
 -o $@ main.c -L. -lwapy $(BLOBS) -lEGL -lGLESv2 -ldl -lm $(LD_EXTRA)
	$(shell mv $(BASENAME).* $(BASENAME)/)
# ============================================================
endif



check:
	$(ECHO)
	$(ECHO) "============================================================"
	$(ECHO) "CC=[$(CC)]"
	$(ECHO) "CPP=[$(CPP)]"
	$(ECHO) "COPT=[$(COPT)]"
	$(ECHO) JSFLAGS=$(JSFLAGS)
	$(ECHO) CPPFLAGS=$(CPPFLAGS)
	$(ECHO)
	$(ECHO) CXX=$(CXX)
	$(ECHO) AS=$(AS)
	$(ECHO) LD=$(LD)
	$(ECHO) OBJCOPY=$(OBJCOPY)
	$(ECHO) SIZE=$(SIZE)
	$(ECHO) STRIP=$(STRIP)
	$(ECHO) AR=$(AR)
	$(ECHO)
#emscripten specifics
	$(ECHO) EMSDK=$(EMSDK)
	$(ECHO) UPSTREAM=$(UPSTREAM)
	$(ECHO) EMSCRIPTEN=$(EMSCRIPTEN)
	$(ECHO) EMSDK_NODE=$(EMSDK_NODE)
	$(ECHO) EMSCRIPTEN_TOOLS=$(EMSCRIPTEN_TOOLS)
	$(ECHO) EM_CONFIG=$(EM_CONFIG)
	$(ECHO) EMMAKEN_COMPILER=$(EMMAKEN_COMPILER)
	$(ECHO) EMMAKEN_CFLAGS=$(EMMAKEN_CFLAGS)
	$(ECHO) EMCC_FORCE_STDLIBS=$(EMCC_FORCE_STDLIBS)
	$(ECHO) EM_CACHE=$(EM_CACHE)
	$(ECHO) EM_CONFIG=$(EM_CONFIG)
	$(shell env|grep ^EM)
	$(ECHO)
	$(shell ${EMSCRIPTEN}/../bin/clang -v 2>&1|grep ^clang )
	#terminate on error so python webserver won't start

#PORTS=$(EM_CACHE)/asmjs/ports-builds
#FROZEN_MPY_DIR ?= modules
#FROZEN_DIR ?= flash


BASENAME=wapy
PROG=$(BASENAME).html
LIBPYTHON = lib$(BASENAME).a
CROSS = 0

# clang has slightly different options to GCC
CLANG = 1

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

CPPFLAGS += -D__WASM__=1 -DNO_NLR=1 -Wno-unused-variable
CC=clang -D__WASM__=1 -DNO_NLR=1
CPP=clang -E -D__CPP__ -D__WASM__=1 -DNO_NLR=1
CPPFLAGS += $(INC) $(WARN) -ansi -std=gnu11 



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

ifdef FFI
# optionnal experimental FFI
SRC_C+= \
	mod/modffi.c \
	mod/ffi/ffi.c \
	mod/ffi/types.c \
	mod/ffi/prep_cif.c	
endif

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



clean:
	$(RM) -rf $(BUILD) $(CLEAN_EXTRA) $(LIBPYTHON)
	$(shell rm $(BASENAME)/$(BASENAME).* || echo echo test data cleaned up)


COPT += $(JSFLAGS) $(WASM_FLAGS)


lib-static: $(OBJ)
	$(ECHO) "Linking static $(LIBPYTHON)"
	$(Q)$(AR) rcs $(LIBPYTHON) $(OBJ)

lib-shared: $(OBJ)	
	$(ECHO) "Linking shared lib$(BASENAME)$(TARGET).wasm"
	$(Q)$(LD) $(LD_SHARED) $(LD_LIB) -o lib$(BASENAME)$(TARGET).wasm $(OBJ) -ldl -lc

libs: lib-static lib-shared

# ============ static + pic was SERIOUSLY BROKEN until clang 11 ==========
# but this is the good one
LD_PROG += $(CFLAGS) $(WASM_FLAGS) 
LD_LIB += $(WASM_FLAGS)
CFLAGS += -fPIC

$(PROG): lib-static
	$(ECHO) "Building static executable $@"
	$(Q)$(CC) $(INC) $(COPT) $(LD_PROG) $(THR_FLAGS) \
 -o $@ main.c  lib$(BASENAME)$(TARGET).a  -ldl -lm $(BLOBS) $(PF)
	$(shell mv $(BASENAME).* $(BASENAME)/)


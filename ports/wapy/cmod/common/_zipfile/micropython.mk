
_ZIPFILE_MOD_DIR := $(USERMOD_DIR)

# Add all C files to SRC_USERMOD.
SRC_USERMOD += $(wildcard $(_ZIPFILE_MOD_DIR)/*.c)

# add module folder to include paths if needed
CFLAGS_USERMOD += -I$(_ZIPFILE_MOD_DIR)

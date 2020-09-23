
ifdef NDK
	CC=$(NDK_CC) -march=armv7-a -mthumb --target=armv7-none-linux-androideabi19 --gcc-toolchain=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64 --sysroot=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/sysroot -DANDROID -fdata-sections -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -D_FORTIFY_SOURCE=2

	LD=$(NDK_LD) -L$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/arm-linux-androideabi/19
	CFLAGS +=  -DANDROID_PLATFORM=android-19 -DAPP_PLATFORM=android-19 -D__ANDROID_API__=19
	LD_EXTRA += -fuse-ld=lld -L/data/cross/pydk/aosp/apkroot-armeabi-v7a/usr/lib -lffi -ldl -lm -lc -landroid
	INC += -I/data/cross/pydk/aosp/apkroot-armeabi-v7a/usr/include -I../wapy/stubs
	OG = -O0 -g3
	LD_SHARED = -shared
else
	LD_EXTRA += -s FORCE_FILESYSTEM=1 --use-preload-plugins
	LD_EXTRA += -L/data/cross/pydk/wasm/apkroot-wasm/usr/lib -lSDL2 -logg
endif


# py object files
PY_CORE_O_BASENAME = $(addprefix py/,\
	mpstate.o \
	malloc.o \
	gc.o \
	pystack.o \
	qstr.o \
	vstr.o \
	mpprint.o \
	unicode.o \
	mpz.o \
	reader.o \
	lexer.o \
	parse.o \
	scope.o \
	compile.o \
	emitcommon.o \
	emitbc.o \
	asmbase.o \
	asmx64.o \
	emitnx64.o \
	asmx86.o \
	emitnx86.o \
	asmthumb.o \
	emitnthumb.o \
	emitinlinethumb.o \
	asmarm.o \
	emitnarm.o \
	asmxtensa.o \
	emitnxtensa.o \
	emitinlinextensa.o \
	emitnxtensawin.o \
	formatfloat.o \
	parsenumbase.o \
	parsenum.o \
	emitglue.o \
	persistentcode.o \
	../ports/wapy/runtime_no_nlr.o \
	runtime_utils.o \
	scheduler.o \
	nativeglue.o \
	pairheap.o \
	ringbuf.o \
	stackctrl.o \
	../ports/wapy/argcheck_no_nlr.o \
	warning.o \
	profile.o \
	map.o \
	obj.o \
	../ports/wapy/objarray_no_nlr.o \
	objattrtuple.o \
	objbool.o \
	objboundmeth.o \
	objcell.o \
	objclosure.o \
	objcomplex.o \
	objdeque.o \
	../ports/wapy/objdict_no_nlr.o \
	objenumerate.o \
	objexcept.o \
	objfilter.o \
	objfloat.o \
	../ports/wapy/objfun_no_nlr.o \
	objgenerator.o \
	objgetitemiter.o \
	objint.o \
	objint_longlong.o \
	objint_mpz.o \
	../ports/wapy/objlist_no_nlr.o \
	objmap.o \
	objmodule.o \
	objobject.o \
	objpolyiter.o \
	objproperty.o \
	objnone.o \
	objnamedtuple.o \
	objrange.o \
	objreversed.o \
	objset.o \
	objsingleton.o \
	objslice.o \
	../ports/wapy/objstr_no_nlr.o \
	objstrunicode.o \
	objstringio.o \
	objtuple.o \
	objtype.o \
	objzip.o \
	opmethods.o \
	sequence.o \
	stream.o \
	binary.o \
	../ports/wapy/builtinimport_no_nlr.o \
	builtinevex.o \
	builtinhelp.o \
	modarray.o \
	../ports/wapy/modbuiltins_no_nlr.o \
	modcollections.o \
	modgc.o \
	modio.o \
	modmath.o \
	modcmath.o \
	modmicropython.o \
	modstruct.o \
	modsys.o \
	moduerrno.o \
	modthread.o \
	vm.o \
	bc.o \
	showbc.o \
	repl.o \
	smallint.o \
	frozenmod.o \
	)


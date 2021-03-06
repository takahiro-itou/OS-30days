
##
##    List of Files.
##

BOOTPACK_OBJS       =  BootPack.obj FontHankaku.obj Func.obj        \
                       Console.obj DscTbl.obj Fifo.obj File.obj     \
                       Graphic.obj Int.obj KeyBoard.obj             \
                       Memory.obj  Mouse.obj MultiTask.obj          \
                       Sheet.obj Timer.obj Window.obj               \
                       ../Common/stdio.obj                          \
                       ../Common/string.obj
KERNEL_IMAGE        =  haribote.sys

##
##    Commands.
##

AS          =  /usr/bin/as
CAT         =  /bin/cat
CP          =  /bin/cp
DD          =  /bin/dd
LD          =  /usr/bin/ld
OBJ2HRB     =  ../Tools/obj2hrb

##
##    Targets.
##

.PHONY      :  all  clean  cleanall  cleanobj
.SUFFIXES   :  .obj   .s

all         :  ${KERNEL_IMAGE}

clean       :  cleanobj
	${RM}  -f  ${KERNEL_IMAGE}

cleanall :  clean
	${RM}  -f                           \
        FontHankaku.o FontHankaku.c

cleanobj :
	${RM}  -f  AsmHead.bin              \
        AsmHead.obj AsmHead.lst         \
        ${BOOTPACK_OBJS}                \
        *.lst  *.map

##
##    Compile and Link Flags.
##

ASFLAGS     =  -march=i386  --32
CFLAGS      =  -march=i386  -m32  -fno-pie  -nostdlib  -O2
CCVERBOSE   =

##
##    Build.
##

${KERNEL_IMAGE} :  AsmHead.bin BootPack.hrb
	${CAT}  AsmHead.bin  BootPack.hrb  >  $@

AsmHead.bin     :  AsmHead.obj
	${LD}  -T ${@:%.bin=%.ls}  -o $@  $<

BootPack.hrb    :  ${BOOTPACK_OBJS}
	${LD}  -T ${@:%.hrb=%.ls}  -o $@    \
        -Map=${@:%.hrb=%.map}           \
        ${BOOTPACK_OBJS}

FontHankaku.c   :  hankaku.txt GenerateFontDef.awk Makefile
	/usr/bin/gawk  -f GenerateFontDef.awk   \
        < $<  > $@

##
##    Suffix Rules.
##

.s.obj  :
	${AS}  -o $@  \
        -a=${@:%.obj=%.lst}         \
        ${ASFLAGS}  $<

.c.obj  :
	${CC}  -o $@  \
        -Wa,-a=${@:%.obj=%.lst}     \
        ${CCVERBOSE}  ${CFLAGS}  -c  $<

##
##    Dependencies
##

AsmHead.bin     :  Makefile AsmHead.ls
BootPack.hrb    :  Makefile BootPack.ls
BootPack.obj    :  Makefile BootPack.h
Console.obj     :  Makefile BootPack.h
DscTbl.obj      :  Makefile BootPack.h
Fifo.obj        :  Makefile BootPack.h
File.obj        :  Makefile BootPack.h
Graphic.obj     :  Makefile BootPack.h
Int.obj         :  Makefile BootPack.h
KeyBoard.obj    :  Makefile BootPack.h
Memory.obj      :  Makefile BootPack.h
Mouse.obj       :  Makefile BootPack.h
MultiTask.obj   :  Makefile BootPack.h
Sheet.obj       :  Makefile BootPack.h
Timer.obj       :  Makefile BootPack.h
Window.obj      :  Makefile BootPack.h

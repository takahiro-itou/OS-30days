
##
##    List of Files.
##

BOOTPACK_OBJS       =  BootPack.obj FontHankaku.obj Func.obj    \
                       ../Common/stdio.obj
KERNEL_IMAGE        =  haribote.sys

##
##    Commands.
##

AS      =  /usr/bin/as
CAT     =  /bin/cat
CP      =  /bin/cp
DD      =  /bin/dd
LD      =  /usr/bin/ld

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
        Func.obj Func.lst

##
##    Compile and Link Flags.
##

ASFLAGS     =  -march=i386  --32
CFLAGS      =  -march=i386  -m32  -fno-pie  -nostdlib
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
        ${BOOTPACK_OBJS}

FontHankaku.c   :  hankaku.txt GenerateFontDef.awk Makefile
	/usr/bin/gawk  -f GenerateFontDef.awk   \
        < $<  > $@

##
##    Suffix Rules.
##

.s.obj  :
	${AS}  -o $@  -a=${@:%.obj=%.lst}  ${ASFLAGS}  $<

.c.obj  :
	${CC}  -o $@  ${CCVERBOSE}  ${CFLAGS}  -c  $<

##
##    Dependencies
##

AsmHead.bin     :  Makefile AsmHead.ls
BootPack.hrb    :  Makefile BootPack.ls

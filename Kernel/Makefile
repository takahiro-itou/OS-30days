
##
##    List of Files.
##

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

cleanobj :
	${RM}  -f  AsmHead.bin  AsmHead.obj AsmHead.lst
	${RM}  -f  BootPack.obj
	${RM}  -f  Func.obj Func.lst

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

BootPack.hrb    :  BootPack.obj Func.obj
	${LD}  -T ${@:%.hrb=%.ls}  -o $@    \
        BootPack.obj Func.obj

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

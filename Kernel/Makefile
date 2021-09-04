
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
	${RM}  -f  AsmHead.bin  AsmHead.obj
	${RM}  -f  BootPack.obj

##
##    Compile and Link Flags.
##

ASFLAGS     =  -march=i386  --32

##
##    Build.
##

${KERNEL_IMAGE} :  AsmHead.bin BootPack.hrb
	${CAT}  AsmHead.bin  BootPack.hrb  >  $@

AsmHead.bin     :  AsmHead.obj
	${LD}  -T LinkerScript  -o $@  $<

BootPack.hrb    :  BootPack.obj
	${LD}  -o $@  -e HariMain  --oformat=binary  $<

##
##    Suffix Rules.
##

.s.obj  :
	${AS}  -o $@  -a=${@:%.o=%.lst}  ${ASFLAGS}  $<

.c.obj  :
	${CC}  -o $@  -c  -nostdlib  $<

##
##    Dependencies
##

AsmHead.bin     :  Makefile LinkerScript
BootPack.hrb    :  Makefile LinkerScript

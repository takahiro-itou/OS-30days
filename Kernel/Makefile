
##
##    List of Files.
##

KERNEL_IMAGE        =  haribote.sys

##
##    Commands.
##

AS      =  /usr/bin/as
CP      =  /bin/cp
DD      =  /bin/dd
LD      =  /usr/bin/ld

##
##    Targets.
##

.PHONY      :  all  clean  cleanall  cleanobj
.SUFFIXES   :  .o   .s

all         :  ${KERNEL_IMAGE}

clean       :  cleanobj
	${RM}  -f  ${KERNEL_IMAGE}

cleanall :  clean

cleanobj :
	${RM}  -f  ${KERNEL_IMAGE:%.sys=%.o}
	${RM}  -f  ${KERNEL_IMAGE:%.sys=%.lst}

##
##    Compile and Link Flags.
##

ASFLAGS     =  -march=i386  --32

##
##    Build.
##

${KERNEL_IMAGE} : ${KERNEL_IMAGE:%.sys=%.o}
	${LD}  -T LinkerScript  -o $@  $<

##
##    Suffix Rules.
##

.s.o  :
	${AS}  -o $@  -a=${@:%.o=%.lst}  ${ASFLAGS}  $<

##
##    Dependencies
##

${KERNEL_IMAGE} : Makefile  LinkerScript

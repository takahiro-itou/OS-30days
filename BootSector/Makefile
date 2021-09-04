
##
##    List of Files.
##

IPLBIN_IMAGE        =  Ipl.bin

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
.SUFFIXES   :  .obj   .s

all         :  ${IPLBIN_IMAGE}

clean       :  cleanobj
	${RM}  -f  ${IPLBIN_IMAGE}

cleanall :  clean

cleanobj :
	${RM}  -f  ${IPLBIN_IMAGE:%.bin=%.obj}
	${RM}  -f  ${IPLBIN_IMAGE:%.bin=%.lst}

##
##    Compile and Link Flags.
##

ASFLAGS     =  -march=i386  --32

##
##    Build.
##

${IPLBIN_IMAGE} : ${IPLBIN_IMAGE:%.bin=%.obj}
	${LD}  -T LinkerScript  -o $@  $<

##
##    Suffix Rules.
##

.s.obj  :
	${AS}  -o $@  -a=${@:%.obj=%.lst}  ${ASFLAGS}  $<

##
##    Dependencies
##

${IPLBIN_IMAGE} : Makefile  LinkerScript

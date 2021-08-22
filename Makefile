
##
##    List of Files.
##

TARGET_IMAGE        =  Haribote.img

##
##    Commands.
##

AS      =  /usr/bin/as
CP      =  /bin/cp
LD      =  /usr/bin/ld

##
##    Targets.
##

.PHONY  :  all  clean  cleanall  cleanobj

all      :  ${TARGET_IMAGE}

clean    :  cleanobj
	${RM}  -f  ${TARGET_IMAGE}

cleanall :  clean
	${RM}  -f  ${TARGET_IMAGE:%.img=%.lst}

cleanobj :
	${RM}  -f  ${TARGET_IMAGE:%.img=%.o}

##
##    Compile and Link Flags.
##

ASFLAGS     =  -march=i386  --32

##
##    Build.
##

${TARGET_IMAGE} : ${TARGET_IMAGE:%.img=%.o}
	${LD}  -T LinkerScript  -o $@  $^

##
##    Suffix Rules.
##

.S.o  :
	${AS}  -o $@  ${ASFLAGS}  $^

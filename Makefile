
##
##    List of Files.
##

TARGET_IMAGE        =  Haribote.img
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
.SUFFIXES   :  .o   .s

all         :  ${TARGET_IMAGE}

clean       :  cleanobj
	${RM}  -f  ${TARGET_IMAGE}  ${IPLBIN_IMAGE}

cleanall :  clean
	${RM}  -f  ${TARGET_IMAGE:%.img=%.lst}

cleanobj :
	${RM}  -f  ${IPLBIN_IMAGE:%.bin=%.o}
	${RM}  -f  ${TARGET_IMAGE:%.img=%.o}

##
##    Compile and Link Flags.
##

ASFLAGS     =  -march=i386  --32

##
##    Build.
##

${TARGET_IMAGE} : ${IPLBIN_IMAGE}
	mformat  -f 1440  -C  -B ${IPLBIN_IMAGE}  -i $@  ::
	${DD}  if=${IPLBIN_IMAGE}  bs=512  count=1  of=$@  conv=notrunc

${IPLBIN_IMAGE} : ${IPLBIN_IMAGE:%.bin=%.o}
	${LD}  -T LinkerScript  -o $@  $^

##
##    Suffix Rules.
##

.s.o  :
	${AS}  -o $@  -a=${@:%.o=%.lst}  ${ASFLAGS}  $^

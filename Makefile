
##
##    List of Sub Directory.
##

SUBDIRS  =  BootSector  Kernel  ApiLib  App

##
##    List of Files.
##

TARGET_IMAGE    =  Haribote.img
IPLBIN_IMAGE    =  BootSector/Ipl.bin
KERNEL_IMAGE    =  Kernel/haribote.sys

APPLICATIONS    =  \
        App/a/a.hrb                 \
        App/hello3/hello3.hrb       \
        App/hello4/hello4.hrb       \
        App/hello5/hello5.hrb       \
        App/winhelo.hrb             \
        App/winhelo2.hrb            \
        App/winhelo3.hrb            \
        App/star1.hrb               \
        App/stars.hrb               \
        App/stars2.hrb              \
        App/lines.hrb               \
        App/walk.hrb                \
        App/noodle.hrb              \
        App/beepdown.hrb            \
        App/color.hrb               \
        App/color2.hrb

##
##    Commands.
##

AS          =  /usr/bin/as
CP          =  /bin/cp
DD          =  /bin/dd
LD          =  /usr/bin/ld
OBJ2HRB     =  ../Tools/obj2hrb

##
##    Targets.
##

.PHONY      :  all  clean  cleanall  cleanobj
.SUFFIXES   :  .o   .s

all         :  ${SUBDIRS}  ${TARGET_IMAGE}

clean       :  ${SUBDIRS}  cleanobj
	${RM}  -f  ${TARGET_IMAGE}

cleanall    :  ${SUBDIRS}  clean

cleanobj    :  ${SUBDIRS}

##
##    Make Sub Directories.
##

RECURSIVE   :
${SUBDIRS}  :  RECURSIVE
	${MAKE}  -C $@  ${MAKECMDGOALS}

##
##    Compile and Link Flags.
##

ASFLAGS     =  -march=i386  --32

##
##    Build.
##

${TARGET_IMAGE} : ${IPLBIN_IMAGE}  ${KERNEL_IMAGE}  \
                  ${APPLICATIONS}  Makefile
	mformat  -f 1440  -C  -B ${IPLBIN_IMAGE}  -i $@  ::
	${DD}  if=${IPLBIN_IMAGE}  bs=512  count=1  of=$@  conv=notrunc
	mcopy  -i $@  ${KERNEL_IMAGE}  ::
	mcopy  -i $@  ${APPLICATIONS}  ::

##
##    Suffix Rules.
##


##
##    List of Sub Directory.
##

SUBDIRS  =  BootSector  Kernel  App

##
##    List of Files.
##

TARGET_IMAGE        =  Haribote.img
IPLBIN_IMAGE        =  BootSector/Ipl.bin
KERNEL_IMAGE        =  Kernel/haribote.sys

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

${TARGET_IMAGE} : ${IPLBIN_IMAGE}  ${KERNEL_IMAGE}  Makefile
	mformat  -f 1440  -C  -B ${IPLBIN_IMAGE}  -i $@  ::
	${DD}  if=${IPLBIN_IMAGE}  bs=512  count=1  of=$@  conv=notrunc
	mcopy  -i $@  ${KERNEL_IMAGE}  ::
	mcopy  -i $@  App/hello.hrb App/hello2.hrb App/a.hrb    \
            App/hello3.hrb App/bug1.hrb App/bug2.hrb        \
            ::

##
##    Suffix Rules.
##

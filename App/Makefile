
##
##    List of Files.
##

APPLICATIONS        =  hlt.hrb

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
.SUFFIXES   :  .hrb  .obj   .s

all         :  ${APPLICATIONS}

clean       :  cleanobj
	${RM}  -f  ${APPLICATIONS}

cleanall :  clean

cleanobj :
	${RM}  -f                           \
        ${APPLICATIONS:%.hrb=%.obj}     \
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


##
##    Suffix Rules.
##

.obj.hrb    :
	${LD}  -T App.ls  -o $@  $<

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

hlt.hrb         :  Makefile App.ls  hlt.obj

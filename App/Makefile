
##
##    List of Files.
##

APPLICATIONS        =  hello.hrb hello2.hrb a.hrb
API_OBJS            =  a_as.obj

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
        ${API_OBJS}                     \
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

hello.hrb   : hello.obj  Makefile Raw.ls
	${LD}  -T Raw.ls  -o $@  $<

hello2.hrb  : hello2.obj  Makefile Raw.ls
	${LD}  -T Raw.ls  -o $@  $<

##
##    Suffix Rules.
##

.obj.hrb    :
	${LD}  -T App.ls  -o $@  $<  ${API_OBJS}

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

a.hrb   : a.obj  Makefile App.ls  ${API_OBJS}

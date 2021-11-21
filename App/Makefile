
##
##    List of Files.
##

APPLICATIONS        =  hello.hrb hello2.hrb a.hrb hello3.hrb    \
                       bug1.hrb bug2.hrb bug3.hrb hello4.hrb    \
                       hello5.hrb winhello.hrb
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

##
##    Suffix Rules.
##

.obj.hrb    :
	${LD}  -T App.ls  -o $@  -Map=${@:%.hrb=%.map}  $<  ${API_OBJS}

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

hello.hrb     : hello.obj     Makefile App.ls  ${API_OBJS}
hello2.hrb    : hello2.obj    Makefile App.ls  ${API_OBJS}

a.hrb         : a.obj         Makefile App.ls  ${API_OBJS}
hello3.hrb    : hello3.obj    Makefile App.ls  ${API_OBJS}
bug1.hrb      : bug1.obj      Makefile App.ls  ${API_OBJS}
bug2.hrb      : bug2.obj      Makefile App.ls  ${API_OBJS}
bug3.hrb      : bug3.obj      Makefile App.ls  ${API_OBJS}
hello4.hrb    : hello4.obj    Makefile App.ls  ${API_OBJS}
hello5.hrb    : hello5.obj    Makefile App.ls  ${API_OBJS}
winhello.hrb  : winhello.obj  Makefile App.ls  ${API_OBJS}



##
##    List of Sub Directory.
##

SUBDIRS  =  a hello3

##
##    List of Files.
##

APPLICATIONS    =  hello4.hrb hello5.hrb winhelo.hrb            \
                   winhelo2.hrb winhelo3.hrb star1.hrb          \
                   stars.hrb stars2.hrb lines.hrb               \
                   walk.hrb noodle.hrb beepdown.hrb             \
                   color.hrb color2.hrb

API_LIBS        =  ../ApiLib/apilib.a

##
##    Commands.
##

AS          =  /usr/bin/as
CAT         =  /bin/cat
CP          =  /bin/cp
DD          =  /bin/dd
NASM        =  /usr/bin/nasm
OBJ2HRB     =  ../Tools/obj2hrb

##
##    Targets.
##

.PHONY      :  all  clean  cleanall  cleanobj
.SUFFIXES   :  .hrb  .obj   .s  .asm

all         :  ${SUBDIRS}  ${APPLICATIONS}

clean       :  ${SUBDIRS}  cleanobj
	${RM}  -f  ${APPLICATIONS}

cleanall    :  ${SUBDIRS}  clean

cleanobj    :  ${SUBDIRS}
	${RM}  -f                           \
        ${APPLICATIONS:%.hrb=%.obj}     \
        ${APPLICATIONS:%.hrb=%.ls}      \
        *.lst  *.map

##
##    Make Sub Directories.
##

RECURSIVE   :
${SUBDIRS}  :  RECURSIVE
	${MAKE}  -C $@  ${MAKECMDGOALS}

##
##    Compile and Link Flags.
##

ASFLAGS         =  -march=i386  --32
CFLAGS          =  -march=i386  -m32  -fno-pie  -nostdlib  -O2
CCVERBOSE       =
NASMFLAGS       =  -fcoff

MALLOC_SIZE     = 0x00010000
STACK_SIZE      = 0x00004000
DATSEG_SIZE     = 0x00004000

##
##    Build.
##

##
##    Suffix Rules.
##

.obj.hrb    :
	${OBJ2HRB}  $@  ${MALLOC_SIZE} ${STACK_SIZE} ${DATSEG_SIZE}  $^

.s.obj  :
	${AS}  -o $@  \
        -a=${@:%.obj=%.lst}         \
        ${ASFLAGS}  $<

.asm.obj    :
	${NASM} -o $@  \
        -l ${@:%.obj=%.lst}         \
        ${NASMFLAGS}  $<

.c.obj  :
	${CC}  -o $@  \
        -Wa,-a=${@:%.obj=%.lst}     \
        ${CCVERBOSE}  ${CFLAGS}  -c  $<

##
##    Dependencies
##

hello4.hrb    : hello4.obj    ${API_LIBS}
hello5.hrb    : hello5.obj    ${API_LIBS}
winhelo.hrb   : winhelo.obj   ${API_LIBS}
winhelo2.hrb  : winhelo2.obj  ${API_LIBS}
winhelo3.hrb  : winhelo3.obj  ${API_LIBS}
star1.hrb     : star1.obj     ${API_LIBS}
stars.hrb     : stars.obj     ${API_LIBS}  ../Common/stdlib.obj
stars2.hrb    : stars2.obj    ${API_LIBS}  ../Common/stdlib.obj
lines.hrb     : lines.obj     ${API_LIBS}
walk.hrb      : walk.obj      ${API_LIBS}
noodle.hrb    : noodle.obj    ${API_LIBS}  ../Common/stdio.obj
beepdown.hrb  : beepdown.obj  ${API_LIBS}
color.hrb     : color.obj     ${API_LIBS}
color2.hrb    : color2.obj    ${API_LIBS}

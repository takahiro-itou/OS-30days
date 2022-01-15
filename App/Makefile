

##
##    List of Sub Directory.
##

SUBDIRS     =  a hello3 hello4 hello5 winhelo winhelo2          \
               winhelo3 star1

##
##    List of Files.
##

APPLICATIONS    =  stars.hrb stars2.hrb lines.hrb               \
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

stars.hrb     : stars.obj     ${API_LIBS}  ../Common/stdlib.obj
stars2.hrb    : stars2.obj    ${API_LIBS}  ../Common/stdlib.obj
lines.hrb     : lines.obj     ${API_LIBS}
walk.hrb      : walk.obj      ${API_LIBS}
noodle.hrb    : noodle.obj    ${API_LIBS}  ../Common/stdio.obj
beepdown.hrb  : beepdown.obj  ${API_LIBS}
color.hrb     : color.obj     ${API_LIBS}
color2.hrb    : color2.obj    ${API_LIBS}

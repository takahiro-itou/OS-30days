
##
##    List of Files.
##

APPLICATIONS        =  hello.hrb hello2.hrb a.hrb hello3.hrb    \
                       hello4.hrb hello5.hrb winhelo.hrb        \
                       winhelo2.hrb winhelo3.hrb star1.hrb      \
                       stars.hrb stars2.hrb lines.hrb           \
                       walk.hrb noodle.hrb beepdown.hrb         \
                       color.hrb color2.hrb crack7.hrb

API_OBJS            =  a_as.obj

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

all         :  ${APPLICATIONS}

clean       :  cleanobj
	${RM}  -f  ${APPLICATIONS}

cleanall :  clean

cleanobj :
	${RM}  -f                           \
        ${APPLICATIONS:%.hrb=%.obj}     \
        ${APPLICATIONS:%.hrb=%.ls}      \
        ${API_OBJS}                     \
        *.lst  *.map

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

hello.hrb     : hello.obj     ${API_OBJS}
hello2.hrb    : hello2.obj    ${API_OBJS}

a.hrb         : a.obj         ${API_OBJS}
hello3.hrb    : hello3.obj    ${API_OBJS}
hello4.hrb    : hello4.obj    ${API_OBJS}
hello5.hrb    : hello5.obj    ${API_OBJS}
winhelo.hrb   : winhelo.obj   ${API_OBJS}
winhelo2.hrb  : winhelo2.obj  ${API_OBJS}
winhelo3.hrb  : winhelo3.obj  ${API_OBJS}
star1.hrb     : star1.obj     ${API_OBJS}
stars.hrb     : stars.obj     ${API_OBJS}  ../Common/stdlib.obj
stars2.hrb    : stars2.obj    ${API_OBJS}  ../Common/stdlib.obj
lines.hrb     : lines.obj     ${API_OBJS}
walk.hrb      : walk.obj      ${API_OBJS}
noodle.hrb    : noodle.obj    ${API_OBJS}  ../Common/stdio.obj
beepdown.hrb  : beepdown.obj  ${API_OBJS}
color.hrb     : color.obj     ${API_OBJS}
color2.hrb    : color2.obj    ${API_OBJS}
crack7.hrb    : crack7.obj

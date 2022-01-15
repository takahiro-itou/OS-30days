
##
##    List of Files.
##

API_OBJS        =  api001.obj api002.obj api003.obj api004.obj  \
                   api005.obj api006.obj api007.obj api008.obj  \
                   api009.obj api010.obj api011.obj api012.obj  \
                   api013.obj api014.obj api015.obj api016.obj  \
                   api017.obj api018.obj api019.obj api020.obj  \
                   ../Common/stdio.obj ../Common/stdlib.obj

API_LIB         =  apilib.a

##
##    Commands.
##

AR          =  /usr/bin/ar
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

all         :  ${API_LIB}

clean       :  cleanobj
	${RM}  -f  ${API_LIB}

cleanall :  clean

cleanobj :
	${RM}  -f                           \
        ${API_OBJS}                     \
        *.lst  *.map

##
##    Compile and Link Flags.
##

ARFLAGS         =  rcs
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

${API_LIB}  : Makefile ${API_OBJS}
	${AR}  ${ARFLAGS}  ${API_LIB}  ${API_OBJS}

##
##    Suffix Rules.
##

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

api001.obj  : Makefile
api002.obj  : Makefile
api003.obj  : Makefile
api004.obj  : Makefile
api005.obj  : Makefile
api006.obj  : Makefile
api007.obj  : Makefile
api008.obj  : Makefile
api009.obj  : Makefile
api010.obj  : Makefile
api011.obj  : Makefile
api012.obj  : Makefile
api013.obj  : Makefile
api014.obj  : Makefile
api015.obj  : Makefile
api016.obj  : Makefile
api017.obj  : Makefile
api018.obj  : Makefile
api019.obj  : Makefile
api020.obj  : Makefile


##
##    List of Files.
##

API_LIBS    =  ../../ApiLib/apilib.a

##
##    Commands.
##

AS          =  /usr/bin/as
CAT         =  /bin/cat
CP          =  /bin/cp
DD          =  /bin/dd
NASM        =  /usr/bin/nasm
OBJ2HRB     =  ../../Tools/obj2hrb

##
##    Targets.
##

.PHONY      :  all  clean  cleanall  cleanobj
.SUFFIXES   :  .hrb  .obj   .c  .s  .asm

all         :  ${APP_NAME}.hrb

clean       :  cleanobj
	${RM}  -f  ${APP_NAME}.hrb

cleanall    :  clean

cleanobj    :
	${RM}  -f  ${APP_NAME}.obj  ${APP_NAME}.ls  *.lst  *.map

##
##    Compile and Link Flags.
##

API_INC_DIR     =  -I../../Common/ -I../../ApiLib/
ASFLAGS         =  -march=i386  --32
CFLAGS          =  -march=i386  -m32  -fno-pie  -nostdlib  -O2  \
                   ${API_INC_DIR}
CCVERBOSE       =
NASMFLAGS       =  -fcoff

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

${APP_NAME}.hrb : ${APP_NAME}.obj  ${API_LIBS}

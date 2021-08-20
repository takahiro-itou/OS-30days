
##
##    List of Files.
##

TARGET_IMAGE        =  Haribote.img
INSTALL_DEST_DIR    =  ln-virtualbox/

##
##    Commands.
##

CP      =  cp

##
##    Targets.
##

.PHONY  :  all  clean  cleanall  cleanobj  install  init

all      :  ${TARGET_IMAGE}

clean    :  cleanobj
	${RM}  -f  ${TARGET_IMAGE}

cleanall :  clean

cleanobj :

install  :  ${TARGET_IMAGE}
	${CP}  -pv  ${TARGET_IMAGE}  ${INSTALL_DEST_DIR}

init     :
	dd  if=/dev/zero  of=${TARGET_IMAGE}  bs=512  count=2880

##
##    Build.
##

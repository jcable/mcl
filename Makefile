# $Id: Makefile,v 1.9 2005/05/12 16:03:12 moi Exp $
#
# build everything...

# We need the GNU make tool, on some systems it is called
# gmake, on others make is an alias to gmake. Choose one...
#
MAKE = make
#MAKE = gmake


# A phony target is one that is not really the name of a file.
# Using .PHONY prevents problems when a file/dir of the same name
# already exists as is the case here with alc/norm.

.PHONY: all
all: alc norm


.PHONY: alc
alc:
	@echo "--------------------"
	@echo "*** MCL-ALC library ***"
	@echo "--------------------"
	cd src/alc; ${MAKE} cleanall; ${MAKE} depend; ${MAKE}
	@echo "--------------------"
	@echo "***SDP library ***"
	@echo "--------------------"
	cd src/sdp_lib/src; ${MAKE} cleanall; ${MAKE} 
	@echo "--------------------"
	@echo "*** FLUTE library ***"
	@echo "--------------------"
	cd src/flute_lib; ${MAKE} cleanall; ${MAKE} depend; ${MAKE}
	@echo "--------------------"
	@echo "*** FCAST-ALC tool ***"
	@echo "--------------------"
	cd fcast; ${MAKE} cleanall_alc; ${MAKE} depend; ${MAKE} alc
	@echo "--------------------"
	@echo "*** RobCast tool ***"
	@echo "--------------------"
	cd robcast; ${MAKE} cleanall; ${MAKE} depend; ${MAKE}
	@echo "--------------------"
	@echo "*** Flute tool ***"
	@echo "--------------------"
	cd flute; ${MAKE} cleanall; ${MAKE} depend; ${MAKE} all
	@echo "--------------------"
	@echo "*** mclftp tool ***"
	@echo "--------------------"
	#cd mclftp; ${MAKE} cleanall; ${MAKE} depend; ${MAKE}
	@echo "skipped..."
	@echo "--------------------"
	@echo "*** ALC validation programs ***"
	@echo "--------------------"
	cd check/alc; ${MAKE} cleanall; ${MAKE} depend; ${MAKE}
	@echo "--------------------"
	@echo "*** FLUTE validation programs ***"
	@echo "--------------------"
	cd check/flute; ${MAKE} cleanall; ${MAKE} depend; ${MAKE}
	@echo "done"


.PHONY: norm
norm:
	@echo "--------------------"
	@echo "*** MCL-NORM library ***"
	@echo "--------------------"
	cd src/norm; ${MAKE} cleanall; ${MAKE} depend; ${MAKE}
	@echo "--------------------"
	@echo "*** FCAST-NORM tool ***"
	@echo "--------------------"
	cd fcast; ${MAKE} cleanall_norm; ${MAKE} depend; ${MAKE} norm
	@echo "--------------------"
	@echo "*** mclftp-NORM tool ***"
	@echo "--------------------"
	cd mclftp; ${MAKE} cleanall; ${MAKE} depend; ${MAKE} norm
	@echo "--------------------"
	@echo "*** NORM validation programs ***"
	@echo "--------------------"
	cd check/norm; ${MAKE} cleanall; ${MAKE} depend; ${MAKE}
	@echo "done"


.PHONY: clean
clean:
	@echo "--------------------"
	@echo "*** Cleaning MCL-ALC library  ***"
	@echo "--------------------"
	cd src/alc; ${MAKE} clean
	@echo "--------------------"
	@echo "*** Cleaning FLUTE library  ***"
	@echo "--------------------"
	cd src/flute_lib; ${MAKE} clean
	@echo "--------------------"
	@echo "*** Cleaning MCL-NORM library  ***"
	@echo "--------------------"
	cd src/norm; ${MAKE} clean
	@echo "--------------------"
	@echo "*** Cleaning FCAST tool ***"
	@echo "--------------------"
	cd fcast; ${MAKE} clean
	@echo "--------------------"
	@echo "*** Cleaning RobCast tool ***"
	@echo "--------------------"
	cd robcast; ${MAKE} clean
	@echo "--------------------"
	@echo "*** Cleaning Flute tool ***"
	@echo "--------------------"
	cd flute; ${MAKE} clean
	@echo "--------------------"
	@echo "*** Cleaning SDP library ***"
	@echo "--------------------"
	cd src/sdp_lib/src; ${MAKE} clean
	@echo "--------------------"
	@echo "*** Cleaning mclftp tool ***"
	@echo "--------------------"
	cd mclftp; ${MAKE} clean
	@echo "--------------------"
	@echo "*** Cleaning ALC validation programs ***"
	@echo "--------------------"
	cd check/alc; ${MAKE} clean
	@echo "--------------------"
	@echo "*** Cleaning FLUTE validation programs ***"
	@echo "--------------------"
	cd check/flute; ${MAKE} clean	
	@echo "--------------------"
	@echo "*** Cleaning NORM validation programs ***"
	@echo "--------------------"
	cd check/norm; ${MAKE} clean
	@echo "done"


.PHONY: cleanall
cleanall:
	@echo "--------------------"
	@echo "*** Cleaning MCL-ALC library  ***"
	@echo "--------------------"
	cd src/alc; ${MAKE} cleanall
	@echo "--------------------"
	@echo "*** Cleaning FLUTE library  ***"
	@echo "--------------------"
	cd src/flute_lib; ${MAKE} cleanall
	@echo "--------------------"
	@echo "*** Cleaning MCL-NORM library  ***"
	@echo "--------------------"
	cd src/norm; ${MAKE} cleanall
	@echo "--------------------"
	@echo "*** Cleaning FCAST tool ***"
	@echo "--------------------"
	cd fcast; ${MAKE} cleanall
	@echo "--------------------"
	@echo "*** Cleaning RobCast tool ***"
	@echo "--------------------"
	cd robcast; ${MAKE} cleanall
	@echo "--------------------"
	@echo "*** Cleaning SDP library ***"
	@echo "--------------------"
	cd src/sdp_lib/src; ${MAKE} cleanall
	@echo "--------------------"
	@echo "*** Cleaning Flute tool ***"
	@echo "--------------------"
	cd flute; ${MAKE} cleanall
	@echo "--------------------"
	@echo "*** Cleaning mclftp tool ***"
	@echo "--------------------"
	cd mclftp; ${MAKE} cleanall
	@echo "--------------------"
	@echo "*** Cleaning ALC validation programs ***"
	@echo "--------------------"
	cd check/alc; ${MAKE} cleanall
	@echo "--------------------"
	@echo "*** Cleaning FLUTE validation programs ***"
	@echo "--------------------"
	cd check/flute; ${MAKE} cleanall	
	@echo "--------------------"
	@echo "*** Cleaning NORM validation programs ***"
	@echo "--------------------"
	cd check/norm; ${MAKE} cleanall
	@echo "*** Cleaning ALC doxygen documents ***"
	@echo "--------------------"
	cd doc/doxygen/alc; rm html/*.html; rm -rf man/man3/*.3
	@echo "*** Cleaning NORM doxygen documents ***"
	@echo "--------------------"
	cd doc/doxygen/norm; rm html/*.html; rm -rf man/man3/*.3
	@echo "done"

# DO NOT DELETE

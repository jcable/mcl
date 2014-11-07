# $Id: Makefile,v 1.3 2004/12/21 16:18:26 roca Exp $
#
# Makefile
#

###### OS Dependant Definitions ######

# they are defined in the following file...
# edit Rules.make as appropriate
include ../Makefile.common


ifeq (${OS},linux)
	# Linux version
endif

ifeq (${OS},solaris)
	# Solaris version
endif


###### Other Variables ######


# List of source, object, and include files
#
# mclrecv2.man.1
# mclsend2.man.1
# mcl_open.man.3
# mcl_ctl.man.3
# mcl_wait_event.man.3
# fcast.man.1

man_FILES = fcast.man.1 fcastn.man.1 flute.man.1 mclrecv2.man.1 mclsend2.man.1 mcl_open.man.3 mcl_ctl.man.3 mcl_wait_event.man.3 mcl_close.man.3 mcl_abort.man.3 mcl_send.man.3 mcl_recv.man.3
html_FILES = fcast.man.1.html fcastn.man.1.html flute.man.1.html mclrecv2.man.1.html mclsend2.man.1.html mcl_open.man.3.html mcl_ctl.man.3.html mcl_wait_event.man.3.html mcl_close.man.3.html mcl_abort.man.3.html mcl_send.man.3.html mcl_recv.man.3.html
ps_FILES = ${html_FILES:.html=.ps.gz}


# General purpose local FLAGS.
#
FLAGS   = 


###### Dependancies ######

all: ${ps_FILES} ${html_FILES}

ps: ${ps_FILES}

html: ${html_FILES}


${ps_FILES}: ${man_FILES}
	@for f in ${man_FILES} ; do \
		echo "create a compressed postscript version of manual $${f}... "; \
		if [ -f "$${f}.ps.gz" ] ; then \
			rm "$${f}.ps.gz"; \
		fi; \
		man -t "./$${f}" > "$${f}.ps"; \
		gzip "$${f}.ps"; \
	done

${html_FILES}: ${man_FILES}
	@for f in ${man_FILES} ; do \
		echo "create an HTML version of manual $${f}... "; \
		man2html "./$${f}" > "$${f}.html"; \
	done
		#/usr/bin/groff -Thtml -mandoc "./$${f}" > "$${f}.html"; \

clean :
	@for f in core ${ps_FILES} ${html_FILES} ; do \
		if [ -f "$${f}" ] ; then \
			rm "$${f}"; \
			echo "$${f} removed"; \
		fi \
	done

cleanall : clean


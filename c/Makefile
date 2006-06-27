# $Header: /home/cjm/cvs/autoguider/c/Makefile,v 1.4 2006-06-27 20:42:51 cjm Exp $

include ../../Makefile.common
include ../commandserver/Makefile.common
include ../ngatcil/Makefile.common
include ../ccd/Makefile.common
include ../Makefile.common

BINDIR			= $(AUTOGUIDER_BIN_HOME)/c/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_DOC_HOME)/cdocs

#DEBUG_CFLAGS		= 
DEBUG_CFLAGS		= -DAUTOGUIDER_DEBUG=10 -DAUTOGUIDER_CIL_SILENCE

# autoguider ccd (general) library
CCD_CFLAGS 		= -I$(AUTOGUIDER_CCD_SRC_HOME)/include
CCD_LDFLAGS 		= -lautoguider_ccd_general

# command_server library
COMMAND_SERVER_CFLAGS 	= -I$(AUTOGUIDER_COMMANDSERVER_SRC_HOME)/include
COMMAND_SERVER_LDFLAGS 	= -lautoguider_commandserver

# ngatcil library
NGATCIL_CFLAGS 		= -I$(AUTOGUIDER_NGATCIL_SRC_HOME)/include
NGATCIL_LDFLAGS 	= -lautoguider_ngatcil

# CFITSIO
CFITSIO_CFLAGS		= -I$(CFITSIOINCDIR) 
CFITSIO_LDFLAGS		= -lcfitsio

# libdprt_object
OBJECT_CFLAGS		= -I/home/dev/src/libdprt/object/include
OBJECT_LDFLAGS		= -ldprt_object

CFLAGS 			= -g -I$(INCDIR) $(DEBUG_CFLAGS) $(CCD_CFLAGS) $(COMMAND_SERVER_CFLAGS) $(NGATCIL_CFLAGS) \
				$(CFITSIO_CFLAGS) $(OBJECT_CFLAGS)
DOCFLAGS 		= -static

EXE_SRCS		= autoguider.c
OBJ_SRCS		= autoguider_buffer.c autoguider_cil.c autoguider_command.c autoguider_dark.c \
			autoguider_field.c autoguider_flat.c autoguider_general.c autoguider_get_fits.c \
			autoguider_guide.c autoguider_object.c autoguider_server.c
SRCS			= $(EXE_SRCS) $(OBJ_SRCS)
HEADERS			= $(OBJ_SRCS:%.c=$(INCDIR)/%.h)
EXE_OBJS		= $(EXE_SRCS:%.c=$(BINDIR)/%.o)
OBJS			= $(SRCS:%.c=$(BINDIR)/%.o)
EXES			= $(EXE_SRCS:%.c=$(BINDIR)/%)
DOCS 			= $(SRCS:%.c=$(DOCSDIR)/%.html)
CONFIG_SRCS		= autoguider.properties
CONFIG_BINS		= $(CONFIG_SRCS:%.properties=$(BINDIR)/%.properties)

top: $(EXES) $(CONFIG_BINS) docs

$(BINDIR)/autoguider: $(OBJS)
	$(CC) $^ -o $@  -L$(LT_LIB_HOME) $(COMMAND_SERVER_LDFLAGS) $(NGATCIL_LDFLAGS) $(CCD_LDFLAGS) \
		$(CFITSIO_LDFLAGS) $(OBJECT_LDFLAGS) $(TIMELIB) $(SOCKETLIB) -lm -lc

#$(BINDIR)/%: %.o
#	$(CC) $< -o $@  -L$(LT_LIB_HOME) $(COMMAND_SERVER_LDFLAGS) $(CCD_LDFLAGS) $(TIMELIB) $(SOCKETLIB) -lm -lc

$(BINDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@  

$(BINDIR)/%.properties: %.properties
	$(CP) $< $@  

docs: $(DOCS)

$(DOCS): $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

$(DOCS) : $(SRCS)

depend:
	makedepend -p$(BINDIR)/ -- $(CFLAGS) -- $(SRCS)

clean:
	$(RM) $(RM_OPTIONS) $(EXES) $(OBJS) $(TIDY_OPTIONS)

tidy:
	$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)

backup: tidy
	$(RM) $(RM_OPTIONS) $(OBJS)

checkin:
	-$(CI) $(CI_OPTIONS) $(SRCS)
	-(cd $(INCDIR); $(CI) $(CI_OPTIONS) $(HEADERS);)

checkout:
	-$(CO) $(CO_OPTIONS) $(SRCS)
	-(cd $(INCDIR); $(CO) $(CO_OPTIONS) $(HEADERS);)

# DO NOT DELETE

/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/signal.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/features.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/signum.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/stdio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/libio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/gconv.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/string.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/commandserver/include/command_server.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/libdprt/object/include/object.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/ccd/include/ccd_driver.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/ccd/include/ccd_exposure.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/include/autoguider_buffer.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/include/autoguider_cil.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/include/autoguider_command.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/include/autoguider_dark.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/include/autoguider_field.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/include/autoguider_flat.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/include/autoguider_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/pthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /usr/include/bits/sigthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/include/autoguider_guide.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/include/autoguider_object.h
/home/dev/bin/autoguider/c/i386-linux/autoguider.o: /home/dev/src/autoguider/include/autoguider_server.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/features.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/pthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/signal.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/signum.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/sigthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/stdio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/libio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/gconv.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/string.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/unistd.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /home/dev/src/autoguider/include/autoguider_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_buffer.o: /home/dev/src/autoguider/include/autoguider_buffer.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/features.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/stdio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/libio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/gconv.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/string.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/unistd.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_cil.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_tcs_guide_packet.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_udp_raw.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /home/dev/src/autoguider/include/autoguider_command.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /home/dev/src/autoguider/include/autoguider_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/pthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/signal.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/signum.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /usr/include/bits/sigthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_cil.o: /home/dev/src/autoguider/include/autoguider_guide.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/features.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/stdio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/libio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/gconv.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/string.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/unistd.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/ccd/include/ccd_exposure.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/commandserver/include/command_server.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/include/autoguider_cil.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/include/autoguider_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/pthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/signal.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/signum.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /usr/include/bits/sigthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/include/autoguider_server.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/include/autoguider_field.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/include/autoguider_get_fits.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/include/autoguider_guide.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/include/autoguider_object.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_command.o: /home/dev/src/autoguider/include/autoguider_command.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/features.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/stdio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/libio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/gconv.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/string.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/unistd.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /home/dev/src/cfitsio3.006/include/fitsio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/stdlib.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/limits.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/limits.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /home/dev/src/cfitsio3.006/include/longnam.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /home/dev/src/autoguider/include/autoguider_dark.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /home/dev/src/autoguider/include/autoguider_field.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /home/dev/src/autoguider/include/autoguider_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/pthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/signal.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/signum.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /usr/include/bits/sigthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_dark.o: /home/dev/src/autoguider/include/autoguider_guide.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/features.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/stdio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/libio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/gconv.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/string.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/unistd.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /home/dev/src/autoguider/ccd/include/ccd_exposure.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /home/dev/src/autoguider/include/autoguider_dark.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /home/dev/src/autoguider/include/autoguider_field.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /home/dev/src/autoguider/include/autoguider_flat.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /home/dev/src/autoguider/include/autoguider_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/pthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/signal.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/signum.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /usr/include/bits/sigthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /home/dev/src/autoguider/include/autoguider_guide.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /home/dev/src/autoguider/include/autoguider_object.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_field.o: /home/dev/src/autoguider/include/autoguider_command.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/features.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/stdio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/libio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/gconv.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/string.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/unistd.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /home/dev/src/cfitsio3.006/include/fitsio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/stdlib.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/limits.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/limits.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /home/dev/src/cfitsio3.006/include/longnam.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /home/dev/src/autoguider/include/autoguider_field.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /home/dev/src/autoguider/include/autoguider_flat.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /home/dev/src/autoguider/include/autoguider_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/pthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/signal.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/signum.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /usr/include/bits/sigthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_flat.o: /home/dev/src/autoguider/include/autoguider_guide.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/features.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/pthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/signal.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/signum.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/sigthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/stdio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/libio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/gconv.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/string.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/unistd.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /home/dev/src/autoguider/commandserver/include/command_server.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /home/dev/src/libdprt/object/include/object.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_general.o: /home/dev/src/autoguider/include/autoguider_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/features.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/stdio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/libio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/gconv.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/stdlib.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/string.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/unistd.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /home/dev/src/cfitsio3.006/include/fitsio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/limits.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/limits.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /home/dev/src/cfitsio3.006/include/longnam.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /home/dev/src/autoguider/include/autoguider_buffer.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /home/dev/src/autoguider/include/autoguider_field.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /home/dev/src/autoguider/include/autoguider_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/pthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/signal.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/signum.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /usr/include/bits/sigthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /home/dev/src/autoguider/include/autoguider_get_fits.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_get_fits.o: /home/dev/src/autoguider/include/autoguider_guide.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/features.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/pthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/signal.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/signum.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/sigthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/stdio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/libio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/gconv.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/string.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/unistd.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/ccd/include/ccd_exposure.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_tcs_guide_packet.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/include/autoguider_cil.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/include/autoguider_dark.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/include/autoguider_field.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/include/autoguider_flat.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/include/autoguider_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/include/autoguider_guide.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/include/autoguider_object.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_guide.o: /home/dev/src/autoguider/include/autoguider_command.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/features.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/stdio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/libio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/gconv.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/string.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/unistd.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /home/dev/src/libdprt/object/include/object.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /home/dev/src/autoguider/include/autoguider_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/pthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/signal.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/signum.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /usr/include/bits/sigthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /home/dev/src/autoguider/include/autoguider_object.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_object.o: /home/dev/src/autoguider/include/autoguider_command.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/features.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/stdio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/libio.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/gconv.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/string.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/unistd.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /home/dev/src/autoguider/commandserver/include/command_server.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /home/dev/src/autoguider/include/autoguider_general.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/pthread.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/signal.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/signum.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/c/i386-linux/autoguider_server.o: /usr/include/bits/sigthread.h

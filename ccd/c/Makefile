# $Header: /home/cjm/cvs/autoguider/ccd/c/Makefile,v 1.3 2011-09-07 11:02:45 cjm Exp $

include ../../../Makefile.common
include ../../Makefile.common
include ../Makefile.common

BINDIR			= $(AUTOGUIDER_CCD_BIN_HOME)/c/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_CCD_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_CCD_DOC_HOME)/cdocs

#DEBUG_CFLAGS		= 
DEBUG_CFLAGS		= -DCCD_DEBUG

# estar_config is used for configuration.
ESTAR_CONFIG_CFLAGS 	= -I$(eSTAR_SRC_HOME)/config/include
ESTAR_CONFIG_LDFLAGS 	= -L$(eSTAR_LIB_HOME) -lestar_config

# CFITSIO library
CFITSIO_CFLAGS		= -I$(CFITSIOINCDIR) 
CFITSIO_LDFLAGS		= -L$(LT_LIB_HOME) -lcfitsio

# log_udp library (log_udp.h is included for verbosity settings)
LOG_UDP_CFLAGS		= -I$(LOG_UDP_SRC_HOME)/include

# dynamic linking loader shared library flags
# -rdynamic ?
DL_LDFLAGS		= -ldl

CFLAGS 			= -g -I$(INCDIR) $(DEBUG_CFLAGS) $(ESTAR_CONFIG_CFLAGS) $(CFITSIO_CFLAGS) $(LOG_UDP_CFLAGS)
DOCFLAGS 		= -static

LIB_SRCS		= ccd_general.c ccd_config.c ccd_driver.c ccd_exposure.c ccd_setup.c ccd_temperature.c
SRCS			= $(LIB_SRCS)
LIB_HEADERS		= $(LIB_SRCS:%.c=$(INCDIR)/%.h)
HEADERS			= $(LIB_HEADERS)
LIB_OBJS		= $(LIB_SRCS:%.c=$(BINDIR)/%.o)
OBJS			= $(SRCS:%.c=$(BINDIR)/%.o)
DOCS 			= $(SRCS:%.c=$(DOCSDIR)/%.html)

top: $(LT_LIB_HOME)/$(CCD_LIBRARYNAME).so docs

static: $(LT_LIB_HOME)/$(CCD_LIBRARYNAME).so

$(LT_LIB_HOME)/$(CCD_LIBRARYNAME).so : $(LIB_OBJS)
	$(CC) $(CCSHAREDFLAG) $(CFLAGS) $(LIB_OBJS) -o $@ $(ESTAR_CONFIG_LDFLAGS) $(CFITSIO_LDFLAGS) $(DL_LDFLAGS)

$(LT_LIB_HOME)/$(CCD_LIBRARYNAME).a : $(LIB_OBJS)
	ar rcv $@ $?

#$(BINDIR)/%: %.o
#	$(CC) $< -o $@  -L$(LT_LIB_HOME) $(TIMELIB) $(SOCKETLIB) -lm -lc

$(BINDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@  

docs: $(DOCS)

$(DOCS): $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

$(DOCS) : $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -p$(BINDIR)/ -- $(CFLAGS) -- $(SRCS)

clean:
	$(RM) $(RM_OPTIONS) $(EXES) $(OBJS) $(LT_LIB_HOME)/$(CCD_LIBRARYNAME).so $(TIDY_OPTIONS)

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

/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/errno.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/time.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/unistd.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /usr/include/getopt.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_general.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/unistd.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /usr/include/getopt.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /home/dev/src/estar/config/include/estar_config.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_config.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/unistd.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/getopt.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/dlfcn.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/bits/dlfcn.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /home/dev/src/autoguider/ccd/include/ccd_driver.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/time.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_driver.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/unistd.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/getopt.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /home/dev/src/cfitsio3.006/include/fitsio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/stdlib.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/limits.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/limits.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/bits/posix1_lim.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/bits/local_lim.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/linux/limits.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/bits/posix2_lim.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /home/dev/src/cfitsio3.006/include/longnam.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_driver.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/time.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_exposure.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/unistd.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/getopt.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /home/dev/src/autoguider/ccd/include/ccd_driver.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/time.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_setup.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/unistd.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/getopt.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_driver.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/time.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
/home/dev/bin/autoguider/ccd/c/i386-linux/ccd_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_general.h

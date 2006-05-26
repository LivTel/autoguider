# $Header: /home/cjm/cvs/autoguider/ccd/c/Makefile,v 1.1 2006-05-26 10:08:06 cjm Exp $

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

# dynamic linking loader shared library flags
# -rdynamic ?
DL_LDFLAGS		= -ldl

CFLAGS 			= -g -I$(INCDIR) $(DEBUG_CFLAGS) $(ESTAR_CONFIG_CFLAGS) $(CFITSIO_CFLAGS)
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

ccd_general.o: /usr/include/errno.h /usr/include/features.h
ccd_general.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
ccd_general.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
ccd_general.o: /usr/include/asm/errno.h
ccd_general.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
ccd_general.o: /usr/include/stdio.h
ccd_general.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
ccd_general.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
ccd_general.o: /usr/include/bits/typesizes.h /usr/include/libio.h
ccd_general.o: /usr/include/_G_config.h /usr/include/wchar.h
ccd_general.o: /usr/include/bits/wchar.h /usr/include/gconv.h
ccd_general.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
ccd_general.o: /usr/include/string.h /usr/include/time.h
ccd_general.o: /usr/include/bits/time.h /usr/include/unistd.h
ccd_general.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
ccd_general.o: /usr/include/getopt.h
ccd_general.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
ccd_config.o: /usr/include/stdio.h /usr/include/features.h
ccd_config.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
ccd_config.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
ccd_config.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
ccd_config.o: /usr/include/bits/typesizes.h /usr/include/libio.h
ccd_config.o: /usr/include/_G_config.h /usr/include/wchar.h
ccd_config.o: /usr/include/bits/wchar.h /usr/include/gconv.h
ccd_config.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
ccd_config.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
ccd_config.o: /usr/include/string.h /usr/include/unistd.h
ccd_config.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
ccd_config.o: /usr/include/getopt.h
ccd_config.o: /home/dev/src/estar/config/include/estar_config.h
ccd_config.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
ccd_config.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
ccd_driver.o: /usr/include/stdio.h /usr/include/features.h
ccd_driver.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
ccd_driver.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
ccd_driver.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
ccd_driver.o: /usr/include/bits/typesizes.h /usr/include/libio.h
ccd_driver.o: /usr/include/_G_config.h /usr/include/wchar.h
ccd_driver.o: /usr/include/bits/wchar.h /usr/include/gconv.h
ccd_driver.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
ccd_driver.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
ccd_driver.o: /usr/include/string.h /usr/include/unistd.h
ccd_driver.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
ccd_driver.o: /usr/include/getopt.h /usr/include/dlfcn.h
ccd_driver.o: /usr/include/bits/dlfcn.h
ccd_driver.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
ccd_driver.o: /home/dev/src/autoguider/ccd/include/ccd_driver.h
ccd_driver.o: /usr/include/time.h /usr/include/bits/time.h
ccd_driver.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
ccd_driver.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
ccd_driver.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
ccd_exposure.o: /usr/include/stdio.h /usr/include/features.h
ccd_exposure.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
ccd_exposure.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
ccd_exposure.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
ccd_exposure.o: /usr/include/bits/typesizes.h /usr/include/libio.h
ccd_exposure.o: /usr/include/_G_config.h /usr/include/wchar.h
ccd_exposure.o: /usr/include/bits/wchar.h /usr/include/gconv.h
ccd_exposure.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
ccd_exposure.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
ccd_exposure.o: /usr/include/string.h /usr/include/unistd.h
ccd_exposure.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
ccd_exposure.o: /usr/include/getopt.h
ccd_exposure.o: /home/dev/src/cfitsio3.006/include/fitsio.h
ccd_exposure.o: /usr/include/stdlib.h /usr/include/limits.h
ccd_exposure.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/limits.h
ccd_exposure.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
ccd_exposure.o: /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h
ccd_exposure.o: /home/dev/src/cfitsio3.006/include/longnam.h
ccd_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
ccd_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_driver.h
ccd_exposure.o: /usr/include/time.h /usr/include/bits/time.h
ccd_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
ccd_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
ccd_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_exposure.h
ccd_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
ccd_setup.o: /usr/include/stdio.h /usr/include/features.h
ccd_setup.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
ccd_setup.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
ccd_setup.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
ccd_setup.o: /usr/include/bits/typesizes.h /usr/include/libio.h
ccd_setup.o: /usr/include/_G_config.h /usr/include/wchar.h
ccd_setup.o: /usr/include/bits/wchar.h /usr/include/gconv.h
ccd_setup.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
ccd_setup.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
ccd_setup.o: /usr/include/string.h /usr/include/unistd.h
ccd_setup.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
ccd_setup.o: /usr/include/getopt.h
ccd_setup.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
ccd_setup.o: /home/dev/src/autoguider/ccd/include/ccd_driver.h
ccd_setup.o: /usr/include/time.h /usr/include/bits/time.h
ccd_setup.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
ccd_setup.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
ccd_setup.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
ccd_temperature.o: /usr/include/stdio.h /usr/include/features.h
ccd_temperature.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
ccd_temperature.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
ccd_temperature.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
ccd_temperature.o: /usr/include/bits/typesizes.h /usr/include/libio.h
ccd_temperature.o: /usr/include/_G_config.h /usr/include/wchar.h
ccd_temperature.o: /usr/include/bits/wchar.h /usr/include/gconv.h
ccd_temperature.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
ccd_temperature.o: /usr/include/bits/stdio_lim.h
ccd_temperature.o: /usr/include/bits/sys_errlist.h /usr/include/string.h
ccd_temperature.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
ccd_temperature.o: /usr/include/bits/confname.h /usr/include/getopt.h
ccd_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_driver.h
ccd_temperature.o: /usr/include/time.h /usr/include/bits/time.h
ccd_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
ccd_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
ccd_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_general.h

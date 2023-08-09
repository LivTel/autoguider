# $Header: /home/cjm/cvs/autoguider/ccd/andor/test/Makefile,v 1.5 2011-09-07 10:59:37 cjm Exp $

include ../../../../Makefile.common
include ../../../Makefile.common
include ../../Makefile.common
include ../Makefile.common

BINDIR			= $(AUTOGUIDER_ANDOR_BIN_HOME)/test/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_ANDOR_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_ANDOR_DOC_HOME)/test

#DEBUG_CFLAGS		= 
DEBUG_CFLAGS		= -DANDOR_DEBUG

# autoguider ccd (general) library
CCD_CFLAGS 		= -I$(AUTOGUIDER_CCD_SRC_HOME)/include
CCD_LDFLAGS 		= -lautoguider_ccd_general

# autoguider ccd (andor) library
ANDOR_CFLAGS 		= -I$(AUTOGUIDER_ANDOR_SRC_HOME)/include

# Andor 2.85.30000
# Current master copy on ltdevsrv
ANDOR_LDFLAGS 		= -L$(LT_SRC_HOME)/andor/andor-2.85.30000/lib/$(HOSTTYPE) -lautoguider_ccd_andor \
			  -landor -lstdc++ -lpthread -ldl

# NB because Andor 2.15 does not come with static libraries, we need the -L here
# Andor 2.12 came with static libraries, and did not need this.
#ANDOR_LDFLAGS 		= -L$(AUTOGUIDER_SRC_HOME)/andor/andor/lib/ -lautoguider_ccd_andor -landor \
#			-lstdc++ -lpthread -ldl

# andor library
#ANDOR_CFLAGS 		= -I$(AUTOGUIDER_SRC_HOME)/andor/andor/examples/common/
#ANDOR_LDFLAGS 		= -L$(AUTOGUIDER_SRC_HOME)/andor/andor/examples/common/ -landor-gcc3 -lpthread -ldl

# CFITSIO library
CFITSIO_CFLAGS		= -I$(CFITSIOINCDIR) 
CFITSIO_LDFLAGS		= -L$(LT_LIB_HOME) -lcfitsio

CFLAGS 			= -g -I$(INCDIR) $(DEBUG_CFLAGS) $(CCD_CFLAGS) $(ANDOR_CFLAGS) $(CFITSIO_CFLAGS)
DOCFLAGS 		= -static

EXE_SRCS		= test_exposure.c test_temperature.c
SRCS			= $(EXE_SRCS)
#LIB_HEADERS		= $(LIB_SRCS:%.c=$(INCDIR)/%.h)
#HEADERS			= $(LIB_HEADERS)
EXE_OBJS		= $(EXE_SRCS:%.c=$(BINDIR)/%.o)
OBJS			= $(SRCS:%.c=$(BINDIR)/%.o)
EXES			= $(EXE_SRCS:%.c=$(BINDIR)/%)
DOCS 			= $(SRCS:%.c=$(DOCSDIR)/%.html)
CONFIG_SRCS		= test_exposure.properties
CONFIG_BINS		= $(CONFIG_SRCS:%.properties=$(BINDIR)/%.properties)

top: $(EXES) $(CONFIG_BINS) docs

$(BINDIR)/%: %.o
	$(CC) $< -o $@  -L$(LT_LIB_HOME) $(ANDOR_LDFLAGS) $(CCD_LDFLAGS) $(CFITSIO_LDFLAGS) $(TIMELIB) $(SOCKETLIB) -lm -lc

$(BINDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@  

$(BINDIR)/%.properties: %.properties
	$(CP) $< $@  

docs: $(DOCS)

$(DOCS): $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

$(DOCS) : $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -p$(BINDIR)/ -- $(CFLAGS) -- $(SRCS)

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

/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/gnu/stubs-32.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/time.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_exposure.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /home/dev/src/autoguider/ccd/andor/include/andor_exposure.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /home/dev/src/autoguider/ccd/andor/include/andor_general.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /home/dev/src/autoguider/ccd/andor/include/andor_setup.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /home/dev/src/cfitsio3200/include/fitsio.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/stdlib.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/sys/types.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/endian.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/endian.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/sys/select.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/select.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/sys/sysmacros.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/alloca.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/limits.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/posix1_lim.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/local_lim.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/linux/limits.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /usr/include/bits/posix2_lim.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_exposure.o: /home/dev/src/cfitsio3200/include/longnam.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/gnu/stubs-32.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/time.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_exposure.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /home/dev/src/autoguider/ccd/andor/include/andor_general.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /home/dev/src/autoguider/ccd/andor/include/andor_setup.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/ccd/andor/test/i386-linux/test_temperature.o: /home/dev/src/autoguider/ccd/andor/include/andor_temperature.h

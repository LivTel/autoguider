# $Header: /home/cjm/cvs/autoguider/ccd/andor/c/Makefile,v 1.6 2011-09-07 10:57:06 cjm Exp $

include ../../../../Makefile.common
include ../../../Makefile.common
include ../../Makefile.common
include ../Makefile.common

BINDIR			= $(AUTOGUIDER_ANDOR_BIN_HOME)/c/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_ANDOR_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_ANDOR_DOC_HOME)/cdocs

#DEBUG_CFLAGS		= 
DEBUG_CFLAGS		= -DANDOR_DEBUG

# autoguider ccd (general) library
CCD_CFLAGS 		= -I$(AUTOGUIDER_CCD_SRC_HOME)/include
CCD_LDFLAGS 		= -lautoguider_ccd_general

# log_udp library (log_udp.h is included for verbosity settings)
LOG_UDP_CFLAGS		= -I$(LOG_UDP_SRC_HOME)/include

# andor library
# need to link to standard c++ library as autoguider_ccd_andor links against andor-gcc3
# which links against stdc++
# Old andor 2.12 links
#ANDOR_CFLAGS 		= -I$(AUTOGUIDER_SRC_HOME)/andor/andor/examples/common/
#ANDOR_LDFLAGS 		= -L$(AUTOGUIDER_SRC_HOME)/andor/andor/examples/common/ -landor-gcc3 -lstdc++ -lpthread -ldl
# New Andor 2.15
#ANDOR_CFLAGS 		= -I$(AUTOGUIDER_SRC_HOME)/andor/andor/include/
#ANDOR_LDFLAGS 		= -L$(AUTOGUIDER_SRC_HOME)/andor/andor/lib/ -landor -lstdc++ -lpthread -ldl
# Andor 2.85.30000
# Current master copy on ltdevsrv
ANDOR_CFLAGS 		= -I$(LT_SRC_HOME)/andor/andor-2.85.30000/include/
ANDOR_LDFLAGS 		= -L$(LT_SRC_HOME)/andor/andor-2.85.30000/lib/$(HOSTTYPE) -landor -lstdc++ -lpthread -ldl

CFLAGS 			= -g -I$(INCDIR) $(DEBUG_CFLAGS) $(ANDOR_CFLAGS) $(CCD_CFLAGS) $(LOG_UDP_CFLAGS) \
			  $(SHARED_LIB_CFLAGS)
DOCFLAGS 		= -static

LIB_SRCS		= andor_general.c andor_setup.c andor_exposure.c andor_temperature.c andor_driver.c
SRCS			= $(LIB_SRCS)
LIB_HEADERS		= $(LIB_SRCS:%.c=$(INCDIR)/%.h)
HEADERS			= $(LIB_HEADERS)
LIB_OBJS		= $(LIB_SRCS:%.c=$(BINDIR)/%.o)
OBJS			= $(SRCS:%.c=$(BINDIR)/%.o)
DOCS 			= $(SRCS:%.c=$(DOCSDIR)/%.html)

top: $(LT_LIB_HOME)/$(ANDOR_LIBRARYNAME).so docs

static: $(LT_LIB_HOME)/$(ANDOR_LIBRARYNAME).so

$(LT_LIB_HOME)/$(ANDOR_LIBRARYNAME).so : $(LIB_OBJS)
	$(CC) $(CCSHAREDFLAG) $(CFLAGS) $(LIB_OBJS) -o $@ $(ANDOR_LDFLAGS)

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
	$(RM) $(RM_OPTIONS) $(EXES) $(OBJS) $(LT_LIB_HOME)/$(ANDOR_LIBRARYNAME).so $(TIDY_OPTIONS)

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

/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/gnu/stubs-32.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/asm-generic/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/asm-generic/errno-base.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/time.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/unistd.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /usr/include/getopt.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /home/dev/src/andor/andor-2.85.30000/include/atmcdLXd.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /home/dev/src/autoguider/ccd/andor/include/andor_general.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_general.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/gnu/stubs-32.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/asm-generic/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/asm-generic/errno-base.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/stdlib.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/time.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/unistd.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /usr/include/getopt.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /home/dev/src/andor/andor-2.85.30000/include/atmcdLXd.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /home/dev/src/log_udp/include/log_udp.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /home/dev/src/autoguider/ccd/andor/include/andor_setup.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /home/dev/src/autoguider/ccd/andor/include/andor_general.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_setup.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/gnu/stubs-32.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/asm-generic/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/asm-generic/errno-base.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/stdlib.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/time.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/unistd.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /usr/include/getopt.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /home/dev/src/andor/andor-2.85.30000/include/atmcdLXd.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /home/dev/src/log_udp/include/log_udp.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_exposure.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /home/dev/src/autoguider/ccd/andor/include/andor_setup.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /home/dev/src/autoguider/ccd/andor/include/andor_general.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /home/dev/src/autoguider/ccd/andor/include/andor_temperature.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_exposure.o: /home/dev/src/autoguider/ccd/andor/include/andor_exposure.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/gnu/stubs-32.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/asm-generic/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/asm-generic/errno-base.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /home/dev/src/andor/andor-2.85.30000/include/atmcdLXd.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /home/dev/src/log_udp/include/log_udp.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/time.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /home/dev/src/autoguider/ccd/andor/include/andor_general.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_temperature.o: /home/dev/src/autoguider/ccd/andor/include/andor_temperature.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/features.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/gnu/stubs-32.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/asm-generic/errno.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/asm-generic/errno-base.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/libio.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/string.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /home/dev/src/log_udp/include/log_udp.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /home/dev/src/autoguider/ccd/include/ccd_general.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/time.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /home/dev/src/autoguider/ccd/andor/include/andor_driver.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /home/dev/src/autoguider/ccd/include/ccd_driver.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /home/dev/src/autoguider/ccd/include/ccd_setup.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /home/dev/src/autoguider/ccd/include/ccd_temperature.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /home/dev/src/autoguider/ccd/andor/include/andor_exposure.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /home/dev/src/autoguider/ccd/andor/include/andor_general.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /home/dev/src/autoguider/ccd/include/ccd_config.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /home/dev/src/autoguider/ccd/andor/include/andor_setup.h
/home/dev/bin/autoguider/ccd/andor/c/i386-linux/andor_driver.o: /home/dev/src/autoguider/ccd/andor/include/andor_temperature.h

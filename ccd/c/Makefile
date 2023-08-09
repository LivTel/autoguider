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

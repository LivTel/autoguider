# $Header: /home/cjm/cvs/autoguider/ccd/fli/c/Makefile,v 1.1 2013-11-26 16:28:44 cjm Exp $

include ../../../../Makefile.common
include ../../../Makefile.common
include ../../Makefile.common
include ../Makefile.common

BINDIR			= $(AUTOGUIDER_FLI_BIN_HOME)/c/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_FLI_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_FLI_DOC_HOME)/cdocs

#DEBUG_CFLAGS		= 
DEBUG_CFLAGS		= -DFLI_DEBUG

# autoguider ccd (general) library
CCD_CFLAGS 		= -I$(AUTOGUIDER_CCD_SRC_HOME)/include
CCD_LDFLAGS 		= -lautoguider_ccd_general

# log_udp library (log_udp.h is included for verbosity settings)
LOG_UDP_CFLAGS		= -I$(LOG_UDP_SRC_HOME)/include

# fli library
FLI_CFLAGS		= -I$(AUTOGUIDER_SRC_HOME)/$(AUTOGUIDER_FLI_HOME)/libfli-1.104/
FLI_LDFLAGS		= -L$(AUTOGUIDER_SRC_HOME)/$(AUTOGUIDER_FLI_HOME)/libfli-1.104/ -lfli

CFLAGS 			= -g -I$(INCDIR) $(DEBUG_CFLAGS) $(FLI_CFLAGS) $(CCD_CFLAGS) $(LOG_UDP_CFLAGS)
DOCFLAGS 		= -static
LIB_SRCS		= fli_setup.c fli_exposure.c fli_temperature.c fli_driver.c
# fli_general.c 
SRCS			= $(LIB_SRCS)
LIB_HEADERS		= $(LIB_SRCS:%.c=$(INCDIR)/%.h)
HEADERS			= $(LIB_HEADERS)
LIB_OBJS		= $(LIB_SRCS:%.c=$(BINDIR)/%.o)
OBJS			= $(SRCS:%.c=$(BINDIR)/%.o)
DOCS 			= $(SRCS:%.c=$(DOCSDIR)/%.html)

top: $(LT_LIB_HOME)/$(FLI_LIBRARYNAME).so docs

$(LT_LIB_HOME)/$(FLI_LIBRARYNAME).so : $(LIB_OBJS)
	$(CC) $(CCSHAREDFLAG) $(CFLAGS) $(LIB_OBJS) -o $@ $(FLI_LDFLAGS)

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

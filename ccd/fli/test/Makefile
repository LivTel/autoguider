# $Header: /home/cjm/cvs/autoguider/ccd/fli/test/Makefile,v 1.2 2013-11-26 17:07:13 cjm Exp $

include ../../../../Makefile.common
include ../../../Makefile.common
include ../../Makefile.common
include ../Makefile.common

BINDIR			= $(AUTOGUIDER_FLI_BIN_HOME)/test/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_FLI_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_FLI_DOC_HOME)/test

# fli library
FLI_CFLAGS 		= -I$(AUTOGUIDER_NOGIT_HOME)/$(AUTOGUIDER_FLI_HOME)/libfli-1.104/
FLI_LDFLAGS 		= -L$(AUTOGUIDER_NOGIT_HOME)/$(AUTOGUIDER_FLI_HOME)/libfli-1.104/ -lfli

# CFITSIO library
CFITSIO_CFLAGS		= -I$(CFITSIOINCDIR) 
CFITSIO_LDFLAGS		= -L$(LT_LIB_HOME) -lcfitsio

CFLAGS 			= -g -I$(INCDIR) $(FLI_CFLAGS) $(CFITSIO_CFLAGS)
#$(DEBUG_CFLAGS) $(CCD_CFLAGS) 

EXE_SRCS		= test_temperature_low_level.c test_exposure_low_level.c
SRCS			= $(EXE_SRCS)
EXE_OBJS		= $(EXE_SRCS:%.c=$(BINDIR)/%.o)
EXES			= $(EXE_SRCS:%.c=$(BINDIR)/%)
DOCS 			= $(SRCS:%.c=$(DOCSDIR)/%.html)

top: $(EXES) docs

$(BINDIR)/%: $(BINDIR)/%.o
	$(CC) $< -o $@  $(CFITSIO_LDFLAGS) $(FLI_LDFLAGS) -lfli

$(BINDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@  

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

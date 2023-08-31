# Makefile
# $Id$

include ../../../Makefile.common
include ../../Makefile.common
include ../Makefile.common

BINDIR			= $(AUTOGUIDER_CCD_BIN_HOME)/test/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_CCD_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_CCD_DOC_HOME)/test


# autoguider ccd (general) library
CCD_LDFLAGS 		= -lautoguider_ccd_general

# CFITSIO library
CFITSIO_CFLAGS		= -I$(CFITSIOINCDIR) 
CFITSIO_LDFLAGS		= -L$(LT_LIB_HOME) -lcfitsio

CFLAGS 			= -g -I$(INCDIR) $(CFITSIO_CFLAGS)
LDFLAGS			= $(CCD_LDFLAGS) -lcfitsio

EXE_SRCS		= test_setup_startup.c test_setup_dimensions.c test_exposure.c test_temperature.c
SRCS			= $(EXE_SRCS)
EXE_OBJS		= $(EXE_SRCS:%.c=$(BINDIR)/%.o)
EXES			= $(EXE_SRCS:%.c=$(BINDIR)/%)
DOCS 			= $(SRCS:%.c=$(DOCSDIR)/%.html)
CONFIG_SRCS		= pco.properties andor.properties
CONFIG_BINS		= $(CONFIG_SRCS:%.properties=$(BINDIR)/%.properties)

top: $(EXES) $(CONFIG_BINS) docs

$(BINDIR)/%: $(BINDIR)/%.o
	$(CC) $< -o $@  $(CFITSIO_LDFLAGS) -lm -lc $(LDFLAGS)

$(BINDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BINDIR)/%.properties: %.properties
	$(CP) $< $@  

docs: $(DOCS)

$(DOCS): $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -- $(CFLAGS) -- $(SRCS)

clean:
	$(RM) $(RM_OPTIONS) $(OBJS) $(PROGS) $(TIDY_OPTIONS)

tidy:
	$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)


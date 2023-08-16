# Makefile
# $Id$

include ../../../../Makefile.common
include ../../../Makefile.common
include ../../Makefile.common
include ../Makefile.common

BINDIR			= $(AUTOGUIDER_PCO_BIN_HOME)/test/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_PCO_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_PCO_DOC_HOME)/test

# CFITSIO library
CFITSIO_CFLAGS		= -I$(CFITSIOINCDIR) 
CFITSIO_LDFLAGS		= -L$(LT_LIB_HOME) -lcfitsio

CFLAGS 			= -g -I$(INCDIR) $(PCO_CFLAGS) $(CFITSIO_CFLAGS)
LDFLAGS			= $(PCO_LDFLAGS) -lcfitsio -lstdc++

EXE_SRCS		=
SRCS			= $(EXE_SRCS)
EXE_OBJS		= $(EXE_SRCS:%.c=$(BINDIR)/%.o)
EXES			= $(EXE_SRCS:%.c=$(BINDIR)/%)
DOCS 			= $(SRCS:%.c=$(DOCSDIR)/%.html)

top: $(EXES) docs

$(BINDIR)/%: $(BINDIR)/%.o
	$(CC) $< -o $@  $(CFITSIO_LDFLAGS) $(PCO_LDFLAGS) -lm -lc $(LDFLAGS)

$(BINDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@  
docs: $(DOCS)

$(DOCS): $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -- $(CFLAGS) -- $(SRCS)

clean:
	$(RM) $(RM_OPTIONS) $(OBJS) $(PROGS) $(TIDY_OPTIONS)

tidy:
	$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)


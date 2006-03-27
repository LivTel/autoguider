# $Header: /home/cjm/cvs/autoguider/ccd/andor/c/Makefile,v 1.1 2006-03-27 14:03:07 cjm Exp $

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

# andor library
# need to link to standard c++ library as autoguider_ccd_andor links against andor-gcc3
# which links against stdc++
ANDOR_CFLAGS 		= -I$(AUTOGUIDER_SRC_HOME)/andor/andor/examples/common/
ANDOR_LDFLAGS 		= -L$(AUTOGUIDER_SRC_HOME)/andor/andor/examples/common/ -landor-gcc3 -lstdc++ -lpthread -ldl

# CFITSIO library
CFITSIO_CFLAGS		= -I$(CFITSIOINCDIR) 
CFITSIO_LDFLAGS		= -L$(LT_LIB_HOME) -lcfitsio

CFLAGS 			= -g -I$(INCDIR) $(DEBUG_CFLAGS) $(ANDOR_CFLAGS) $(CCD_CFLAGS) $(CFITSIO_CFLAGS)
DOCFLAGS 		= -static

LIB_SRCS		= andor_general.c andor_setup.c andor_exposure.c andor_temperature.c
SRCS			= $(LIB_SRCS)
LIB_HEADERS		= $(LIB_SRCS:%.c=$(INCDIR)/%.h)
HEADERS			= $(LIB_HEADERS)
LIB_OBJS		= $(LIB_SRCS:%.c=$(BINDIR)/%.o)
OBJS			= $(SRCS:%.c=$(BINDIR)/%.o)
DOCS 			= $(SRCS:%.c=$(DOCSDIR)/%.html)

top: $(LT_LIB_HOME)/$(ANDOR_LIBRARYNAME).so docs

static: $(LT_LIB_HOME)/$(ANDOR_LIBRARYNAME).so

$(LT_LIB_HOME)/$(ANDOR_LIBRARYNAME).so : $(LIB_OBJS)
	$(CC) $(CCSHAREDFLAG) $(CFLAGS) $(LIB_OBJS) -o $@ $(ANDOR_LDFLAGS) $(CFITSIO_LDFLAGS)

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
	makedepend -- $(CFLAGS) -- $(SRCS)

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


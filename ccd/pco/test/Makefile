# Makefile
# $Id$

include ../../../../Makefile.common
include ../../../Makefile.common
include ../../Makefile.common
include ../Makefile.common

BINDIR			= $(AUTOGUIDER_PCO_BIN_HOME)/test/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_PCO_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_PCO_DOC_HOME)/test

#
# PCO SDK
#
PCO_LOW_LEVEL_CFLAGS	= -I$(LT_SRC_HOME)/autoguider-nogit/pco/pco_camera_1_1_19/pco_usb_pl/pco_classes \
		  -I$(LT_SRC_HOME)/autoguider-nogit/pco/pco_camera_1_1_19/pco_common/pco_classes/ \
		  -I$(LT_SRC_HOME)/autoguider-nogit/pco/pco_camera_1_1_19/pco_common/ \
		  -I$(LT_SRC_HOME)/autoguider-nogit/pco/pco_camera_1_1_19/pco_common/pco_include/

PCO_LOW_LEVEL_LIBDIR	= $(LT_SRC_HOME)/autoguider-nogit/pco/pco_camera_1_1_19/pco_usb_pl/lib/
PCO_LOW_LEVEL_LDFLAGS	= -L$(PCO_LOW_LEVEL_LIBDIR) -lpcocam_usb -lusb-1.0 -pthread

# PCO autoguider driver library
PCO_LDFLAGS		= -l$(AUTOGUIDER_HOME)_$(CCD_HOME)_$(PCO_HOME)

# autoguider ccd (general) library
CCD_CFLAGS 		= -I$(AUTOGUIDER_CCD_SRC_HOME)/include
CCD_LDFLAGS 		= -lautoguider_ccd_general

# CFITSIO library
CFITSIO_CFLAGS		= -I$(CFITSIOINCDIR) 
CFITSIO_LDFLAGS		= -L$(LT_LIB_HOME) -lcfitsio

CFLAGS 			= -g -I$(INCDIR) $(CCD_CFLAGS) $(CFITSIO_CFLAGS)
LDFLAGS			=  $(PCO_LDFLAGS) $(CCD_LDFLAGS) -lcfitsio -lstdc++

EXE_SRCS		= pco_test_get_roi_step_size.c pco_test_exposure.c pco_test_initialise_finalise.c
SRCS			= $(EXE_SRCS)
EXE_OBJS		= $(EXE_SRCS:%.c=$(BINDIR)/%.o)
EXES			= $(EXE_SRCS:%.c=$(BINDIR)/%)
DOCS 			= $(SRCS:%.c=$(DOCSDIR)/%.html)
CONFIG_SRCS		= pco.properties
CONFIG_BINS		= $(CONFIG_SRCS:%.properties=$(BINDIR)/%.properties)

top: $(EXES) $(BINDIR)/pco_test_initialise_finalise_low_level $(CONFIG_BINS) docs

$(BINDIR)/pco_test_initialise_finalise_low_level: pco_test_initialise_finalise_low_level.cpp
	g++ $(PCO_LOW_LEVEL_CFLAGS) pco_test_initialise_finalise_low_level.cpp -o $(BINDIR)/pco_test_initialise_finalise_low_level $(PCO_LOW_LEVEL_LDFLAGS) -lrt

$(BINDIR)/%: $(BINDIR)/%.o
	$(CC) $< -o $@  $(CFITSIO_LDFLAGS) $(PCO_LDFLAGS) -lm -lc $(LDFLAGS)

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


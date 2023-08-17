# Makefile
# $Id: Makefile | Makefile | Makefile $

include ../../../../Makefile.common
include ../../../Makefile.common
include ../../Makefile.common
include ../Makefile.common


BINDIR			= $(AUTOGUIDER_PCO_BIN_HOME)/c/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_PCO_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_PCO_DOC_HOME)/cdocs

#DEBUG_CFLAGS		= 
DEBUG_CFLAGS		= -DPCO_DEBUG

# autoguider ccd (general) library
CCD_CFLAGS 		= -I$(AUTOGUIDER_CCD_SRC_HOME)/include
CCD_LDFLAGS 		= -lautoguider_ccd_general

# log_udp library (log_udp.h is included for verbosity settings)
LOG_UDP_CFLAGS		= -I$(LOG_UDP_SRC_HOME)/include

CFLAGS 			= -g -I$(INCDIR) $(DEBUG_CFLAGS) $(PCO_CFLAGS) $(CCD_CFLAGS) $(LOG_UDP_CFLAGS) \
			  $(SHARED_LIB_CFLAGS)
DOCFLAGS 		= -static
LIB_SRCS		= pco_driver.cpp
# pco_setup.cpp pco_exposure.cpp pco_temperature.cpp


SRCS			= $(LIB_SRCS)
LIB_HEADERS		= $(LIB_SRCS:%.c=$(INCDIR)/%.h)
HEADERS			= $(LIB_HEADERS)
LIB_OBJS		= $(LIB_SRCS:%.cpp=$(BINDIR)/%.o)
OBJS			= $(SRCS:%.cpp=$(BINDIR)/%.o)
DOCS 			= $(SRCS:%.cpp=$(DOCSDIR)/%.html)

top: $(LT_LIB_HOME)/$(PCO_LIBRARYNAME).so docs

$(LT_LIB_HOME)/$(PCO_LIBRARYNAME).so : $(LIB_OBJS)
	g++ $(CCSHAREDFLAG) $(LIB_OBJS) -o $@ $(PCO_LDFLAGS)

$(BINDIR)/%.o: %.cpp
	g++ -c $(CFLAGS) $< -o $@  

docs: $(DOCS)

$(DOCS): $(SRCS) $(HEADERS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -- $(CFLAGS) -- $(SRCS)

clean:
	$(RM) $(RM_OPTIONS) $(OBJS) $(LT_LIB_HOME)/$(PCO_LIBRARYNAME).so $(TIDY_OPTIONS)

tidy:
	$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)
	-(cd $(INCDIR); $(RM) $(RM_OPTIONS) $(TIDY_OPTIONS);)

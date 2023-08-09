# $Header: /home/cjm/cvs/autoguider/commandserver/test/Makefile,v 1.6 2011-09-07 11:09:20 cjm Exp $

include ../../../Makefile.common
include ../../Makefile.common
include ../Makefile.common

TEST_HOME		= test
BINDIR			= $(AUTOGUIDER_COMMANDSERVER_BIN_HOME)/$(TEST_HOME)/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_COMMANDSERVER_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_COMMANDSERVER_DOC_HOME)/$(TEST_HOME)

# log_udp library (log_udp.h is included for verbosity settings)
LOG_UDP_CFLAGS		= -I$(LOG_UDP_SRC_HOME)/include

#DEBUG_CFLAGS	= 
DEBUG_CFLAGS	= -DCOMMAND_SERVER_DEBUG=5
#-DGETFITS_DEBUG
#-DGETFITS_DEBUG2

CFLAGS 		= -g -I$(INCDIR) -I$(CFITSIOINCDIR) $(DEBUG_CFLAGS) $(LOG_UDP_CFLAGS)
DOCFLAGS 	= -static

EXE_SRCS	= test_server.c send_command.c test_getfits_command.c
SRCS		= $(EXE_SRCS)
EXE_OBJS	= $(EXE_SRCS:%.c=$(BINDIR)/%.o)
OBJS		= $(SRCS:%.c=$(BINDIR)/%.o)
EXES		= $(EXE_SRCS:%.c=$(BINDIR)/%)
DOCS 		= $(SRCS:%.c=$(DOCSDIR)/%.html)

top: $(EXES) docs

$(BINDIR)/test_server: $(BINDIR)/test_server.o
	$(CC) $< -o $@  -L$(LT_LIB_HOME) -lautoguider_commandserver -lcfitsio $(TIMELIB) $(SOCKETLIB) -lm -lc

$(BINDIR)/%: $(BINDIR)/%.o
	$(CC) $< -o $@  -L$(LT_LIB_HOME) -lautoguider_commandserver $(TIMELIB) $(SOCKETLIB) -lm -lc

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

checkout:
	-$(CO) $(CO_OPTIONS) $(SRCS)

# DO NOT DELETE

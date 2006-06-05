# $Header: /home/cjm/cvs/autoguider/ngatcil/test/Makefile,v 1.1 2006-06-01 15:28:15 cjm Exp $

include ../../../Makefile.common
include ../../Makefile.common
include ../../commandserver/Makefile.common
include ../Makefile.common

TEST_HOME		= test
BINDIR			= $(AUTOGUIDER_NGATCIL_BIN_HOME)/$(TEST_HOME)/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_NGATCIL_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_NGATCIL_DOC_HOME)/$(TEST_HOME)

#DEBUG_CFLAGS	= 
DEBUG_CFLAGS	= -DNGATCIL_DEBUG=10

# ngatcil library
NGATCIL_LDFLAGS		= -lautoguider_ngatcil

# command_server library
COMMAND_SERVER_CFLAGS 	= -I$(AUTOGUIDER_COMMANDSERVER_SRC_HOME)/include
COMMAND_SERVER_LDFLAGS 	= -lautoguider_commandserver

CFLAGS 		= -g -I$(INCDIR) $(DEBUG_CFLAGS) $(COMMAND_SERVER_CFLAGS)
DOCFLAGS 	= -static

EXE_SRCS	= test_tcs.c
SRCS		= $(EXE_SRCS)
EXE_OBJS	= $(EXE_SRCS:%.c=$(BINDIR)/%.o)
OBJS		= $(SRCS:%.c=$(BINDIR)/%.o)
EXES		= $(EXE_SRCS:%.c=$(BINDIR)/%)
DOCS 		= $(SRCS:%.c=$(DOCSDIR)/%.html)

top: $(EXES) docs

$(BINDIR)/%: $(BINDIR)/%.o
	$(CC) $< -o $@  -L$(LT_LIB_HOME) $(NGATCIL_LDFLAGS) $(COMMAND_SERVER_LDFLAGS) $(TIMELIB) $(SOCKETLIB) -lm -lc

$(BINDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@  

docs: $(DOCS)

$(DOCS): $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

$(DOCS) : $(SRCS)

depend:
	makedepend -p$(BINDIR)/ -- $(CFLAGS) -- $(SRCS)

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

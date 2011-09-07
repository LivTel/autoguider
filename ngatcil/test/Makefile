# $Header: /home/cjm/cvs/autoguider/ngatcil/test/Makefile,v 1.5 2011-09-07 11:24:59 cjm Exp $

include ../../../Makefile.common
include ../../Makefile.common
include ../../commandserver/Makefile.common
include ../Makefile.common

TEST_HOME		= test
BINDIR			= $(AUTOGUIDER_NGATCIL_BIN_HOME)/$(TEST_HOME)/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_NGATCIL_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_NGATCIL_DOC_HOME)/$(TEST_HOME)

# log_udp library (log_udp.h is included for verbosity settings)
LOG_UDP_CFLAGS		= -I$(LOG_UDP_SRC_HOME)/include

#DEBUG_CFLAGS	= 
DEBUG_CFLAGS	= -DNGATCIL_DEBUG=10

# ngatcil library
NGATCIL_LDFLAGS		= -lautoguider_ngatcil

# command_server library
COMMAND_SERVER_CFLAGS 	= -I$(AUTOGUIDER_COMMANDSERVER_SRC_HOME)/include
COMMAND_SERVER_LDFLAGS 	= -lautoguider_commandserver

CFLAGS 		= -g -I$(INCDIR) $(DEBUG_CFLAGS) $(COMMAND_SERVER_CFLAGS) $(LOG_UDP_CFLAGS)
DOCFLAGS 	= -static

EXE_SRCS	= test_tcs.c test_autoguider.c test_sdb.c test_cil_server.c test_raw_send.c
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

/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/features.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/libio.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/string.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_udp_raw.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_tcs_guide_packet.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_cil.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /usr/include/stdint.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_tcs.o: /home/dev/src/autoguider/commandserver/include/command_server.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/errno.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/features.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/math.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/huge_val.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/mathdef.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/mathcalls.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/pthread.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/sched.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/time.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/signal.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/initspin.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/sigthread.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/libio.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/string.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_udp_raw.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_tcs_guide_packet.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_cil.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /usr/include/stdint.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_autoguider.o: /home/dev/src/autoguider/commandserver/include/command_server.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/features.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/libio.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/string.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_cil.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /usr/include/stdint.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_udp_raw.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_sdb.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_ags_sdb.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/features.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/libio.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/string.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/time.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_cil.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /usr/include/stdint.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_cil_server.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_udp_raw.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/errno.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/features.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/netdb.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/netinet/in.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/stdint.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stddef.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/socket.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/limits.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/limits.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/posix1_lim.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/local_lim.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/linux/limits.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/posix2_lim.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/sys/types.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/time.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/sockaddr.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/asm/socket.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/asm/sockios.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/in.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/endian.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/endian.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/byteswap.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/netdb.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/libio.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/lib/gcc-lib/i386-redhat-linux/3.2.2/include/stdarg.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/string.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/arpa/inet.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/sys/socket.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/sys/uio.h
/home/dev/bin/autoguider/ngatcil/test/i386-linux/test_raw_send.o: /usr/include/bits/uio.h

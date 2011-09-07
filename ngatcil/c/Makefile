# $Header: /home/cjm/cvs/autoguider/ngatcil/c/Makefile,v 1.5 2011-09-07 11:23:56 cjm Exp $

include ../../../Makefile.common
include ../../Makefile.common
include ../Makefile.common

BINDIR			= $(AUTOGUIDER_NGATCIL_BIN_HOME)/c/$(HOSTTYPE)
INCDIR 			= $(AUTOGUIDER_NGATCIL_SRC_HOME)/include
DOCSDIR 		= $(AUTOGUIDER_NGATCIL_DOC_HOME)/cdocs

# log_udp library (log_udp.h is included for verbosity settings)
LOG_UDP_CFLAGS		= -I$(LOG_UDP_SRC_HOME)/include

#DEBUG_CFLAGS	= 
DEBUG_CFLAGS	= -DNGATCIL_DEBUG=10

CFLAGS 		= -g -I$(INCDIR) $(DEBUG_CFLAGS) $(LOG_UDP_CFLAGS)
DOCFLAGS 	= -static

LIB_SRCS	=ngatcil_general.c ngatcil_udp_raw.c ngatcil_tcs_guide_packet.c ngatcil_cil.c ngatcil_ags_sdb.c
SRCS		= $(LIB_SRCS)
LIB_HEADERS	= $(LIB_SRCS:%.c=$(INCDIR)/%.h)
HEADERS		= $(LIB_HEADERS)
LIB_OBJS	= $(LIB_SRCS:%.c=$(BINDIR)/%.o)
OBJS		= $(SRCS:%.c=$(BINDIR)/%.o)
DOCS 		= $(SRCS:%.c=$(DOCSDIR)/%.html)

top: $(LT_LIB_HOME)/$(LIBRARYNAME).so docs

$(LT_LIB_HOME)/$(LIBRARYNAME).so : $(LIB_OBJS)
	$(CC) $(CCSHAREDFLAG) $(CFLAGS) $(LIB_OBJS) -o $@

$(LT_LIB_HOME)/$(LIBRARYNAME).a : $(LIB_OBJS)
	ar rcv $@ $?

$(BINDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@  

docs: $(DOCS)

$(DOCS): $(SRCS)
	-$(CDOC) -d $(DOCSDIR) -h $(INCDIR) $(DOCFLAGS) $(SRCS)

$(DOCS) : $(SRCS)

depend:
	makedepend $(MAKEDEPENDFLAGS) -p$(BINDIR)/ -- $(CFLAGS) -- $(SRCS)

clean:
	$(RM) $(RM_OPTIONS) $(EXES) $(OBJS) $(LT_LIB_HOME)/$(LIBRARYNAME).so $(TIDY_OPTIONS)

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

#
# $Log: not supported by cvs2svn $
# Revision 1.4  2009/01/30 18:01:10  cjm
# Added log_udp include file.
#
# Revision 1.3  2006/07/20 15:16:42  cjm
# Added ngatcil_ags_sdb.c.
#
# Revision 1.2  2006/06/06 10:34:01  cjm
# Added ngatcil_cil.c.
#
# Revision 1.1  2006/06/01 15:28:13  cjm
# Initial revision
#
#

# DO NOT DELETE

/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/errno.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/features.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/gnu/stubs-32.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/asm-generic/errno.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/asm-generic/errno-base.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/libio.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/stdlib.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/string.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/time.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/unistd.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/bits/posix_opt.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/bits/confname.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /usr/include/getopt.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_general.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/errno.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/features.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/gnu/stubs-32.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/errno.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/linux/errno.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/asm/errno.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/asm-generic/errno.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/asm-generic/errno-base.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/netdb.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/netinet/in.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/stdint.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/sys/socket.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/sys/uio.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/sys/types.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/time.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/uio.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/socket.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/limits.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/posix1_lim.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/local_lim.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/linux/limits.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/posix2_lim.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/sockaddr.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/asm/socket.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/asm/sockios.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/in.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/endian.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/endian.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/byteswap.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/netdb.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/pthread.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/sched.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/sched.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/signal.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/sigset.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/pthreadtypes.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/setjmp.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/libio.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/stdlib.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/string.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /usr/include/arpa/inet.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /home/dev/src/log_udp/include/log_udp.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_udp_raw.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_udp_raw.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/math.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/features.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/gnu/stubs-32.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/bits/huge_val.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/bits/mathdef.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/bits/mathcalls.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/libio.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /usr/include/string.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /home/dev/src/log_udp/include/log_udp.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_udp_raw.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_tcs_guide_packet.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_tcs_guide_packet.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/features.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/gnu/stubs-32.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/libio.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/string.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/time.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /home/dev/src/log_udp/include/log_udp.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_udp_raw.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_cil.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_cil.o: /usr/include/stdint.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/limits.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/features.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/sys/cdefs.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/bits/wordsize.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/gnu/stubs.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/gnu/stubs-32.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/bits/posix1_lim.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/bits/local_lim.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/linux/limits.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/bits/posix2_lim.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/stdint.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/bits/wchar.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/stdio.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/bits/types.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/bits/typesizes.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/libio.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/_G_config.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/wchar.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/gconv.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/bits/stdio_lim.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/bits/sys_errlist.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/string.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/time.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /usr/include/bits/time.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /home/dev/src/log_udp/include/log_udp.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_ags_sdb.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_general.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_udp_raw.h
/home/dev/bin/autoguider/ngatcil/c/i386-linux/ngatcil_ags_sdb.o: /home/dev/src/autoguider/ngatcil/include/ngatcil_cil.h

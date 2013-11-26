# Makefile
# $Header: /home/cjm/cvs/autoguider/ccd/Makefile,v 1.3 2013-11-26 16:27:36 cjm Exp $

include ../../Makefile.common
include ../Makefile.common

DIRS = c andor fli test 
top:
	@for i in $(DIRS); \
	do \
		(echo making in $$i...; cd $$i; $(MAKE) ); \
	done;

checkin:
	-@for i in $(DIRS); \
	do \
		(echo checkin in $$i...; cd $$i; $(MAKE) checkin; $(CI) $(CI_OPTIONS) Makefile); \
	done;

checkout:
	@for i in $(DIRS); \
	do \
		(echo checkout in $$i...; cd $$i; $(CO) $(CO_OPTIONS) Makefile; $(MAKE) checkout); \
	done;

depend:
	@for i in andor c test; \
	do \
		(echo depend in $$i...; cd $$i; $(MAKE) depend);\
	done;

clean:
	$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)
	@for i in $(DIRS); \
	do \
		(echo clean in $$i...; cd $$i; $(MAKE) clean); \
	done;

tidy:
	$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)
	@for i in $(DIRS); \
	do \
		(echo tidy in $$i...; cd $$i; $(MAKE) tidy); \
	done;

backup: checkin
	@for i in $(DIRS); \
	do \
		(echo backup in $$i...; cd $$i; $(MAKE) backup); \
	done;
	$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)
#
# $Log: not supported by cvs2svn $
# Revision 1.2  2011/09/07 12:13:06  cjm
# Swapped directory order, andor test programs links to libautoguider_ccd_general.
#
# Revision 1.1  2006/06/01 15:28:03  cjm
# Initial revision
#
#

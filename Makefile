# Makefile
# $Header: /home/cjm/cvs/autoguider/Makefile,v 1.1 2011-09-07 11:29:11 cjm Exp $

include ../Makefile.common

DIRS = commandserver ccd ngatcil c java
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

# We should only do a make depend after checking in (all Makefiles).
# This is because we now do make depend on ltdevsrv as eng, and edit the Makefiles on ltobs9 as cjm.
# A make depend on ltdevsrv changes the Makefiles to be owned by eng, and any checked out by cjm
# then cannot be edited by cjm, or checked in by eng.
depend: checkin
	@for i in $(DIRS); \
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
#

# $Header: /home/cjm/cvs/autoguider/docs/Makefile,v 1.1 2011-09-07 11:29:30 cjm Exp $

include ../../Makefile.common
include ../Makefile.common

DOCSDIR 	= $(AUTOGUIDER_DOC_HOME)/latex
TEXSRCS 	= autoguider_changes.tex
EPS_SRCS	= iss_ag_case1.eps

DVIOBJS		= $(TEXSRCS:%.tex=%.dvi)
PSOBJS		= $(TEXSRCS:%.tex=$(DOCSDIR)/%.ps)

top: $(PSOBJS)

$(DOCSDIR)/%.ps: %.dvi
	$(DVIPS) $< -o
	$(MV) $*.ps $(DOCSDIR)

%.dvi: %.tex
	$(LATEX) $<
	$(LATEX) $<
	$(LATEX) $<

depend:
	echo "NO depend rule for latex files."

clean:
	$(RM) $(RM_OPTIONS) $(DVIOBJS) $(PSOBJS) $(EPSOBJS) $(LATEX_TIDY_OPTIONS) $(TIDY_OPTIONS)

tidy:
	$(RM) $(RM_OPTIONS) $(LATEX_TIDY_OPTIONS) $(TIDY_OPTIONS)

backup:
	$(RM) $(RM_OPTIONS) $(DVIOBJS) $(EPSOBJS) $(LATEX_TIDY_OPTIONS) $(TIDY_OPTIONS)

checkin:
	-$(CI) $(CI_OPTIONS) $(TEXSRCS) $(FIGSRCS) $(CONTROL_PS_SRCS)

checkout:
	-$(CO) $(CO_OPTIONS) $(TEXSRCS) $(FIGSRCS) $(CONTROL_PS_SRCS)

#
# $Log: not supported by cvs2svn $
#

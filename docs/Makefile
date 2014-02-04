# $Header: /home/cjm/cvs/autoguider/docs/Makefile,v 1.2 2014-02-04 16:34:44 cjm Exp $

include ../../Makefile.common
include ../Makefile.common

PS2PDF		= ps2pdf


DOCSDIR 	= $(AUTOGUIDER_DOC_HOME)/latex
TEXSRCS 	= autoguider_changes.tex autoguider_overview.tex iucaaag_installation.tex autoguider_user_guide.tex \
		autoguider_source_code.tex autoguider_configuration_guide.tex iucaaag_software_installation.tex 
EPS_SRCS	= iss_ag_case1.eps
PNG_SRCS	= guide_packets_with_time_346_16_xy.png tcs_display_screen_1.png tcs_display_screen_2.png \
		Network_Connections_1.png Network_Connections_2.png

DVIOBJS		= $(TEXSRCS:%.tex=%.dvi)
PSOBJS		= $(TEXSRCS:%.tex=$(DOCSDIR)/%.ps)
PDFOBJS		= $(TEXSRCS:%.tex=$(DOCSDIR)/%.pdf)

top: $(PSOBJS) $(PDFOBJS)

$(DOCSDIR)/%.pdf: $(DOCSDIR)/%.ps
	$(PS2PDF) $< $@

$(DOCSDIR)/%.ps: %.dvi
	$(DVIPS) $< -o
	$(MV) $*.ps $(DOCSDIR)

%.dvi: %.tex
	$(LATEX) $<
	$(LATEX) $<
	$(LATEX) $<

autoguider_user_guide.dvi: guide_packets_with_time_346_16_xy.eps tcs_display_screen_1.eps tcs_display_screen_2.eps

autoguider_overview.dvi: autoguider_field_guide_loops.eps

autoguider_source_code.dvi: autoguider_directory_structure1.eps

iucaaag_installation.dvi: Network_Connections_1.eps Network_Connections_2.eps ag_rotation_20091109.eps

%.eps: %.png
	$(CONVERT) $< $@

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
# Revision 1.1  2011/09/07 11:29:30  cjm
# Initial revision
#
#

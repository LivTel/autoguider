# Makefile
# $Header: /home/cjm/cvs/autoguider/java/Makefile,v 1.1 2009-01-30 18:02:11 cjm Exp $
# $Revision: 1.1 $
include ../Makefile.common
include ../../Makefile.common

MAKE 		= make
DIRS 		= ngat/autoguider/command
JAR_FILE       	= ngat_autoguider.jar
JARLIBDIR	= $(LT_JAVALIB_HOME)
LIBDIR		= $(AUTOGUIDER_BIN_HOME)/java/
DOCSDIR 	= $(AUTOGUIDER_DOC_HOME)/javadocs/
DOCFLAGS 	= -version -author -private
JAVASRCS 	= ngat/autoguider/command/*.java ngat/autoguider/gui/*.java 
JAVAPKGS	= ngat.autoguider.command ngat.autoguider.gui
JAVADOCS 	= $(JAVASRCS:%.java=$(DOCSDIR)/%.html)
OBJS 		= $(JAVASRCS:%.java=$(LIBDIR)/%.class)

top: $(JARLIBDIR)/$(JAR_FILE) javadocs

$(JARLIBDIR)/$(JAR_FILE): $(OBJS)
	( cd $(LIBDIR); $(JAR) $(JAR_OPTIONS) $(JARLIBDIR)/$(JAR_FILE) ngat)

$(OBJS): $(JAVASRCS)
	$(JAVAC) -sourcepath .:$(LT_SRC_HOME) -classpath $(LIBDIR):$(CLASSPATH) $(JAVAC_OPTIONS) -d $(LIBDIR) $^

#
# Do all javadocs at top level, to ensure javadoc creates package links correctly
#
javadocs: $(JAVADOCS)

$(JAVADOCS): $(JAVASRCS)
	$(JAVADOC) -sourcepath $(LT_SRC_HOME):$(AUTOGUIDER_SRC_HOME)/java -d $(DOCSDIR) $(DOCFLAGS) \
		-link http://java.sun.com/j2se/1.4.2/docs/api/ $(JAVAPKGS)

checkin:
	@for i in $(DIRS); \
	do \
		(echo checkin in $$i...; cd $$i; $(CI) $(CI_OPTIONS) *.java; $(CI) $(CI_OPTIONS) Makefile ); \
	done;

checkout:
	@for i in $(DIRS); \
	do \
		(echo checkout in $$i...; cd $$i; $(CO) $(CO_OPTIONS) *.java; $(CO) $(CO_OPTIONS) Makefile ); \
	done;

depend:
	echo no depend in ngat directory.

clean:
	-$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)
	-$(RM) $(RM_OPTIONS) $(JARLIBDIR)/$(JAR_FILE)
	@for i in $(DIRS); \
	do \
		(echo clean in $$i...; cd $$i; $(MAKE) clean); \
	done;
	$(RM) $(RM_OPTIONS) $(JAVADOCS)
	$(RM) $(RM_OPTIONS) -r $(DOCSDIR)/*

tidy:
	-$(RM) $(RM_OPTIONS) $(TIDY_OPTIONS)
	@for i in $(DIRS); \
	do \
		(echo tidy in $$i...; cd $$i; $(MAKE) tidy); \
	done;

backup: checkin

#
# $Log: not supported by cvs2svn $
#

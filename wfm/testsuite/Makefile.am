#
# Makefile.am for core/wfm/testsuite
#

SUBDIRS = config lib wfm.test tests

AM_CXXFLAGS = -DY2LOG=\"wfm\"

PACKAGE = wfm
AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(top_srcdir)/libycp/src/include	\
	-I$(top_srcdir)/liby2/src/include	\
	-I$(top_srcdir)/libscr/src/include	\
	-I$(srcdir)/../src			\
	${Y2UTIL_CFLAGS}

noinst_PROGRAMS = runwfm runc

runwfm_SOURCES = runwfm.cc

runwfm_LDADD = \
	../src/libpy2wfm.la			\
        $(top_builddir)/scr/src/libpy2scr.la    \
 	$(top_builddir)/libscr/src/libscr.la	\
	$(top_builddir)/libycp/src/libycp.la	\
        $(top_builddir)/libycp/src/libycpvalues.la    \
 	$(top_builddir)/liby2/src/liby2.la	\
 	$(top_builddir)/debugger/liby2debug.la	\
	${Y2UTIL_LIBS}

runc_SOURCES = runc.cc
runc_LDFLAGS = -L$(libdir)
runc_LDADD = ../src/libpy2wfm.la		\
        $(top_builddir)/scr/src/libpy2scr.la    \
 	$(top_builddir)/libscr/src/libscr.la	\
	$(top_builddir)/libycp/src/libycp.la	\
        $(top_builddir)/libycp/src/libycpvalues.la    \
 	$(top_builddir)/debugger/liby2debug.la	\
	$(top_builddir)/liby2/src/liby2.la	\
	${Y2UTIL_LIBS}

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp	\
	libycp.log libycp.sum site.bak

EXTRA_DIST = README

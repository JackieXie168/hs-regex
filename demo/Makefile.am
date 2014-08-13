noinst_PROGRAMS = pmatch ci_strcmp re ex_regex regex_example testregex

ci_strcmp_SOURCES = ci_strcmp.c
pmatch_SOURCES = pmatch.c
re_SOURCES = debug.c split.c main.c
ex_regex_SOURCES = ex_regex.c
regex_example_SOURCES = regex_example.c
testregex_SOURCES = testregex.c

REGEX_SOURCES = \
							$(top_srcdir)/src/regcomp.c \
							$(top_srcdir)/src/regexec.c \
							$(top_srcdir)/src/regerror.c \
							$(top_srcdir)/src/regfree.c

REGEX_Headers = \
							$(top_srcdir)/include/cclass.h \
							$(top_srcdir)/include/cname.h \
							$(top_srcdir)/include/regex2.h \
							$(top_srcdir)/include/utils.h

REGEX_Docs = \
							$(top_srcdir)/doc/regex.3 \
							$(top_srcdir)/doc/regex.7

ALLSRC = $(re_SOURCES) $(REGEX_SOURCES)

REGEXHSRC = $(top_srcdir)/include/regex2.h $(REGEX_SOURCES)

ci_strcmp_LDADD = $(top_srcdir)/src/libregex.la
pmatch_LDADD = $(top_srcdir)/src/libregex.la
re_LDADD = $(top_srcdir)/src/libregex.la
ex_regex_LDADD = $(top_srcdir)/src/libregex.la
regex_example_LDADD = $(top_srcdir)/src/libregex.la
testregex_LDADD = $(top_srcdir)/src/libregex.la

ci_strcmp_CFLAGS = -DMAIN

#AM_CFLAGS = -O3 -Wall -fomit-frame-pointer -funroll-loops -DPOSIX_MISTAKE -Dstatic= -Dconst= -DREDEBUG 

INCLUDES = -I$(top_srcdir)/include -I.

# If you have a pre-ANSI compiler, put -o into MKHFLAGS.  If you want
# the Berkeley __P macro, put -b in.
MKHFLAGS=

default:	r

# Stuff that matters only if you're trying to lint the package.
LINTFLAGS=-I. -Dstatic= -Dconst= -DREDEBUG
LINTC= $(REGEX_SOURCES) debug.c main.c
JUNKLINT=possible pointer alignment|null effect

# arrangements to build forward-reference header files
.c.ih:
	sh mkh $(MKHFLAGS) -p $< > $@

regex.h: $(REGEXHSRC) mkh
	sh mkh $(MKHFLAGS) -i _REGEX_H_ $(REGEXHSRC) > regex.h

# regression test
r:	re tests
	./re <tests
	./re -el <tests
	./re -er <tests

# 57 variants, and other stuff, for development use -- not useful to you
ra:	./re tests
	-./re <tests
	-./re -el <tests
	-./re -er <tests

rx:	./re tests
	./re -x <tests
	./re -x -el <tests
	./re -x -er <tests

t:	./re tests
	-time ./re <tests
	-time ./re -cs <tests
	-time ./re -el <tests
	-time ./re -cs -el <tests

l:	$(LINTC)
	lint $(LINTFLAGS) -h $(LINTC) 2>&1 | egrep -v '$(JUNKLINT)' | tee lint

fullprint:
	ti README NEWS notes todo | list
	ti *.h | list
	list *.c
	list regex.3 regex.7

print:
	ti README NEWS notes todo | list
	ti *.h | list
	list reg*.c engine.c


mf.tmp:	Makefile
	sed '/^REGEXH=/s/=.*/=regex.h/' Makefile | sed '/#DEL$$/d' >$@

DTRH=$(REGEX_Headers)
PRE=COPYING README NEWS
POST=mkh $(REGEX_Docs) tests $(DTRH) $(ALLSRC) fake/*.[ch]
FILES=$(PRE) Makefile $(POST)
DTR=$(PRE) Makefile=mf.tmp $(POST)
dtr:	$(FILES) mf.tmp
	makedtr $(DTR) >$@
	rm mf.tmp

cio:	$(FILES)
	cio $(FILES)

rdf:	$(FILES)
	rcsdiff -c $(FILES) 2>&1 | p

# various forms of cleanup
tidy:
	rm -f junk* core core.* *.core dtr *.tmp lint

CLEANFILES = *~ *.s $(tidy) re regex.h

TESTS = test1.sh test2.sh test3.sh
test1.sh: re
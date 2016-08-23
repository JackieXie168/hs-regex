**[UNMAINTAINED] This library does not have a maintainer. The source code and repository will be kept at this URL indefinitely. If you'd like to help maintain this codebase, create an issue on this repo explaining why you'd like to become a maintainer and tag @tessel/maintainers in the body.**

# hsregex

This is a standalone library, originally a port of Henry Spencer's Regex Library from Tcl 8.6a2.

In the hope that the Tcl developers take this port as the base for their RE
module, great effort was paid to not update the *.c files, sadly it was not
possible. Some *.h files suffered dirty updates for the same reasons.

To build and test
	make
	./regtest_hsrex.sh

To rebuild
	make clean
	make

To build against the other library uncomment the proper line in the file
regtest_hsrex.sh and execute again.
	# Either this one
	$CC -I. -I$H/inc -L. -lhsrex -o $rgbin $rgsrc
	# or this one
	#$CC -I. -I$H/inc -L. -lhswrex -DREGEX_WCHAR -o $rgbin $rgsrc

You would like to test with debuging information. Uncomment the proper line in
the Makefile and rebuild.
	# Either this one
	#CFLAGS = -DREGEX_STANDALONE -fPIC -DREG_DEBUG -g
	# Or this one
	CFLAGS = -DREGEX_STANDALONE -fPIC -D_NDEBUG -O3

Two libraries are provided, libhsrex.so and libhswrex.so. The first one is for
ascii character code and the second one for wide characters. Both libraries
were tested in Linux and Solaris. Compiling and runing in Window$ should be
easy.

The following entry point where defined in each library:
re_comp()	(re_wcomp() for wide char) to compile a RE
re_exec()	(re_wexec() for wide char) to parse data against a compiled RE.
regfree()	To dispose the memory of a compiled RE.
regerror()	Translates error codes to ascii strings.

It is pretty easy to add support for a regcomp() regexec() front end. That
front end functions should take care of UTF-8 to wide charater conversion, for
instance.

The regression test script regtest_hsrex.sh contains an example of how to use
the libraries. It just test cases I was interested on. Adding more use cases to
that script should be easy.

Send any comments to Walter Waldo <wawasa@gmail.com>

Enjoy it,

Walter Waldo.

To run the sample on Linux you may have to set LD_LIBRARY_PATH, e.g.:

        LD_LIBRARY_PATH=`pwd` rexx apitest1.rex

or
	export LD_LIBRARY_PATH=`pwd`
        rexx apitest1.rex

---

To compile on Linux use Makefile.linux:

 	make -f Makefile.linux clean
	make -f Makefile.linux

To compile on MacOS use Makefile.apple:

 	make -f Makefile.apple clean
	make -f Makefile.apple


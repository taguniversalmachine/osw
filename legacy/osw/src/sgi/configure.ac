# Process this file with autoconf to produce a configure script.
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([oswSGIHiResTimer.cpp])

AC_SUBST(DEFINES)
AC_SUBST(TCL_INCLUDE_DIR)


# Checks for programs.
AC_PROG_CXX(CC g++)
AC_PROG_CC(cc gcc)
AC_PROG_INSTALL

# Checks for libraries.

AC_CHECK_LIB([pthread], [main], , "Requires pthreads"; exit)
AC_CHECK_LIB([C], [main])
AC_CHECK_LIB([Cio], [main])
AC_CHECK_LIB([audio], [main])
AC_CHECK_LIB([m], [main])

AC_CHECK_LIB(tcl8.3, main,,
      AC_CHECK_LIB(tcl8.2, main,,
            echo "No Tcl library found"; exit),-L/usr/local/lib )

AC_CHECK_LIB(tk8.3, main,,
       AC_CHECK_LIB(tk8.2, main,,
              echo "No Tk library found"; exit),-L/usr/local/lib )


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([alloca.h malloc.h netinet/in.h stdlib.h string.h])
AC_CHECK_HEADERS([errno.h fcntl.h sys/ioctl.h sys/time.h unistd.h])
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h])

AC_CHECK_HEADER(tcl.h,TCL_INCLUDE_DIR="",
    AC_CHECK_HEADER(/usr/local/include/tcl.h,TCL_INCLUDE_DIR="-I /usr/local/include", 
    AC_CHECK_HEADER(tcl8.1/tcl.h,TCL_INCLUDE_DIR="-I /usr/include/tcl8.1", 
    AC_CHECK_HEADER(tcl8.2/tcl.h,TCL_INCLUDE_DIR="-I /usr/include/tcl8.2",
    AC_CHECK_HEADER(tcl8.3/tcl.h,TCL_INCLUDE_DIR="-I /usr/include/tcl8.3",
    AC_CHECK_HEADER(tcl8.4/tcl.h,TCL_INCLUDE_DIR="-I /usr/include/tcl8.4", 
    AC_CHECK_HEADER(tcl8.5/tcl.h,TCL_INCLUDE_DIR="-I /usr/include/tcl8.5", 
    echo "no tcl/tk header found";exit)))))))


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor memset memcpy pow strerror sin cos tan sqrt log log10 acos asin atan ceil sinh cosh tanh])
AC_FUNC_VPRINTF


# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_HEADER_STDC
AC_CHECK_FUNCS([gethostbyaddr gethostbyname gettimeofday memset select socket strerror])

AC_CHECK_PROGS(WISH,[wish8.3 wish8.4 wish8.2])
AC_PATH_PROG(WISH_DIR,$WISH)
AC_SUBST(TCL_BIN,$WISH_DIR)


CFLAGS="$CFLAGS  -I../include -I../generic -I. -I../types -I../sdif -I../osc -O2 -n32 -OPT:Olimit=0 -D_OSWCORE -D _USING_TCL"

CXXFLAGS="-LANG:std -prelink -ptused"

AC_SUBST(ac_srcdir_abs,`pwd`)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT



echo ""
echo "------------------------------------------"
echo "Open Sound World; Configuration completed."

echo "------------------------------------------"
echo ""







# Process this file with autoconf to produce a configure script.
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([oswMacOSXHiResTimer.cpp])

AC_SUBST(STLPORT_DIR)
AC_SUBST(DEFINES)
AC_SUBST(TCL_INCLUDE_DIR)
AC_SUBST(TK_INCLUDE_DIR)

AC_ARG_VAR(STLPORT_DIR,"Set STLport header directory")

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX


# Checks for libraries.
AC_CHECK_LIB([pthread], [main], , "Requires pthreads"; exit)
#AC_CHECK_LIB([stlport_gcc], [main], , echo "Please install stlport!"; exit)


AC_CHECK_FILE([/Library/Frameworks/Tcl.framework/Tcl],LIBS="$LIBS -framework Tcl",
 AC_CHECK_LIB(tcl8.4, main,,
   AC_CHECK_LIB(tcl8.3, main,,
      AC_CHECK_LIB(tcl8.2, main,,
            echo "No Tcl library found"; exit))))

AC_CHECK_FILE([/Library/Frameworks/Tk.framework/Tk],LIBS="$LIBS -framework Tk",
  AC_CHECK_LIB(tk8.4, main,,
    AC_CHECK_LIB(tk8.3, main,,
       AC_CHECK_LIB(tk8.2, main,,
              echo "No Tk library found"; exit))))

AC_CHECK_FILE([/System/Library/Frameworks/CoreAudio.framework/CoreAudio],
	LIBS="$LIBS -framework CoreAudio",
	echo "CoreAudio not found"; exit)

AC_CHECK_FILE([/System/Library/Frameworks/CoreMIDI.framework/CoreMIDI],
	LIBS="$LIBS -framework CoreMIDI",
	echo "CoreMIDI not found"; exit)

AC_CHECK_FILE([/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation],
	LIBS="$LIBS -framework CoreFoundation",
	echo "CoreMIDI not found"; exit)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([alloca.h malloc.h netinet/in.h stdlib.h string.h])
AC_CHECK_HEADERS([errno.h fcntl.h sys/ioctl.h sys/time.h unistd.h])
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h])


AC_CHECK_FILE(/Library/Frameworks/Tcl.framework/Headers/tcl.h,
		    TCL_INCLUDE_DIR="/Library/Frameworks/Tcl.framework/Headers",
    AC_CHECK_HEADER(tcl.h,TCL_INCLUDE_DIR="",
    AC_CHECK_HEADER(tcl8.1/tcl.h,TCL_INCLUDE_DIR="/usr/include/tcl8.1", 
    AC_CHECK_HEADER(tcl8.2/tcl.h,TCL_INCLUDE_DIR="/usr/include/tcl8.2",
    AC_CHECK_HEADER(tcl8.3/tcl.h,TCL_INCLUDE_DIR="/usr/include/tcl8.3",
    AC_CHECK_HEADER(tcl8.4/tcl.h,TCL_INCLUDE_DIR="/usr/include/tcl8.4", 
    AC_CHECK_HEADER(tcl8.5/tcl.h,TCL_INCLUDE_DIR="/usr/include/tcl8.5", 
    echo "no tcl header found";exit)))))))


AC_CHECK_FILE(/Library/Frameworks/Tk.framework/Headers/tk.h,
		    TK_INCLUDE_DIR="/Library/Frameworks/Tk.framework/Headers",
    AC_CHECK_HEADER(tk.h,TK_INCLUDE_DIR="",
    AC_CHECK_HEADER(tk8.1/tk.h,TK_INCLUDE_DIR="/usr/include/tk8.1", 
    AC_CHECK_HEADER(tk8.2/tk.h,TK_INCLUDE_DIR="/usr/include/tk8.2",
    AC_CHECK_HEADER(tk8.3/tk.h,TK_INCLUDE_DIR="/usr/include/tk8.3",
    AC_CHECK_HEADER(tk8.4/tk.h,TK_INCLUDE_DIR="/usr/include/tk8.4", 
    AC_CHECK_HEADER(tk8.5/tk.h,TK_INCLUDE_DIR="/usr/include/tk8.5", 
    echo "no tk header found";exit)))))))


AC_LANG(C++)
AC_REQUIRE_CPP
AC_MSG_CHECKING(for ext/hash_map)
AC_TRY_COMPILE([#include <ext/hash_map>
namespace std {
  using namespace __gnu_cxx;
}
using namespace std;
],[ hash_map<int,int> hm; return 0;],
AC_MSG_RESULT(yes)
CFLAGS="$CFLAGS -DOSW_EXT_HASH_MAP",
  AC_MSG_RESULT(no)
  AC_CHECK_HEADER(hash_map,,echo "no hash_map header found"; exit))


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


CFLAGS="$CFLAGS -I../include -I../generic -I. -I../types -I../sdif  -I../osc $TCL_INCLUDE_DIR $TK_INCLUDE_DIR  -O2 -D_OSWCORE -D_USING_TCL -march=pentium -march=i586"

CXXFLAGS="-ftemplate-depth-64 -felide-constructors -Wno-sign-compare -Wno-reorder"

AC_SUBST(ac_srcdir_abs,`pwd`)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo ""
echo "------------------------------------------"
echo "Open Sound World; Configuration completed."

echo "------------------------------------------"
echo ""








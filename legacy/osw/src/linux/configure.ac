# Process this file with autoconf to produce a configure script.
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([oswLinuxHiResTimer.cpp])

AC_SUBST(alsa,"")
AC_SUBST(jack,"")
AC_SUBST(stackcheck,"yes")
AC_SUBST(STLPORT_DIR)
AC_SUBST(DEFINES)
AC_SUBST(TCL_INCLUDE_DIR)
AC_SUBST(TK_INCLUDE_DIR)
AC_SUBST(FINAL_DIR)

AC_ARG_ENABLE(alsa,[  --enable-alsa          compile ALSA support],alsa=$enableval)
AC_ARG_ENABLE(jack,[  --enable-jack          compile JACK support],jack=$enableval)

#arg needed to disable -fstack-check on systems like mine (AC)
AC_ARG_ENABLE(stackcheck,[  --disable-stackcheck    remove fstack-check option (need on some RedHat systems)],stackcheck=$enableval)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

# Require ALSA if --enable-alsa was used
if test "$alsa" = "yes" ; then
	AC_CHECK_LIB([asound], [main], ,[AC_MSG_ERROR(Cannot find ALSA library)])
fi

AC_SUBST(ALSALIB,"$LIBS")
LIBS=""

# Require JACK if --enable-jack was used
if test "$jack" = "yes" ; then
	AC_CHECK_LIB([jack], [main], ,[AC_MSG_ERROR(Cannot find JACK library)])
fi

AC_SUBST(JACKLIB,"$LIBS")
LIBS=""

AC_SUBST(DALSA, "")
AC_SUBST(DJACK, "")


AC_SUBST(FSTACK_CHECK, "-fstack-check")
if test "$stackcheck" != "yes"; then
  AC_SUBST(FSTACK_CHECK,"")
fi

if test "$alsa" = "yes" ; then
  AC_SUBST(DALSA, "-DALSA")
fi

if test "$jack" = "yes" ; then
  AC_SUBST(DJACK, "-DJACK")
fi


AC_CHECK_LIB(tcl8.4, main,,
	[AC_CHECK_LIB(tcl8.3, main,,
      		[AC_CHECK_LIB(tcl8.2, main,,
            		[AC_MSG_ERROR(Cannot find Tcl library)])])])

AC_CHECK_LIB(tk8.4, main,,
	[AC_CHECK_LIB(tk8.3, main,,
       		[AC_CHECK_LIB(tk8.2, main,,
              		[AC_MSG_ERROR(Cannot find Tk library)])])])

AC_CHECK_LIB([pthread], [main], , [AC_MSG_ERROR(Cannot find pthread library)])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([alloca.h malloc.h netinet/in.h stdlib.h string.h])
AC_CHECK_HEADERS([errno.h fcntl.h sys/ioctl.h sys/time.h unistd.h])
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h])
AC_CHECK_HEADERS([sys/types.h limits.h  sys/stat.h])

AC_CHECK_HEADER(tcl.h,[TCL_INCLUDE_DIR=""],
       [AC_CHECK_HEADER(tcl8.5/tcl.h,[TCL_INCLUDE_DIR="-I/usr/include/tcl8.5"],
       [AC_CHECK_HEADER(tcl8.4/tcl.h,[TCL_INCLUDE_DIR="-I/usr/include/tcl8.4"],
       [AC_CHECK_HEADER(tcl8.3/tcl.h,[TCL_INCLUDE_DIR="-I/usr/include/tcl8.3"],
       [AC_CHECK_HEADER(tcl8.2/tcl.h,[TCL_INCLUDE_DIR="-I/usr/include/tcl8.2"],
       [AC_CHECK_HEADER(tcl8.1/tcl.h,[TCL_INCLUDE_DIR="-I/usr/include/tcl8.1"],
       [AC_MSG_ERROR([Could not locate tcl.h])]
       )])])])])])

# Need to make sure that tcl.h is resovlable to check tk.h
export CPPFLAGS="$TCL_INCLUDE_DIR";

AC_CHECK_HEADER(
       tk.h,[TK_INCLUDE_DIR=""],
       [AC_CHECK_HEADER(tk8.5/tk.h,[TK_INCLUDE_DIR="-I/usr/include/tk8.5"],
       [AC_CHECK_HEADER(tk8.4/tk.h,[TK_INCLUDE_DIR="-I/usr/include/tk8.4"],
       [AC_CHECK_HEADER(tk8.3/tk.h,[TK_INCLUDE_DIR="-I/usr/include/tk8.3"],
       [AC_CHECK_HEADER(tk8.2/tk.h,[TK_INCLUDE_DIR="-I/usr/include/tk8.2"],
       [AC_CHECK_HEADER(tk8.1/tk.h,[TK_INCLUDE_DIR="-I/usr/include/tk8.1"],
       [AC_MSG_ERROR([Could not locate tk.h])],
       )])])])])])

# Need X11/Xlib.h
AC_CHECK_HEADER(X11/Xlib.h, , [AC_MSG_ERROR(Could not find X11/Xlib.h)])

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


AC_LANG(C++)
AC_REQUIRE_CPP
AC_MSG_CHECKING(for ext/hash_map)
AC_TRY_COMPILE([#include <ext/hash_map>
namespace std {
  using namespace __gnu_cxx;
}
using namespace std;
],[ hash_map<int,int> hm; return 0;],
[AC_MSG_RESULT(yes)
CFLAGS="$CFLAGS -DOSW_EXT_HASH_MAP"],
  [AC_MSG_RESULT(no)
  AC_CHECK_HEADER(hash_map,,echo "no hash_map header found"; exit)])


# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_HEADER_STDC
AC_CHECK_FUNCS([gethostbyaddr gethostbyname gettimeofday memset select socket strerror])

AC_CHECK_PROGS(WISH,[wish8.4 wish8.3 wish8.2])
AC_PATH_PROG(WISH_DIR,$WISH)
AC_SUBST(TCL_BIN,$WISH_DIR)

CFLAGS="$CFLAGS -I../include -I../generic -I. -I../types -I../sdif  -I../osc $TCL_INCLUDE_DIR $TK_INCLUDE_DIR  -O2 -D_OSWCORE -D_USING_TCL -march=pentium -march=i586"

CXXFLAGS="-ftemplate-depth-64 -felide-constructors -Wno-sign-compare -Wno-reorder"

AC_SUBST([ac_srcdir_abs],`pwd`)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo ""
echo "------------------------------------------"
echo "Open Sound World; Configuration completed."
 
if test "$alsa" = "yes"; then
  echo "  ALSA Enabled: Yes"
else
  echo "  ALSA Enabled: No"
fi

if test "$jack" = "yes"; then
  echo "  JACK Enabled: Yes"
else
  echo "  JACK Enabled: No"
fi

echo "------------------------------------------"
echo ""


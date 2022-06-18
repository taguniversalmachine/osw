
#ifndef _OSWCFG_H
#define _OSWCFG_H

#if defined(_WINDOWS) || defined (__WIN32__)
#  ifdef _OSWOSX
#    define _oswexport __declspec(dllimport)
#  else
#    define _oswexport __declspec(dllexport) 
#  endif
#else
#  define _oswexport
#endif

#endif // _OSW_EXPORT_H





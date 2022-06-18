/*
 * July 5, 1991
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained. 
 * Lance Norskog And Sundry Contributors are not responsible for 
 * the consequences of using this software.
 */

/*
 May, 1999
 Adapted for use in Open Sound Library by Amar Chaudhary
 */

#include "st.h"

/* this is up here for SCO ??? */
#include <string.h>
#if	defined(unix) || defined(AMIGA) || defined(__OS2__)
#include <sys/types.h>
#include <sys/stat.h>
#endif
#if	defined(__STDC__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <ctype.h>
#ifdef VMS
#include <errno.h>
#include <perror.h>
#define LASTCHAR        ']'
#else
#include <errno.h>
#define LASTCHAR        '/'
#endif
/* YOU MAY WANT THIS
#include <getopt.h>
*/

/*
 * util.c.
 * Incorporate Jimen Ching's fixes for real library operation: Aug 3, 1994.
 * Redo all work from scratch, unfortunately.
 * Separate out all common variables used by effects & handlers,
 * and utility routines for other main programs to use.
 */


EXPORT float volume = 1.0;	/* expansion coefficient */
EXPORT int dovolume = 0;

EXPORT float amplitude = 1.0;	/* Largest sample so far */

EXPORT int writing = 0;	/* are we writing to a file? */

/* export flags */
EXPORT int verbose = 0;	/* be noisy on stderr */
EXPORT int summary = 0;	/* just print summary of information */

#ifdef	DOS
char writebuf[BUFSIZ];	/* output write buffer */
#endif

#ifndef __APPLE__
EXPORT char *myname;
#endif

EXPORT int soxpreview = 0;	/* preview mode */


/* We can probably assume Standard C by this day an age - AC */
/* OK, maybe not... */

void
#if	defined(__STDC__)
report(char *fmt, ...)
#else
report(va_alist) 
va_dcl
#endif
{
	va_list args;
#if	!defined(__STDC__)
	char *fmt;
#endif

#if !defined(DOS) && !defined(__APPLE__)
	/* single-threaded machines don't really need this */
	fprintf(stderr, "%s: ", myname);
#endif
#if	!defined(__STDC__)
	va_start(args);
	fmt = va_arg(args, char *);
#else
	va_start(args, fmt);
#endif
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}


#ifndef __APPLE__
void
#if	defined(__STDC__)
warn(char *fmt, ...)
#else
warn(va_alist) 
va_dcl
#endif
{
	va_list args;
#if	!defined(__STDC__)
	char *fmt;
#endif

#if !defined(DOS) && !defined(__APPLE__)
	/* single-threaded machines don't really need this */
	fprintf(stderr, "%s: ", myname);
#endif
#if	!defined(__STDC__)
	va_start(args);
	fmt = va_arg(args, char *);
#else
	va_start(args, fmt);
#endif
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}
#endif

void
#if	defined(__STDC__)
fail(char *fmt, ...)
#else
fail(va_alist) 
va_dcl
#endif
{
	va_list args;
#if	!defined(__STDC__)
	char *fmt;
#endif
	extern void cleanup();

#if !defined(DOS) && !defined(__APPLE__)
	/* single-threaded machines don't really need this */
	fprintf(stderr, "%s: ", myname);
#endif
#if	!defined(__STDC__)
	va_start(args);
	fmt = va_arg(args, char *);
#else
	va_start(args, fmt);
#endif
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
	cleanup();
#if 0
	exit(2);
#endif
}

int strcmpcase(s1, s2)
char *s1, *s2;
{
	while(*s1 && *s2 && (tolower(*s1) == tolower(*s2)))
		s1++, s2++;
	return *s1 - *s2;
}

/*
 * Check that we have a known format suffix string.
 */
void
gettype(formp)
ft_t formp;
{
	char **list;
	int i;

	if (! formp->filetype)
fail("Must give file type for %s file, either as suffix or with -t option",
formp->filename);
	for(i = 0; formats[i].names; i++) {
		for(list = formats[i].names; *list; list++) {
			char *s1 = *list, *s2 = formp->filetype;
			if (! strcmpcase(s1, s2))
				break;	/* not a match */
		}
		if (! *list)
			continue;
		/* Found it! */
		formp->h = &formats[i];
		return;
	}
	if (! strcmpcase(formp->filetype, "snd")) {
		verbose = 1;
		report("File type '%s' is used to name several different formats.", formp->filetype);
		report("If the file came from a Macintosh, it is probably");
		report("a .ub file with a sample rate of 11025 (or possibly 5012 or 22050).");
		report("Use the sequence '-t .ub -r 11025 file.snd'");
		report("If it came from a PC, it's probably a Soundtool file.");
		report("Use the sequence '-t .sndt file.snd'");
		report("If it came from a NeXT, it's probably a .au file.");
		fail("Use the sequence '-t .au file.snd'\n");
	}
	fail("File type '%s' of %s file is not known!",
		formp->filetype, formp->filename);
}


void copyformat(ft, ft2)
ft_t ft, ft2;
{
	int noise = 0, i;
	double factor;

	if (ft2->info.rate == 0) {
		ft2->info.rate = ft->info.rate;
		noise = 1;
	}
	if (ft2->info.size == -1) {
		ft2->info.size = ft->info.size;
		noise = 1;
	}
	if (ft2->info.style == -1) {
		ft2->info.style = ft->info.style;
		noise = 1;
	}
	if (ft2->info.channels == -1) {
		ft2->info.channels = ft->info.channels;
		noise = 1;
	}
	if (ft2->comment == NULL) {
		ft2->comment = ft->comment;
		noise = 1;
	}
	/* 
	 * copy loop info, resizing appropriately 
	 * it's in samples, so # channels don't matter
	 */
	factor = (double) ft2->info.rate / (double) ft->info.rate;
	for(i = 0; i < NLOOPS; i++) {
		ft2->loops[i].start = ft->loops[i].start * factor;
		ft2->loops[i].length = ft->loops[i].length * factor;
		ft2->loops[i].count = ft->loops[i].count;
		ft2->loops[i].type = ft->loops[i].type;
	}
	/* leave SMPTE # alone since it's absolute */
	ft2->instr = ft->instr;
}

void cmpformats(ft, ft2)
ft_t ft, ft2;
{
}

/* check that all settings have been given */
void checkformat(ft) 
ft_t ft;
{
	if (ft->info.rate == 0)
		fail("Sampling rate for %s file was not given\n", ft->filename);
	if ((ft->info.rate < 100) || (ft->info.rate > 50000L))
		fail("Sampling rate %lu for %s file is bogus\n", 
			ft->info.rate, ft->filename);
	if (ft->info.size == -1)
		fail("Data size was not given for %s file\nUse one of -b/-w/-l/-f/-d/-D", ft->filename);
	if (ft->info.style == -1 && ft->info.size != FLOAT)
		fail("Data style was not given for %s file\nUse one of -s/-u/-U/-A", ft->filename);
	/* it's so common, might as well default */
	if (ft->info.channels == -1)
		ft->info.channels = 1;
	/*	fail("Number of output channels was not given for %s file",
			ft->filename); */
}





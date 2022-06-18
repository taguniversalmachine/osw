/*
 * Originally created: July 5, 1991
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained. 
 * Lance Norskog And Sundry Contributors are not responsible for 
 * the consequences of using this software.
 *
 * 1999 Adapted for Open Sound Library by Amar Chaudhary
 */

#include "st.h"

/*
 * Sound Tools file format and effect tables.
 */

/* File format handlers. */

/* File formats downsized by Amar Chaudhary.  If I don't know what it is,
   then I'm not supporting it */


char *aiffnames[] = {
	"aiff",
	"aif",
	(char *) 0
};
extern void aiffstartread();
extern LONG aiffread();
extern void aiffstopread();
extern void aiffstartwrite();
extern void aiffwrite();
extern void aiffstopwrite();

char *alnames[] = {
	"al",
	(char *) 0
};
extern void alstartread();
extern void alstartwrite();

#if 1
char *aunames[] = {
	"au",
#ifdef	NeXT
	"snd",
#endif
	(char *) 0
};
extern void austartread();
extern LONG auread();
extern void austartwrite();
extern void auwrite();
extern void austopwrite();
#endif

char *autonames[] = {
	"auto",
	(char *) 0,
};

extern void autostartread();
extern void autostartwrite();

char *cdrnames[] = {
	"cdr",
	(char *) 0
};
extern void cdrstartread();
extern LONG cdrread();
extern void cdrstopread();
extern void cdrstartwrite();
extern void cdrwrite();
extern void cdrstopwrite();

char *datnames[] = {
	"dat",
	(char *) 0
};
extern void datstartread();
extern LONG datread();
extern void datstartwrite();
extern void datwrite();

char *rawnames[] = {
	"raw",
	(char *) 0
};
extern void rawstartread();
extern LONG rawread();
extern void rawstartwrite();
extern void rawwrite();
extern void rawstopwrite();

char *sbnames[] = {
	"sb",
	(char *) 0
};
extern void sbstartread();
extern void sbstartwrite();

char *sfnames[] = {
	"sf",
	(char *) 0
};

extern void sfstartread();
extern void sfstartwrite();

char *swnames[] = {
	"sw",
	(char *) 0
};
extern void swstartread();
extern void swstartwrite();

char *ubnames[] = {
	"ub",
	"sou",
	"fssd",
#ifdef	MAC
	"snd",
#endif
	(char *) 0
};
extern void ubstartread();
extern void ubstartwrite();

char *ulnames[] = {
	"ul",
	(char *) 0
};
extern void ulstartread();
extern void ulstartwrite();

char *uwnames[] = {
	"uw",
	(char *) 0
};
extern void uwstartread();
extern void uwstartwrite();

char *wavnames[] = {
	"wav",
	(char *) 0
};
extern void wavstartread();
extern LONG wavread();
extern void wavstartwrite();
extern void wavwrite();
extern void wavstopwrite();


extern void nothing();
extern LONG nothing_success();

EXPORT format_t formats[] = {
	{aiffnames, FILE_STEREO,
		aiffstartread, aiffread, aiffstopread,	   /* SGI/Apple AIFF */
		aiffstartwrite, aiffwrite, aiffstopwrite},
	{alnames, FILE_STEREO,
		alstartread, rawread, nothing, 	           /* a-law byte raw */
		alstartwrite, rawwrite, nothing},	

	{aunames, FILE_STEREO,
		austartread, auread, nothing,	       /* SPARC .AU w/header */
		austartwrite, auwrite, austopwrite},

	{autonames, FILE_STEREO,
		autostartread, nothing_success, nothing,/* Guess from header */
		autostartwrite, nothing, nothing},	 /* patched run time */
#if 0
	/* Support this one... later ;) -AC */
	{cdrnames, FILE_STEREO,
		cdrstartread, cdrread, cdrstopread,	      /* CD-R format */
		cdrstartwrite, cdrwrite, cdrstopwrite},
#endif
	{datnames, 0,
		datstartread, datread, nothing, 	/* Text data samples */
		datstartwrite, datwrite, nothing},
	{rawnames, FILE_STEREO,
		rawstartread, rawread, nothing,	        /* Raw format */
	        rawstartwrite, rawwrite, nothing},
	{sbnames, FILE_STEREO,
		sbstartread, rawread, nothing, 	          /* signed byte raw */
		sbstartwrite, rawwrite, nothing},	
	{sfnames, FILE_STEREO,
		sfstartread, rawread, nothing, 	         /* IRCAM Sound File */
		sfstartwrite, rawwrite, nothing},	    /* Relies on raw */
	{swnames, FILE_STEREO,
		swstartread, rawread, nothing, 	          /* signed word raw */
		swstartwrite, rawwrite, nothing},
	{ubnames, FILE_STEREO,
		ubstartread, rawread, nothing, 	        /* unsigned byte raw */
		ubstartwrite, rawwrite, nothing},
	{ulnames, FILE_STEREO,
		ulstartread, rawread, nothing, 	           /* u-law byte raw */
		ulstartwrite, rawwrite, nothing},	
	{uwnames, FILE_STEREO,
		uwstartread, rawread, nothing, 	        /* unsigned word raw */
		uwstartwrite, rawwrite, nothing},	
	{wavnames, FILE_STEREO,
		wavstartread, wavread, nothing, 	   /* Microsoft .wav */
		wavstartwrite, wavwrite, wavstopwrite},	
	{0, 0,
	 0, 0, 0, 0, 0, 0}
};




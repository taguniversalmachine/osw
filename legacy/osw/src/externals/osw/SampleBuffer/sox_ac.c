/*
 * SampleBuffer file I/O adapted by Amar Chaudhary from...
 *
 * Sox - The Swiss Army Knife of Audio Manipulation.
 *
 * This is the main function for the command line sox program.
 *
 * July 5, 1991
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained. 
 * Lance Norskog And Sundry Contributors are not responsible for 
 * the consequences of using this software.
 *
 * Change History:
 *
 * June 1, 1998 - Chris Bagwell (cbagwell@sprynet.com)
 *   Added patch to get volume working again.  Based on patch sent from
 *   Matija Nalis <mnalis@public.srce.hr>.
 *   Added command line switches to force format to ADPCM or GSM.
 *
 * September 12, 1998 - Chris Bagwell (cbagwell@sprynet.com)
 *   Reworked code that handled effects.  Wasn't correctly draining
 *   stereo effects and a few other problems.
 *   Made command usage (-h) show supported effects and file formats.
 *   (this is partially from a patch by Leigh Smith
 *    leigh@psychokiller.dialix.oz.au).
 *
 */ 

#include "st.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if	defined(__STDC__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <ctype.h>

#define LASTCHAR        '/'


/*
 * SOX main program.
 *
 * Rewrite for new nicer option syntax.  July 13, 1991.
 * Rewrite for separate effects library.  Sep. 15, 1991.
 * Incorporate Jimen Ching's fixes for real library operation: Aug 3, 1994.
 * Rewrite for multiple effects: Aug 24, 1994.
 */


static LONG ibufl[BUFSIZ/2];	/* Left/right interleave buffers */
static LONG ibufr[BUFSIZ/2];	
static LONG obufl[BUFSIZ/2];
static LONG obufr[BUFSIZ/2];

void init();
void process();

struct soundstream informat, outformat;
ft_t ft;
char *ifile, *itype;

extern int verbose;

int main(argc, argv)
int argc;
char **argv;
{
  verbose = 1;
	init();
	
	ifile = NULL;

	/* Get input format options */
	ft = &informat;
	ifile = argv[1];
	if (! strcmp(ifile, "-"))
		ft->fp = stdin;
	else if ((ft->fp = fopen(ifile, READBINARY)) == NULL)
		fail("Can't open input file '%s': %s", 
			ifile, strerror(errno));
	ft->filename = ifile;
	

	informat.seekable  = 1;

	/* If file types have not been set with -t
	   (and the never will be -AC) set from file names. */
	if (! informat.filetype) {
		if ((informat.filetype = strrchr(ifile, LASTCHAR)) != NULL)
			informat.filetype++;
		else
			informat.filetype = ifile;
		if ((informat.filetype = strrchr(informat.filetype, '.')) != NULL)
			informat.filetype++;
		else /* Default to "auto" */
			informat.filetype = "auto";
	}

	/* Default the input comment to the filename. 
	 * The output comment will be assigned when the informat 
	 * structure is copied to the outformat. 
	 */
	informat.comment = informat.filename;

	process();
	fflush(stderr);
	return(0);
}

void init() {

	/* init files */
	informat.info.rate      = outformat.info.rate  = 0;
	informat.info.size      = outformat.info.size  = -1;
	informat.info.style     = outformat.info.style = -1;
	informat.info.channels  = outformat.info.channels = -1;
	informat.comment   = outformat.comment = NULL;
	informat.swap      = 0;
	informat.filetype  = outformat.filetype  = (char *) 0;
	informat.fp        = stdin;
	informat.filename  = "input";
}

/* 
 * Process input file -> effect table -> output file
 *	one buffer at a time
 */

void process() {
    LONG i;
    int e, f, havedata;

    gettype(&informat);
    /* Read and write starters can change their formats. */
    puts (informat.filetype);
    (* informat.h->startread)(&informat);
    checkformat(&informat);
    
    
    report("Input file: using sample rate %lu\n\tsize %s, style %s, %d %s",
	   informat.info.rate, sizes[informat.info.size], 
	   styles[informat.info.style], informat.info.channels, 
	   (informat.info.channels > 1) ? "channels" : "channel");
    if (informat.comment)
	report("Input file: comment \"%s\"\n", informat.comment);

#if 0
    /* Read initial chunk of input data. */
    efftab[0].olen = (*informat.h->read)(&informat, 
					 efftab[0].obuf, (LONG) BUFSIZ);
    efftab[0].odone = 0;

    /* Change the volume of this initial input data if needed. */
    if (dovolume)
	for (i = 0; i < efftab[0].olen; i++)
	    efftab[0].obuf[i] = volumechange(efftab[0].obuf[i]);

    /* Run input data through effects and get more until olen == 0 */
    while (efftab[0].olen > 0) {

	/* mark chain as empty */
	for(e = 1; e < neffects; e++)
	    efftab[e].odone = efftab[e].olen = 0;

	do {

	    /* run entire chain BACKWARDS: pull, don't push.*/
	    /* this is because buffering system isn't a nice queueing system */
	    for(e = neffects - 1; e > 0; e--) 
		if (flow_effect(e))
		    break;

	    /* If outputing and output data was generated then write it */
	    if (writing&&(efftab[neffects-1].olen>efftab[neffects-1].odone)) {
		(* outformat.h->write)(&outformat, efftab[neffects-1].obuf, 
				       (LONG) efftab[neffects-1].olen);
	        efftab[neffects-1].odone = efftab[neffects-1].olen;
	    }

	    /* if stuff still in pipeline, set up to flow effects again */
	    havedata = 0;
	    for(e = 0; e < neffects - 1; e++)
		if (efftab[e].odone < efftab[e].olen) {
		    havedata = 1;
		    break;
		}
	} while (havedata);

	/* Read another chunk of input data. */
	efftab[0].olen = (*informat.h->read)(&informat, 
		efftab[0].obuf, (LONG) BUFSIZ);
	efftab[0].odone = 0;

	/* Change volume of these samples if needed. */
	if (dovolume)
	    for (i = 0; i < efftab[0].olen; i++)
		efftab[0].obuf[i] = volumechange(efftab[0].obuf[i]);
    }

    /* Drain the effects out first to last, 
     * pushing residue through subsequent effects */
    /* oh, what a tangled web we weave */
    for(f = 1; f < neffects; f++)
    {
	while (1) {

	    if (drain_effect(f) == 0)
		break;		/* out of while (1) */
	
	    if (writing&&efftab[neffects-1].olen > 0)
		(* outformat.h->write)(&outformat, efftab[neffects-1].obuf,
				       (LONG) efftab[neffects-1].olen);

	    if (efftab[f].olen != BUFSIZ)
		break;
	}
    }
	

    /* Very Important: 
     * Effect stop is called BEFORE files close.
     * Effect may write out more data after. 
     */
    
    for (e = 1; e < neffects; e++) {
	(* efftab[e].h->stop)(&efftab[e]);
	if (efftabR[e].name)
	    (* efftabR[e].h->stop)(&efftabR[e]);
    }
#endif

    (* informat.h->stopread)(&informat);
    fclose(informat.fp);

}

/* called from util.c:fail */
void cleanup() {
  /* Close the input file and outputfile before exiting*/
  if (informat.fp)
    fclose(informat.fp);
  if (outformat.fp) {
    fclose(outformat.fp);
    /* remove the output file because we failed, if it's ours. */
    /* unless its a unix /dev/sound type file */
#ifdef unix
    if (filetype(fileno(outformat.fp)) == S_IFREG)
#endif
      REMOVE(outformat.filename);
  }
  fflush(stderr);
}




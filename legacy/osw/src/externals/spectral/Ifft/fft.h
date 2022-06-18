/*
 * Copyright (c) 1992 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*	fft.h
	 real and complex forward and inverse fft's */

#ifdef __cplusplus
typedef bool Boolean;
extern "C" {
#else
 #ifdef FALSE
  typedef int Boolean;
 #else
  typedef enum {FALSE,TRUE} Boolean;
 #endif
#endif

void fftReal(int n, float *rR, float *r,int rstride);
void ifftReal(int n, float *res, float *r,float *c, int istride);
void fftComplex(int n, float *a,  Boolean notinverse);
void fftRealfast(int n, float *r);
void ifftRealfast(int n, float *res, float *r,float *c, int istride);

#ifdef __cplusplus
}
#endif


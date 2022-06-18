
/*
 * Copyright (c) 1992 Regents of the University of California.
 * All rights reserved.
 * The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "vectorop.h"
#include <math.h>
#include <limits.h>


#define PI 3.14159265358979323
void SineFunction(int n, float *stab, int stride, float from, float to)
{
	int j;
	float f = (to-from)/n;

	for(j=0;j<n;++j)
		stab[j*stride] = sin(from +j*f);
}
void ExpFunction(int n, float *etab, int stride, float from, float to)
{
	int j;
	float f = (to-from)/n;

	for(j=0;j<n;++j)
		etab[j*stride] = exp(from+j*f);
}
float productvec( int n,
	 float *a, int astride, float *b, int bstride)
{
	int i;
	float sum = 0.0f;
	for(i=0;i<n;++i)
	{
		sum += a[i*astride] * b[i*bstride];
		
	}
	return sum;
}
void addvec2(int n, float *out, int stride,
	 float *a, int astride, float *b, int bstride)
{
	int i;
	for(i=0;i<n;++i)
	{
		out[i*stride] = a[i*astride] + b[i*bstride];
	}
}
void mulacc(int n, float *out, int stride,
	 float *a, int astride, float *b, int bstride)
{
	int i;
	for(i=0;i<n;++i)
	{
		out[i*stride] += a[i*astride] * b[i*bstride];
	}
}
void kmulacc(float k, int n, float *out, int stride,
	 float *a, int astride)
{
	int i;
	for(i=0;i<n;++i)
	{
		out[i*stride] += a[i*astride] * k;
	}
}
void us_mulacc(int n, float *out,
	 float *a, float *b)
{
	int i;
	for(i=0;i<n;++i)
	{
		out[i] += a[i] * b[i];
	}
}
void us_accvec(int n, float *out,
	 float *a)
{
	int i;
	for(i=0;i<n;++i)
	{
		out[i] += a[i];
	}
}
void addvec3(int n, float *out, int stride,
	 float *a, int astride,
	 float *b, int bstride,
	  float *c, int cstride)
{
	int i;
	for(i=0;i<n;++i)
	{
		out[i*stride] = a[i*astride] + b[i*bstride] + c[i*cstride];
	}
}
void addvec4(int n, float *out, int stride,
	 float *a, int astride,
	 float *b, int bstride,
	  float *c, int cstride,
	  float *d, int dstride)
	  
{
	int i;
	for(i=0;i<n;++i)
	{
		out[i*stride] = a[i*astride] + b[i*bstride] + c[i*cstride] + d[i*dstride];
	}
}
void assignvec(int n, float *out, int stride,
	 float *a, int astride)
{
	int i;
	for(i=0;i<n;++i)
	{
		out[i*stride] = a[i*astride];
	}
	 
}
void smulvec(float s, int n, float *out, int stride, float *in, int istride)
{
	int i;
	for(i=0;i<n;++i)
		out[i*stride] = s*in[i*istride];
}
void mulvec(int n, float *out, int stride, float *a, int astride, float *b, int bstride)
{
	int i;
	for(i=0;i<n;++i)
		out[i*stride] = a[i*astride]*b[i*bstride];
}
void us_mulvec(int n, float *out,  float *a,  float *b)
{
	int i;
	for(i=0;i<n;++i)
		out[i] = a[i]*b[i];
}
void kvec(float k, int n, float *out, int stride)
{
	int i;
	for(i=0;i<n;++i)
		out[i*stride] = k;
}
static unsigned long state;
void noisevec( int n, float *out, int stride)
{
	 unsigned long m = 1103515245L;
	 unsigned long a = 12345;
	 unsigned long st = state;
	 float sc = 1.0f/INT_MAX;
	int i;

	for(i=0;i<n;++i)
	{
		st = (st*m+a);
		out[i*stride] = ((long)(st&0xffff0000))*sc;
	}
	
	state = st;
}
#define NADD 16
void gaussian_noisevec( int n, float *out, int stride)
{
	 unsigned long m = 5609937L; 
	 unsigned long a = 868765L;
	 unsigned long mask = 0xffffffL;
	 unsigned long st = state;
	 float sc = 1.0f/((float)NADD*32767.0f);
	 int t;
	int i,j;
	
	for(i=0;i<n;++i)
	{
		t= 0;
		for(j=0;j<NADD;++j)
		{
			st = (st*m+a)&mask;
		 	t += (int)(st>>8);
		 }
		 out[i*stride] = t*sc -1.0f;
	}

	state = st;
}
float dbtolin(float x)
{
	return exp(x*0.1151292546497f);
}	
/*___________________________ HANNING WINDOW ____________________________*/
#define hana	0.5f
#define	hanb	0.5f
void hanning(int n, float *sig, int stride)
{
	int	i;
	float k = 1.0f  /n ;
	double ka = PI * (2.0/n) ;
	for(i=0;i < n; ++i)
		sig[i*stride] = k*(hana + hanb * cos( i*ka -PI));
}


/*_____________________ BLACKMAN-HARRIS WINDOW __________________________*/

#define  a03   0.42323f
#define  a13   0.49755f
#define  a23   0.07922f
#define  a04   0.35875f
#define  a14   0.48829f
#define  a24   0.14128f
#define  a34   0.01168f
#define  a1h   0.54f
#define  a2h   0.46f

/*  Blackman-Harris Window  and window normalization */
void RvecBhwind(int n, float *wind,int stride, int k)
{
 register float t,s;
 register float ph1,dph1,ph2,dph2,ph3,dph3;
 register long i;

   {
   dph1 = 2.0*PI/n;
   dph2 = 2.0*dph1;
   ph1  = 0;
   ph2  = 0; 
   s= 0.0f;

   switch(k)
   {
     case 3:
        for (i=0; i<n; ++i)
        {
        	t = a03 - a13*cos(ph1) + a23*cos(ph2);
        	ph1 += dph1;
        	ph2 += dph2;
         	s+= t;
         	wind[i*stride] =  t;
        }
        break;
     case 4:
        dph3 = 3*dph1;
        ph3  = -3*PI-dph3;
        for (i=0; i<n; ++i)
        {
          t = a04;
          t += a14*cos(ph1);
          t +=  a24*cos(ph2);
          t += a34*cos(ph3);
         	ph1 += dph1;
        	ph2 += dph2;
        	ph3 += dph3;
          s += t;
          wind[i*stride] = t;
          }
        break;
    }
#ifdef SHIT
    s = 1.0f/s;
    for(i=0;i<n;++i)
    	wind[i*stride] *= s;
#endif
   }
}
void RvecBhwind3(int n, float *wind,int stride)
{
	RvecBhwind( n, wind, stride, 3);
}
/*_________________________ RECTANGULAR WINDOW __________________________*/
void RvecRecwind(int n, float *sig, int stride)
{
	kvec(1.0f/n, n, sig,stride);	 
}


/*_________________________ TRIANGULAR  WINDOW __________________________*/
void RvecTriangularwind(int n, float *sig, int stride)
{
	int i = 0;
	float k = 2.0f/(n*n);

	while(i <= n/2.0)
	{
		sig[i*stride] = i * k;
		i++;
	}
	while(i <= n - 1)
	{
		sig[i*stride] = (n - i) * k;
		i++;
	}
}


/*___________________________ HAMMING WINDOW ____________________________*/
#define hama	0.5f
#define	hamb	0.426f
void RvecHammingwind(int n, float *sig, int stride)
{
	int	i = 0;
	float k = 1.0f/n;
	double ka = PI * (2.0/n) ;
	while(i < n)
	{
		sig[i*stride] = k*(hama + hamb * cos( i*ka -PI));
		i++;
	}
}

#ifdef UNUSED
void CvecDFT(int fsize, float *real, int stride,
float *ac, int astride, float *a, int acstride)
{
	
	int ispec,isig;


	for (ispec=0;ispec<fsize;ispec++)
	{
		float k = 2.0f * PI * ispec /fsize; 
		real[ispec*stride] = 0.0f;
		ac[ispec*acstride] = 0.0f;
		for (isig=0;isig<fsize;isig++)
		{
			real[ispec*stride] += a[isig*astride] * cos(k*isig);
			ac[ispec*acstride] -= a[isig*astride] * sin(k*isig);
		}
	}
}
#endif




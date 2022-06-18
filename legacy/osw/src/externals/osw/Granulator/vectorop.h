
#ifdef __cplusplus
extern "C" {
#endif

float dbtolin(float);
void SineFunction(int n, float *stab, int stride, float from, float to);
void ExpFunction(int n, float *etab, int stride, float from, float to);
void assignvec(int n, float *out, int stride,
	 float *a, int astride);
void addvec2(int n, float *out, int stride,
	 float *a, int astride, float *b, int bstride);
void addvec3(int n, float *out, int stride,
	 float *a, int astride,
	 float *b, int bstride,
	  float *c, int cstride);
void addvec4(int n, float *out, int stride,
	 float *a, int astride,
	 float *b, int bstride,
	  float *c, int cstride,
	  float *d, int dstride);
void noisevec(register int n, float *out, int stride);
void gaussian_noisevec(register int n, float *out, int stride);
void smulvec(float s, int n, float *out, int stride, float *in, int istride);
void mulvec(int n, float *out, int stride, float *a, int astride, float *b, int bstride);
void kvec(float k, int n, float *out, int stride);
void kmulacc(float k, int n, float *out, int stride, float *a, int astride);

float productvec( int n,
	 float *a, int astride, float *b, int bstride);
void hanning(int n, float *sig, int stride);
void RvecBhwind(int n, float *wind,int stride, int k);
void RvecBhwind3(int n, float *wind,int stride);
void RvecRecwind(int n, float *sig, int stride);
void RvecHammingwind(int n, float *sig, int stride);
void mulacc(int n, float *out, int stride,
	 float *a, int astride, float *b, int bstride);
void us_mulacc(int n, float *out,
	 float *a, float *b);
void us_accvec(int n, float *out,
	 float *a);
void us_mulvec(int n, float *out,  float *a,  float *b);

void RvecTriangularwind(int n, float *sig, int stride);

#ifdef __cplusplus
}
#endif





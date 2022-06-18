
//*********************************************************************
//* Base64 - a simple base64 encoder and decoder.
//*
//*     Copyright (c) 1999, Bob Withers - bwit@pobox.com
//*
//* This code may be freely used for any purpose, either personal
//* or commercial, provided the authors copyright notice remains
//* intact.
//*
//* changed binary type from string to void * (Amar Chaudhary, 6/2000)
//*********************************************************************

#include <string>
using std::string;

#ifndef _oswexport
#if defined(_MSC_VER)
#  define _oswexport __declspec(dllexport) 
#else
#  define _oswexport
#endif
#endif


namespace Base64 {
  _oswexport string encode(void *vdata, int n, const int size);
  _oswexport int decode(string data, void *voutput, const int size);
};




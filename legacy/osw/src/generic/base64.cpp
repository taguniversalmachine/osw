
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

#include "base64.h"

const char          fillchar = '=';
static string       cvt = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                          "abcdefghijklmnopqrstuvwxyz"
                          "0123456789+/";

static void swap (unsigned char *data, const int size) {
#ifdef LITTLE_ENDIAN
      for (int l = 0; l < size / 2; ++l) {
	unsigned char t = data[l];
	data[l] = data[size-l-1];
	data[size-l-1] = t;
      }
#endif     
}

_oswexport string Base64::encode(void *vdata, int n, const int size) 
{
    auto     string::size_type  i;
    auto     char               c;
    auto     string             ret;
    auto     int                len = n * size;
    
    unsigned char  *data = (unsigned char *) vdata;

    
    for (i = 0; i < len; ++i) {
      if (i % size == 0) {
	if (i > size) {
	  swap(data+i-size,size);
	}
	swap(data+i,size);
      }
      c = (data[i] >> 2) & 0x3f;
      ret.append(1, cvt[c]);
      c = (data[i] << 4) & 0x3f;
      ++i;
      if (i % size == 0) {
	swap(data+i-size,size);
	swap(data+i,size);
      }
      if (i < len) {
	c |= (data[i] >> 4) & 0x3f;
      }
      ret.append(1, cvt[c]);
      if (i < len)
	{
	  c = (data[i] << 2) & 0x3f;
	  ++i;
	  if (i % size == 0) {
	    swap(data+i-size,size);
	    swap(data+i,size);
	  }
	  if (i < len)
	    c |= (data[i] >> 6) & 0x3f;
	  
	  ret.append(1, cvt[c]);
	}
      else
	{
	  ++i;
	  if (i % size == 0) {
	    swap(data+i-size,size);
	    swap(data+i,size);
	  }
	  ret.append(1, fillchar);
	}
      
      if (i < len)
	{
	  c = data[i] & 0x3f;
	  ret.append(1, cvt[c]);
	}
      else
	{
	  ret.append(1, fillchar);
	}
    }

    if (len % 3 == 0) {
      swap(data+len-size,size);
    }
    

    return(ret);
}

// decode now takes a void * parameter to receive output data
//   and returns the number if output bytes (AC 6/2000)

_oswexport int Base64::decode(string data, void *voutput, int size)
{
    auto     string::size_type  i;
    auto     char               c;
    auto     char               c1;
    auto     string::size_type  len = data.length();
    auto     unsigned char      *output = (unsigned char *) voutput;
    auto     int                output_length = 0;
    
    for (i = 0; i < len; ++i)
    {
        c = (char) cvt.find(data[i]);
        ++i;
        c1 = (char) cvt.find(data[i]);
        c = (c << 2) | ((c1 >> 4) & 0x3);
	output[output_length] = c;
	++output_length;
	if (output_length % size == 0) {
	  swap(output+output_length-size,size);
	}
        if (++i < len)
        {
            c = data[i];
            if (fillchar == c)
                break;

            c = (char) cvt.find(c);
            c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
	    output[output_length] = c1;
	    ++output_length;
	    if (output_length % size == 0) {
	      swap(output+output_length-size,size);
	    }
        }

        if (++i < len)
        {
            c1 = data[i];
            if (fillchar == c1)
                break;

            c1 = (char) cvt.find(c1);
            c = ((c << 6) & 0xc0) | c1;
	    output[output_length] = c;
	    ++output_length;
	    if (output_length % size == 0) {
	      swap(output+output_length-size,size);
	    }
        }
    }

    return output_length;
}


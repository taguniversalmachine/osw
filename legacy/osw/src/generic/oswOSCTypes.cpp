/* 
Copyright (c) 2001-2004 Amar Chaudhary. All rights reserved.
Copyright (c) 1998-2001 Regents of the University of California.
All rights reserved.

     ``The contents of this file are subject to the Open Sound World Public
     License Version 1.0 (the "License"); you may not use this file except in
     compliance with the License. A copy of the License should be included
     in a file named "License" or "License.txt" in the distribution from 
     which you obtained this file. 

     Software distributed under the License is distributed on an "AS IS"
     basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
     License for the specific language governing rights and limitations
     under the License.

     The Original Code is Open Sound World (OSW) software.

     The Initial Developer of the Original Code is Amar Chaudhary.
     Portions created by Amar Chaudhary are Copyright (C) 1998-2001 
     Regents of the University of California and Amar Chaudhary. 
     All Rights Reserved.

     Contributor(s):

  
From the UC Regents:

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

Written by Amar Chaudhary, The Center for New Music and Audio Technologies, 
University of California, Berkeley.

     IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
     SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
     PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
     DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF
     SUCH DAMAGE.

     REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
     FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
     DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
     REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
     ENHANCEMENTS, OR MODIFICATIONS.
     
*/

/* oswOSCTypes.h
   Implementation of OpenSound Control (OSC) type conversion services
   for OSW
*/

#include "osw.h"
#include "oswList.h"
#include "oswTime.h"
#include "oswOSCTypes.h"

#ifdef _WINDOWS
/* needed for htonl */
#include <winsock.h>
// "GetMessage" is a macro in Windows, which causes problems (AC)
#undef GetMessage
#else
#include <netinet/in.h>
#endif

namespace osw {

  /**********************/

  struct TypeInfoComp : std::binary_function<type_info*,
                        type_info*, bool> {
    bool operator()(const type_info* lhs,
		    const type_info* rhs) const
    { return lhs->before(*rhs); }
  };
  
  typedef map<const type_info *,string, TypeInfoComp> TypeTagMap;
  static TypeTagMap *theTypeTagMap = NULL;

  BaseDeclareOSCTypeTag::BaseDeclareOSCTypeTag(const type_info &atype, 
					       const string &atypetag) {
    if (!theTypeTagMap) {
      theTypeTagMap = new TypeTagMap;
    }
    (*theTypeTagMap)[&atype] = atypetag;
  }

  DECLARE_OSC_TYPETAG(float,"f");
  DECLARE_OSC_TYPETAG(int,"i");
  DECLARE_OSC_TYPETAG(char,"c");
  DECLARE_OSC_TYPETAG(string,"s");
  DECLARE_OSC_TYPETAG(double,"d");
  DECLARE_OSC_TYPETAG(Time,"t");
  DECLARE_OSC_TYPETAG(Unit,"I");
  DECLARE_OSC_TYPETAG(Samples,"v");
  DECLARE_OSC_TYPETAG(Integer16Samples,"V");
  DECLARE_OSC_TYPETAG(IntegerSamples,"3");
  DECLARE_OSC_TYPETAG(Complex,"C");

  string OSCStripLastName(const string &address) {
    // Find location of the 'last slash'
    int last_slash = address.rfind('/');
    string newstr = address;
    if(newstr.size() - last_slash > 0) {
      newstr.erase(last_slash,newstr.size()-last_slash);
    }
    return newstr;
  }

  /****************************/

  OSCParser::~OSCParser() {
  }

  void OSCParser::MessageActivate(char *address, char *typetags, char *data, char *boundary) {
  }

  char *OSCParser::DataAfterAlignedString(char *data, char *boundary) {
    // Return the boundary of the next well defined string
  
    int i;

    if (data > boundary || (boundary - data) %4 != 0) {
      throw Exception("Bad boundary");
    }   
  
    for (i = 0; data + i < boundary && data[i] != '\0'; i++) {
      if (!isprint(data[i])) throw Exception("Non printable character");
    }
 
    if (data + i == boundary) {
      throw Exception("Bad OSC string boundary (OSC requires \"data\" to be \"data\\0\\0\\0\\0)");
    }
  
    /* Now data[i] is the first null character */
    i++;
   
    for (; data + i < boundary && (i % STRING_ALIGN_PAD) != 0; i++) {
      if (data[i] != '\0') {
        throw Exception("Incorrectly padded string");
      }
    }

    if(i % STRING_ALIGN_PAD != 0) {
      throw Exception("Incorrectly padded string");
    }

    return data+i;
  }
  
  void OSCParser::ParseOSCPacket(char *data, char *boundary) {

    int n = boundary - data;

    if(data >= boundary) {
      throw Exception("Bad boundary");
    }

    // Stop Degenerate case of #bundle transform
    if (n >= 8 && strncmp(data,"#bundle",8) == 0) {
      if (n < 16) {
        throw Exception("Degenerate bundle message");
      }
         
      // A loop that un-winds a #bundle message
      /* We don't care about the time tag, so skip it for now */
      int i = 16;
      while (data + i < boundary) {
        // bundle format is [ 4 bytes int size ] [ n bytes packet ] :|
        int size = ntohl(*((int *) (data + i)));
        if ((size % 4) != 0) {
          ostringstream os;
          os << "Bad size count " << size << " in bundle (not a multiple of 4)";
          throw Exception(os.str());
        }
        i += 4;
        if(data+i+size > boundary) {
          throw Exception("Bad bundle boundary");
        }
        ParseOSCPacket(data+i, data+i+size);
        i += size;
      }
      if (i != n) {
        throw Exception("Buffer contained extra data");
      }
    }
    // Parse each individual message
    else {
  
      char *address = data;
      char *typeTags = DataAfterAlignedString(address, boundary);
      char *contents = DataAfterAlignedString(typeTags, boundary);
      MessageActivate(address, typeTags, contents, boundary);
    }
  }

  void OSCParser::ReadOSCMessage(char *data, char *boundary) {
    try {
      ParseOSCPacket(data, boundary);
    } catch(Exception e) {
      Error(e.GetMessage());
    }
  }

  // Is this function necessary??
  static const char * GetTypeTagFromList(List &alist) {
    static string result;
    result = "";
    for (List::iterator index = alist.begin();
	 index != alist.end();
	 ++index) {
      result += GetTypeTagFromDatum(*index);
    }
    return result.c_str();
  }

  // Dangerous!!!  Not pointer safe!!
  static void ListToOSC(OSCbuf *buf, List &alist) {
    for (List::iterator index = alist.begin();
	 index != alist.end();
	 ++index) {
      DatumToOSC(buf,*index);
    }
  }

  _oswexport const char *GetTypeTagFromState(BaseState *state) {
    
    if (state->TypeOf() == typeid(bool)) {
      return (*((State<bool> *)state)) ? "T" : "F";
    } else if (state->TypeOf() == typeid(List)) {
      return GetTypeTagFromList(**((State<List> *) state));
    }
    TypeTagMap::iterator found = theTypeTagMap->find(&state->TypeOf());
    if (found == theTypeTagMap->end()) {
      return "bs";
    } else {
      return found->second.c_str();
    }
  }

  _oswexport const char *GetTypeTagFromDatum(BaseDatum *datum) {

    if (datum->TypeOf() == typeid(bool)) {
      return (*((Datum<bool> *)datum)) ? "T" : "F";
    } else if (datum->TypeOf() == typeid(List)) {
      return GetTypeTagFromList(((Datum<List> *)datum)->Reference());
    }
    TypeTagMap::iterator found = theTypeTagMap->find(&datum->TypeOf());
    if (found == theTypeTagMap->end()) {
      return "bs";
    } else {
      return found->second.c_str();
    }
  }

#define TWO_TO_THE_32_FLOAT 4294967296.0f
#define TWO_TO_THE_MINUS_32_FLOAT 2.32830643654e-010

  // Danger! Not pointer safe!!!

  _oswexport void StateToOSC(OSCbuf *buf, BaseState *state) {

    if (state->TypeOf() == typeid(Any)) {
      DatumToOSC(buf,(*((State<Any> *)state))->GetDatum());
    } else if (state->TypeOf() == typeid(float)) {
      OSC_writeFloatArg(buf,*((State<float> *)state));
    } else if (state->TypeOf() == typeid(int)) {
      OSC_writeIntArg(buf,*((State<int> *)state));
    } else if (state->TypeOf() == typeid(string)) {
      OSC_writeStringArg(buf,
			 const_cast<char *>((*((State<string> *)state))->c_str()));
    } else if (state->TypeOf() == typeid(bool)) {
      OSC_skipNextTypeTag(buf);
    } else if (state->TypeOf() == typeid(Unit)) {
      OSC_skipNextTypeTag(buf);
    } else if (state->TypeOf() == typeid(double)) {
      OSC_writeDoubleArg(buf,*((State<double> *)state));
    } else if (state->TypeOf() == typeid(Time)) {
      double timeAsDouble = **((State<Time> *)state);
#ifdef HAS8BYTEINT
      OSC_writeTimeTagArg(buf,OSCTimeTag(timeAsDouble * TWO_TO_THE_32_FLOAT));
#else
      OSCTimeTag tt;
      double timeInSeconds = floor(timeAsDouble);
      tt.seconds = (uint32) timeInSeconds;
      tt.fraction = 
        (uint32) ((timeAsDouble - timeInSeconds) * TWO_TO_THE_32_FLOAT);
      OSC_writeTimeTagArg(buf,tt);
#endif
    } else if (state->TypeOf() == typeid(Samples)
	       || state->TypeOf() == typeid(IntegerSamples) 
	       || state->TypeOf() == typeid(Integer16Samples)) {
      OSC_skipNextTypeTag(buf);
      *((int *)(buf->bufptr)) = htonl(state->Size());
      buf->bufptr += 4;
      memcpy(buf->bufptr,state->Bytes(),state->Size());
      buf->bufptr += state->Size();
    } else if (state->TypeOf() == typeid(Complex)) {
      OSC_skipNextTypeTag(buf);
      int *intp = (int *) state->Bytes();
      *((int4byte *) buf->bufptr) = htonl(intp[0]);
      buf->bufptr += 4;
      *((int4byte *) buf->bufptr) = htonl(intp[1]);      
      buf->bufptr += 4;
    } else if (state->TypeOf() == typeid(List)) {
      ListToOSC(buf,**((State<List> *) state));
    } else {
      OSC_writeBlobArg(buf,(unsigned char *)state->Bytes(),state->Size());
      OSC_writeStringArg(buf,const_cast<char *>(RegisteredTypeName(state->TypeOf()).c_str()));      
    }
  }

  // Danger!!! Unsafe pointer ops!!!
   
  _oswexport void DatumToOSC(OSCbuf *buf, BaseDatum *datum) {
    if (datum->TypeOf() == typeid(float)) {
      OSC_writeFloatArg(buf,*((Datum<float> *)datum));
    } else if (datum->TypeOf() == typeid(int)) {
      OSC_writeIntArg(buf,*((Datum<int> *)datum));
    } else if (datum->TypeOf() == typeid(string)) {
      OSC_writeStringArg(buf,
			 const_cast<char *>(((Datum<string> *)datum)->Reference().c_str()));
    } else if (datum->TypeOf() == typeid(bool)) {
      OSC_skipNextTypeTag(buf);
    } else if (datum->TypeOf() == typeid(Unit)) {
      OSC_skipNextTypeTag(buf);
    } else if (datum->TypeOf() == typeid(double)) {
      OSC_writeDoubleArg(buf,*((Datum<double> *)datum));
    } else if (datum->TypeOf() == typeid(Time)) {
      double timeAsDouble = ((Datum<Time> *)datum)->Reference();
#ifdef HAS8BYTEINT
      OSC_writeTimeTagArg(buf,OSCTimeTag(timeAsDouble * TWO_TO_THE_32_FLOAT));
#else
      OSCTimeTag tt;
      double timeInSeconds = floor(timeAsDouble);
      tt.seconds = (uint32) timeInSeconds;
      tt.fraction = 
        (uint32)((timeAsDouble - timeInSeconds) * TWO_TO_THE_32_FLOAT);
      OSC_writeTimeTagArg(buf,tt);
#endif
    } else if (datum->TypeOf() == typeid(Samples)
	       || datum->TypeOf() == typeid(IntegerSamples) 
	       || datum->TypeOf() == typeid(Integer16Samples)) {
      OSC_skipNextTypeTag(buf);
      *((int *)(buf->bufptr)) = htonl(datum->Size());
      buf->bufptr += 4;
      memcpy(buf->bufptr,datum->Bytes(),datum->Size());
      buf->bufptr += datum->Size();
    } else if (datum->TypeOf() == typeid(Complex)) {
      OSC_skipNextTypeTag(buf);
      int *intp = (int *) datum->Bytes();
      *((int4byte *) buf->bufptr) = htonl(intp[0]);
      buf->bufptr += 4;
      *((int4byte *) buf->bufptr) = htonl(intp[1]);      
      buf->bufptr += 4;
    } else if (datum->TypeOf() == typeid(List)) {
      ListToOSC(buf,((Datum<List> *) datum)->Reference());
    } else {
      OSC_writeBlobArg(buf,(unsigned char *)datum->Bytes(),datum->Size());
      OSC_writeStringArg(buf,const_cast<char *>(RegisteredTypeName(datum->TypeOf()).c_str()));
    }    
  }


  // Danger!! Not pointer safe!!

  BaseDatum *OSCParser::OSCToDatum (char *&p, char *&typetag, char *boundary) {
    BaseDatum *datum;
    // Ensure that basic boundary conditions are met for each type
    // Dynamic-sized types (e.g. blob, vector) use additional checks.
    // Some types have no size and need no boundary checking.
    switch (*typetag) {
      case 'i' : case 'f' : case 'v' : case 'V' : 
        if(p + 4 > boundary) {
          throw Exception("Buffer size error while decoding datum.");
        }
      break;
      case 'd' : case 't' : case 'C' : case '3' : case 'b' : 
        if(p + 8 > boundary) {
          throw Exception("Buffer size error while decoding datum.");
        }
      break;
    }
    char *q = p; // temp pointer for later use
    switch (*typetag) {
    case 'i' :
      datum = new Datum<int>(ntohl(*((int *) p)));
      p += 4;
      return datum;
    case 'f' : {
      int i = ntohl(*((int *) p));
      float *floatp = ((float *) (&i));
      datum = new Datum<float>(*floatp);
      p += 4;
      return datum;
    }
    case 'd' : {
      int intp[2];
      int temp;
      intp[0] = ntohl (*((int *) p));
      p += 4;
      intp[1] = ntohl (*((int *) p));
      temp = intp[0];
      intp[0] = intp[1];
      intp[1] = temp;
      p += 4;
      return new Datum<double>(*((double *)intp));
    }
    case 't' : {
      int intp[2];
      intp[0] = ntohl (*((int *) p));
      p += 4;
      intp[1] = ntohl (*((int *) p));
      p += 4;
#ifdef HAS8BYTEINT
      {
	int temp;
	temp = intp[0];
	intp[0] = intp[1];
	intp[1] = temp;
      }
#endif
      return new Datum<Time>(Time(double(intp[0]) + intp[1] * 
				  TWO_TO_THE_MINUS_32_FLOAT));
    }    
    case 'I' :
      return new Datum<Unit>;
    case 'T' :
      return new Datum<bool>(true);
    case 'F' :
      return new Datum<bool>(false);
    case 'v' : {
      int size = ntohl (*((int *) p));
      p += 4;
      q = p;
      if(p + size > boundary) {
        throw Exception("Float Samples vector size goes past packet boundary.");
      }
      p += size;
      return new Datum<Samples>(Samples((float *) q, size / 4));
    }
    case 's' :
    case 'S' :
      q = p;
      p = DataAfterAlignedString(p, boundary);
      return new Datum<string>(q);
    case 'V' : {
      int size = ntohl (*((int *) p));
      p += 4;
      q = p;
      if(p + size > boundary) {
        throw Exception("Integer16Samples vector size goes past packet boundary.");
      }
      p += size;
      return new Datum<Integer16Samples>(Integer16Samples((Integer16 *) q, size / 2));
    }
    case '3' : {
      int size = ntohl (*((int *) p));
      p += 4;
      q = p;
      if(p + size > boundary) {
        throw Exception("IntegerSamples size is beyond packet boundary.");
      }
      p += size;
      return new Datum<IntegerSamples>(IntegerSamples((Integer *) q, size / 4));
    }
    case 'C' :
      int intp[2];
      intp[0] = ntohl (*((int *) p));
      p += 4;
      intp[1] = ntohl (*((int *) p));
      p += 4;
      return new Datum<Complex>(Complex(*((float *)intp),*((float *)(intp+1))));
    case 'b' : {
      int size = ntohl (*((int *) p));
      p += 4;
      int alignedsize = size;
      while (alignedsize % 4 != 0) ++alignedsize;
      if(p + alignedsize > boundary) {
        throw Exception("Bad size for OSC blob field");
      }
      // We are expecting a string after the blob, the string contains the OSC raw data type
      if( typetag[1] == 's') {
        char *blobStart = p;
        char *typeStringStart = p + alignedsize;
        p = DataAfterAlignedString(typeStringStart, boundary);
        datum = MakeDatum(typeStringStart);
        datum->SetFromBytes(blobStart,size);
        typetag++;
        return datum;
      } else {
        // Oh well... no type was specified so treat it as Raw.
        datum = new Datum<Raw>(Raw(size,p));
        p += alignedsize;
        return datum;
      }
    }
    default :
      // Make sure not to pass a N (null) typetag to this function!!
      Error (string("Unknown Type Tag") + typetag);
      return NULL;
    }
  }

  //! This cleanup handler removes the TypeTagMap object
  BEGIN_CLEANUP_HANDLER(TypeTagMap)
  {
    delete theTypeTagMap;
  }
  END_CLEANUP_HANDLER(TypeTagMap)

}


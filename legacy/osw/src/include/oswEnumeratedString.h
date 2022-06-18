
/* 
Copyright (c) 2001-2003 Amar Chaudhary. All rights reserved.
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

*/

/*
  oswEnumeratedString.h
  OSW Enumerated-String type
*/

#ifndef _OSW_ENUMERATED_STRING_H
#define _OSW_ENUMERATED_STRING_H

#include <set>

using namespace std;

namespace osw {

  //! Exception that is thrown when an enumerated-string assignment fails
  /**
   * @sa EnumeratedString
   */
  struct BadEnumeratedStringException : public Exception {

    //! Constructor takes the string that failed as an argument
    /**
     * @sa EnumeratedString
     */
    BadEnumeratedStringException(const string &badstring) :
      Exception ("enumerated-string assignment failed for " + badstring) {
    }
  };

  //! The OSW enumerated-string data type
  /**
   *  Enumerated-strings are limited to symbols contained in
   *  a list. 
   */
  struct _oswexport EnumeratedString {

    //! Set of symbols available for this EnumeratedString
    typedef std::set<String> SymbolSet;

    //! EnumeratedString constructor
    /**
     * @param astring optional initialization String
     * Construct a new EnumeratedString and set its value and symbol set
     * to the initial string
     */
    EnumeratedString (const String &astring = "") :
      xstring(astring) {
      AddSymbol(astring);
    }

    //! EnumeratedString constructor for mass-symbol initialization
    /**
     * @param stringv NULL-terminated list of C-strings to add to the new EnumeratedString's symbol set
     * This constructor adds all the symbols in stringv the symbol set
     * and sets its value to the empty string ""
     */
    EnumeratedString (const char *stringv[]) {
      for (int i = 0; stringv[i] != NULL; ++i) {
        AddSymbol(stringv[i]);
      }
    }

    //! Add an available symbol to an EnumeratedString
    void AddSymbol(const String &astring) {
      xsymbols.insert(astring);
    }

    //! Remove a symbol from an EnumeratedString's available set
    void RemoveSymbol(const String &astring) {
      xsymbols.erase(astring);
    }

    //! Access the value of an EnumeratedString as a constant C string
    operator const char * () const {return xstring.c_str();}

    //! Test the values of two EnumeratedStrings for equality
    bool operator == (const EnumeratedString &toCompare) const {
      return xstring == toCompare.xstring;
    }

    //! Test if the value of an EnumeratedString is less than another
    bool operator < (const EnumeratedString &toCompare) const {
      return xstring < toCompare.xstring;
    }

    //! Test if the value of an EnumeratedString is equal to another String
    bool operator == (const String &toCompare) const {
      return xstring < toCompare;
    }

    //! Test if the value of an EnumeratedString is less than another String
    bool operator < (const String &toCompare) const {
      return xstring < toCompare;
    }

    //! Assign a new string value to this EnumeratedString 
    /**
     *  @param toAssign the new String value to assign to this EnumeratedString
     *  This operator sets the value of the EnumeratedString to the value 
     *  toAssign only if toAssign is in the EnumeratedString's set of available
     *  symbols.  If not, value is not reassigned, and a 
     *  BadEnumeratedStringException is thrown.  An empty string is always
     *  valid as an assignment value.
     *  @sa BadEnumeratedStringException
     */
   
    EnumeratedString & operator = (const string &toAssign) {
      if (toAssign == "" || xsymbols.find(toAssign) != xsymbols.end()) {
        xstring = toAssign;
      } else {
        throw(BadEnumeratedStringException((string)toAssign));
      }
      return *this;
    }

    //! Assign a new value to this EnumeratedString from another EnumeratedString
    /**
     *  @param toAssign the new EnumeratedString value to assign to this EnumeratedString
     *  This operator sets the value of the EnumeratedString to the value 
     *  toAssign only if the value of toAssign is in the EnumeratedString's 
     *  set of available symbols.  If not, value is not reassigned, and a 
     *  BadEnumeratedStringException is thrown.  An empty string is always
     *  valid as an assignment value.
     *  @sa BadEnumeratedStringException
     */
   
    EnumeratedString & operator = (const EnumeratedString &toAssign) {
      if (toAssign.xstring == "" ||
          xsymbols.find(toAssign.xstring) != xsymbols.end()) {
        xstring = toAssign.xstring;
      } else {
        throw(BadEnumeratedStringException(toAssign.xstring));
      }
      return *this;
    }

    //! Output-stream operator for EnumeratedStrings
    friend ostream & operator << (ostream &os, const EnumeratedString &toPrint) {
      return os << toPrint.xstring;
    }

    //! Input-stream operator for EnumeratedStrings
    /**
     * The next string value in the input stream is read, and if its value is
     * in the EnumeratedString's set, it is assigned to the EnumeratedString,
     * otherwise a BadEnumeratedStringException is thrown.
     * @sa BadEnumeratedStringException
     */
    friend istream & operator >> (istream &is, EnumeratedString &toInput) {
      try {
        String temp;
        is >> temp;
        toInput = temp;
      } catch (const Exception &e) {
        osw::Error(e.what());
      }
      return is;
    }

  private:
    String    xstring;
    SymbolSet xsymbols;
  };


  //! Fill an EnumeratedString's symbol set with the current list of valid OSW types
  /**
   * @param aEnumString EnumeratedString to fill with symbols representing the valid OSW types
   */
  _oswexport void SetEnumeratedStringSymbolsToTypes(EnumeratedString &aEnumString);
  
} // namespace osw


#endif // _OSW_ENUMERATED_STRING_H

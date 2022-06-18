
/* 
Copyright (c) 2001 Amar Chaudhary. All rights reserved.
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

/*
  oswmem.h
  A memory manager that reuses fixed-size blocks
  (is this really any better that malloc and free?)
  Amar Chaudhary

*/

#ifndef _OSWMEM_H
#define _OSWMEM_H

#include "oswCfg.h"
#include <memory>


using namespace std;


namespace osw {

  void _oswexport *FindRecycledBlock (size_t asize);
  void _oswexport RecycleBlock (void *toRecycle, size_t asize);

  template<class T, class A = std::allocator<T> >
    class allocator {

    public:
      
      typedef T* pointer;
      typedef size_t size_type;
      typedef size_t difference_type;
      typedef T& reference;
      typedef const T* const_pointer;
      typedef const T& const_reference;
      typedef T value_type;
      
      pointer allocate(size_type asize, const void *unused) {
	return pointer(FindRecycledBlock(asize));
      }
      
      void deallocate(void *toDeallocate, size_type asize) {
	RecycleBlock(toDeallocate,asize);
      }

      char *_Charalloc(size_type asize) {
	return (char *)FindRecycledBlock(asize);
      }
      
      pointer address(reference r) const {
	return xalloc.address(r);
      }

      const_pointer address (const_reference cr) const {
	return xalloc.address(cr);
      }

      void construct (pointer p, const T &v) {
	xalloc.construct(p,v);
      }

      void destroy (pointer p) {
	xalloc.destroy(p);
      }

      size_type max_size() const {
	return xalloc.max_size();
      }
     
    private:
      
      A  xalloc;
  };

  template<class S, class T> 
    bool operator == (const allocator<T> alloc1, const allocator<T> alloc2) {
    return true;
  }

  template<class S, class T>
    bool operator != (const allocator<T> alloc1, const allocator<T> alloc2) {
    return false;
  }


  // put one of these in each module that uses either osw::allocator
  //  or RecycleBlock functions

  struct MemInit {

    MemInit();

  };

}

#endif // _OSWMEM_H


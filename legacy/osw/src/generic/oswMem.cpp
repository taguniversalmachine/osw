
#include <functional>
#include "oswMem.h"
#include "oswMutex.h"
#ifdef OSW_EXT_HASH_MAP
#include <ext/hash_map>
#else
#include <hash_map>
#endif

#include <iostream>

#define MAXFREEBLOCKS 1024
#define BLOCKSIZE     64
#define LOGBLOCKSIZE    6


#ifdef OSW_IMPORT_FROM_GNU_CXX
namespace std {
  using namespace __gnu_cxx;
}
#endif
using namespace std;
#ifndef OSW_USE_OWN_STL
  namespace stl=stlport;
#else
#define stl
#endif

struct recycled_mem {

  void * stack[MAXFREEBLOCKS];
  int top;

  recycled_mem() {
    top = -1;
    memset(stack,0,MAXFREEBLOCKS * sizeof(void *));
  }
  
  bool empty() {
    return top == -1;
  }

  void push(void *toPush) {
    if (top >= MAXFREEBLOCKS) {
      //cerr << "Memory Overflow!\n";
      ::delete [] (char *) toPush;
      return;
    }
    ++top;
    stack[top] = toPush;
  }

  void *pop() {
    void *toPop = NULL;
    if (top >= 0) {
      toPop = stack[top];
      --top;
    }
    return toPop;
  }

  bool operator == (const recycled_mem &toCompare) {
    return true;
  }

  bool operator != (const recycled_mem &toCompare) {
    return false;
  }
};


#if 0  
typedef stl::hash_map<unsigned int, recycled_mem, stl::hash<unsigned int>, equal_to<unsigned int> > MemoryTable;

static MemoryTable  *memTable = NULL;
static osw::Semaphore *xsemaphore = NULL;
#endif

void osw::RecycleBlock (void *toRecycle, size_t asize) {

  free(toRecycle);

#if 0
  //xsemaphore->Acquire();
  int numblocks = asize >> LOGBLOCKSIZE;
  if (asize & (BLOCKSIZE-1)) {
    ++numblocks;
  }
  //cerr << "Discarding " << numblocks << " blocks\n";
  (*memTable)[numblocks].push(toRecycle);
  //cerr << memTable[numblocks].top << endl;
  //xsemaphore->Release();
#endif
}

void *(osw::FindRecycledBlock) (size_t asize) {	

  return malloc(asize);

#if 0
  //xsemaphore->Acquire();
  int numblocks = asize >> LOGBLOCKSIZE;
  if (asize & (BLOCKSIZE-1)) {
    ++numblocks;
  }

  //cerr << memTable[numblocks].top << ' ' << memTable.size() << endl;
  if ((*memTable)[numblocks].empty()) {
    //cerr << "Allocating " << numblocks << " blocks for " << asize << " bytes " << endl;
    //xsemaphore->Release();
    return ::new char [numblocks * BLOCKSIZE];

  } else {
    //cerr << "Recycling " << numblocks << "blocks for " << asize << "bytes\n";
    //xsemaphore->Release();
    return (*memTable)[numblocks].pop();
  }
#endif
}


osw::MemInit::MemInit() {
#if 0
  if (memTable == NULL) {
    memTable = new MemoryTable;
  }
  if (xsemaphore == NULL) {
    xsemaphore = new osw::Semaphore;
  }
#endif
}







